from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from speech_recognition import Recognizer
import asyncio
import json
import serial
import threading
import os
from typing import List, Dict
import time
import audioop
from datetime import datetime
from rapidfuzz import process
import speech_recognition as sr

# Importaciones de lógica local
from app.utils import cargar_mazo_por_categoria
from app.database import init_db
from app.models import obtener_top_10, registrar_o_recuperar_jugador
from app.game_logic import engine
from app.timer_service import TimerService
from app.hardware_constants import HW_ACTIONS

app = FastAPI()
recognizer = Recognizer()

# =============================
# MEDIA / AUDIOS
# =============================
PATH_AUDIOS = r"C:\sc\UNITEC-TTB\UNITEC-TTB-Media"
app.mount("/audios", StaticFiles(directory=PATH_AUDIOS), name="audios")
print(f"✅ Servidor de audios activo en: {PATH_AUDIOS}")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# =============================
# CONFIGURACIÓN SERIAL
# =============================
PUERTO_SERIAL = "COM3"
BAUDRATE_SERIAL = 115200

try:
    ser = serial.Serial(
        port=PUERTO_SERIAL,
        baudrate=BAUDRATE_SERIAL,
        timeout=0.05,
        write_timeout=0.1,
        rtscts=False,
        dsrdtr=False
    )

    ser.setDTR(True)
    ser.setRTS(True)
    time.sleep(0.5)

    ser.reset_input_buffer()
    ser.reset_output_buffer()

    print(f"✅ Conexión serial exitosa en {PUERTO_SERIAL} @ {BAUDRATE_SERIAL}")
except Exception as e:
    print(f"❌ Error al abrir Serial: {e}")
    ser = None


class ConnectionManager:
    def __init__(self):
        self.active_connections: Dict[str, List[WebSocket]] = {"VUE": []}
        self.audio_buffer = bytearray()
        self.timer_service = TimerService(self.trigger_explosion)
        self.is_mic_streaming = False
        self.current_card = {}
        self.voice_task_running = False
        self.last_mic_start_time = 0.0
        self.last_mic_stop_time = 0.0

        # Filtro de audios duplicados
        self.last_audio_command = ""
        self.last_audio_time = 0.0

        try:
            dict_path = os.path.join(os.path.dirname(__file__), "app", "diccionario.json")
            with open(dict_path, "r", encoding="utf-8") as f:
                self.diccionario = json.load(f)
            print(f"✅ Diccionario cargado desde: {dict_path}")
        except Exception as e:
            print(f"❌ Error fatal al cargar diccionario: {e}")
            self.diccionario = {}

    async def connect(self, websocket: WebSocket, client_type: str):
        await websocket.accept()
        self.active_connections[client_type].append(websocket)
        print(f"✅ WebSocket conectado: {client_type} | Total: {len(self.active_connections[client_type])}")

    def disconnect(self, websocket: WebSocket, client_type: str):
        if websocket in self.active_connections[client_type]:
            self.active_connections[client_type].remove(websocket)
            print(f"⚠️ WebSocket desconectado: {client_type} | Total: {len(self.active_connections[client_type])}")

    async def send_to_vue(self, message: dict):
        print(f"📤 A Vue: {message}")
        dead_connections = []

        for connection in self.active_connections["VUE"]:
            try:
                await connection.send_json(message)
            except Exception as e:
                print(f"❌ Error enviando a Vue: {e}")
                dead_connections.append(connection)

        for connection in dead_connections:
            self.disconnect(connection, "VUE")

    async def send_to_esp(self, message: dict):
        if not ser or not ser.is_open:
            print("❌ Serial no disponible para enviar al ESP32.")
            return

        try:
            msg_type = message.get("type")

            if msg_type == "PLAY":
                message = {
                    "type": "CMD_AUDIO",
                    "file": message.get("id"),
                    "loop": False
                }

            elif msg_type == "START_TIC_TAC":
                message = {
                    "type": "CMD_AUDIO",
                    "file": "5_0_0_tic_tac",
                    "loop": True
                }

            elif msg_type == "BOOM":
                await self.send_to_esp({"type": "CMD_LED", "state": 9})
                message = {
                    "type": "CMD_AUDIO",
                    "file": "5_0_1_explosion",
                    "loop": False
                }

            elif msg_type == "STOP_ALL":
                message = {"type": "CMD_STOP_ALL"}

            if message.get("type") == "CMD_AUDIO":
                current_file = message.get("file", "")
                now = time.time()

                duplicate_window = 0.8 if current_file == "5_0_0_tic_tac" else 1.5

                if current_file == self.last_audio_command and (now - self.last_audio_time) < duplicate_window:
                    print(f"⚠️ Audio duplicado ignorado: {current_file}")
                    return

                self.last_audio_command = current_file
                self.last_audio_time = now

            msg = json.dumps(message, ensure_ascii=False) + "\n"
            ser.write(msg.encode("utf-8"))
            ser.flush()
            print(f"📤 Enviado al ESP32: {msg.strip()}")

        except Exception as e:
            print(f"❌ Error enviando al ESP32: {e}")

    async def send_to_esp32(self, message: dict):
        await self.send_to_esp(message)

    async def trigger_explosion(self):
        await self.send_to_esp32({"type": "BOOM"})
        await self.send_to_vue({"type": "GAME_OVER_TIMEOUT"})

    async def handle_voice_validation(self):
        if self.voice_task_running:
            print("⚠️ Validación de voz ya en curso, ignorando duplicado.")
            return

        self.voice_task_running = True

        try:
            audio_len = len(self.audio_buffer)
            print(f"🎤 Procesando {audio_len} bytes de audio...")

            if audio_len == 0:
                print("❌ No llegó audio al backend.")
                await self.send_to_esp32({"type": "CMD_LED", "state": 11})
                await self.send_to_vue({"type": "WORD_INCORRECT", "reason": "No llegó audio al backend"})
                self.timer_service.resume()
                return

            # Alinear a 16-bit PCM
            if audio_len % 2 != 0:
                self.audio_buffer = self.audio_buffer[:-1]
                audio_len = len(self.audio_buffer)

            # Métricas de debug
            try:
                rms = audioop.rms(bytes(self.audio_buffer), 2)
                peak = audioop.max(bytes(self.audio_buffer), 2)
                duration_sec = audio_len / 32000.0  # 16kHz * 2 bytes * mono

                print(f"📊 DEBUG AUDIO -> bytes={audio_len}, dur={duration_sec:.2f}s, rms={rms}, peak={peak}")

                if peak < 100:
                    print("⚠️ Audio casi muerto: señal muy baja.")
                elif peak > 30000:
                    print("⚠️ Audio posiblemente saturado.")
            except Exception as e:
                print(f"⚠️ No se pudieron calcular métricas de audio: {e}")

            if audio_len < 4000:
                print("⚠️ Audio demasiado corto para reconocer.")
                await self.send_to_esp32({"type": "CMD_LED", "state": 11})
                await self.send_to_vue({"type": "WORD_INCORRECT", "reason": "Audio demasiado corto"})
                self.timer_service.resume()
                return

            audio_data = sr.AudioData(bytes(self.audio_buffer), 16000, 2)

            # Guardar WAV siempre en carpeta fija
            try:
                debug_dir = os.path.join(os.path.dirname(__file__), "debug_audio")
                os.makedirs(debug_dir, exist_ok=True)

                filename = f"test_audio_{datetime.now().strftime('%Y%m%d_%H%M%S')}.wav"
                wav_path = os.path.join(debug_dir, filename)

                with open(wav_path, "wb") as f:
                    f.write(audio_data.get_wav_data())

                print(f"💾 WAV guardado en: {wav_path}")
            except Exception as e:
                print(f"❌ Error guardando WAV: {e}")

            palabra_raw = recognizer.recognize_google(audio_data, language="es-MX")
            print(f"📡 Google escuchó: {palabra_raw}")

            palabra_escuchada = palabra_raw.lower().strip()

            categoria = self.current_card.get("categoria")
            silaba = self.current_card.get("silaba", "").lower()
            dificultad = self.current_card.get("dificultad", "").upper()

            if not categoria or not silaba or not dificultad:
                print("❌ Error: No hay datos de carta cargados.")
                self.timer_service.resume()
                return

            try:
                palabras_validas = self.diccionario.get(categoria, {}).get(dificultad, {}).get(silaba.upper(), [])
                palabras_validas = [p.lower() for p in palabras_validas]
            except Exception as e:
                print(f"❌ Error en diccionario: {e}")
                palabras_validas = []

            if not palabras_validas:
                motivo = f"No hay palabras válidas para {categoria}/{dificultad}/{silaba.upper()}"
                print(f"❌ {motivo}")
                await self.send_to_esp32({"type": "CMD_LED", "state": 11})
                await self.send_to_vue({"type": "WORD_INCORRECT", "reason": motivo})
                self.timer_service.resume()
                return

            match = process.extractOne(palabra_escuchada, palabras_validas, score_cutoff=80)

            es_valida_final = False
            palabra_final = palabra_escuchada

            if match:
                palabra_corregida, score, index = match
                palabra_final = palabra_corregida

                if categoria == "TIC" and palabra_corregida.startswith(silaba):
                    es_valida_final = True
                elif categoria == "TAC" and silaba in palabra_corregida:
                    es_valida_final = True
                elif categoria == "BUM" and palabra_corregida.endswith(silaba):
                    es_valida_final = True

            if es_valida_final:
                print(f"✅ ¡VÁLIDA! Escuchado: '{palabra_escuchada}' -> Match: '{palabra_final}'")
                self.timer_service.stop()
                await self.send_to_esp32({"type": "CMD_LED", "state": 10})
                await self.send_to_esp32({
                    "type": "CMD_SEQUENCE",
                    "files": ["5_1_0_correct", "5_1_1_es_correcto"]
                })
                await self.send_to_vue({
                    "type": "WORD_CORRECT",
                    "word": palabra_final.capitalize()
                })
            else:
                motivo = f"'{palabra_escuchada}' no cumple la regla de {categoria} o no está en el mazo {dificultad}."
                print(f"❌ RECHAZADA: {motivo}")
                await self.send_to_esp32({"type": "CMD_LED", "state": 11})
                await self.send_to_esp32({
                    "type": "CMD_SEQUENCE",
                    "files": ["5_2_0_incorrect", "5_2_1_otra_oportunidad"]
                })
                await self.send_to_vue({
                    "type": "WORD_INCORRECT",
                    "reason": motivo
                })
                self.timer_service.resume()

        except sr.UnknownValueError:
            print("❌ Google no entendió el audio.")
            await self.send_to_esp32({"type": "CMD_LED", "state": 11})
            await self.send_to_esp32({
                "type": "CMD_SEQUENCE",
                "files": ["5_2_0_incorrect", "5_2_1_otra_oportunidad"]
            })
            await self.send_to_vue({"type": "WORD_INCORRECT", "reason": "No se entendió la palabra"})
            self.timer_service.resume()

        except sr.RequestError as e:
            print(f"❌ Error de conexión con Google Speech: {e}")
            self.timer_service.resume()

        except Exception as e:
            print(f"❌ Error inesperado procesando voz: {e}")
            self.timer_service.resume()

        finally:
            self.audio_buffer.clear()
            self.voice_task_running = False


manager = ConnectionManager()
last_event_time = 0.0


# =============================
# PROCESAMIENTO DE EVENTOS DEL ESP32
# =============================
async def process_esp32_event(data: dict):
    global last_event_time

    msg_type = data.get("type", "")
    action = data.get("action", "")
    value = data.get("value", "")
    now = time.time()

    if msg_type in ("SYSTEM", "DEBUG", "ERROR"):
        print(f"📥 ESP32 -> {data}")
        return

    if msg_type == "MIC_START":
        if manager.is_mic_streaming:
            print("⚠️ MIC_START duplicado ignorado.")
            return

        if (now - manager.last_mic_start_time) < 0.25:
            print("⚠️ MIC_START rebotado ignorado.")
            return

        manager.last_mic_start_time = now
        manager.is_mic_streaming = True
        manager.audio_buffer.clear()
        manager.timer_service.pause()
        print("🎙️ MIC_START recibido")
        return

    if msg_type == "MIC_STOP":
        if not manager.is_mic_streaming:
            print("⚠️ MIC_STOP sin streaming activo, ignorado.")
            return

        if (now - manager.last_mic_stop_time) < 0.25:
            print("⚠️ MIC_STOP rebotado ignorado.")
            return

        manager.last_mic_stop_time = now
        manager.is_mic_streaming = False
        print("🛑 MIC_STOP recibido")
        asyncio.create_task(manager.handle_voice_validation())
        return

    if msg_type != "HW_EVENT":
        return

    if action in ("BTN_TALK_START", "BTN_TALK_STOP"):
        print(f"⚠️ Evento antiguo de talk ignorado: {action}")
        return

    if action == getattr(HW_ACTIONS, "TALK_START", "__NO_MATCH__"):
        print("⚠️ TALK_START antiguo ignorado.")
        return

    if action == getattr(HW_ACTIONS, "TALK_STOP", "__NO_MATCH__"):
        print("⚠️ TALK_STOP antiguo ignorado.")
        return

    current_time = time.time()
    if (current_time - last_event_time) < 0.12:
        return
    last_event_time = current_time

    if action == HW_ACTIONS.SELECT:
        await manager.send_to_vue({"type": "HW_SELECT_CLICK"})
        return

    if action == HW_ACTIONS.TURNO:
        await manager.send_to_vue({"type": "HW_ROLL_DICE_TURNS"})
        await manager.send_to_esp32({"type": "CMD_LED", "state": 3})
        return

    if action == HW_ACTIONS.CAT:
        await manager.send_to_vue({"type": "HW_ROLL_DICE_CAT"})
        await manager.send_to_esp32({"type": "CMD_LED", "state": 4})
        return

    if action == HW_ACTIONS.MOVE or action == "HW_MOVE":
        await manager.send_to_vue({"type": "HW_MAZO_MOVE", "dir": value})
        return

    print(f"⚠️ Acción HW no manejada: action={action}, value={value}")


# =============================
# HILO DE ESCUCHA SERIAL
# =============================
def serial_listen_thread(loop, serial_conn):
    if not serial_conn or not serial_conn.is_open:
        print("❌ Serial no disponible.")
        return

    print("✅ Hilo serial iniciado.")
    text_buffer = b""

    while True:
        try:
            if serial_conn.in_waiting > 0:
                chunk = serial_conn.read(serial_conn.in_waiting)

                if manager.is_mic_streaming:
                    marker = b'{"type":"MIC_STOP"}'
                    idx = chunk.find(marker)

                    if idx != -1:
                        audio_part = chunk[:idx]
                        if audio_part:
                            manager.audio_buffer.extend(audio_part)

                        text_buffer += chunk[idx:]
                        manager.is_mic_streaming = False
                        print(f"📝 Fin de streaming detectado. Bytes capturados: {len(manager.audio_buffer)}")

                        if len(manager.audio_buffer) >= 4000:
                            asyncio.run_coroutine_threadsafe(
                                manager.handle_voice_validation(),
                                loop
                            )
                        else:
                            print(f"⚠️ Stream terminado con muy poco audio: {len(manager.audio_buffer)} bytes")
                    else:
                        manager.audio_buffer.extend(chunk)

                else:
                    text_buffer += chunk

                while b"\n" in text_buffer:
                    raw_line, text_buffer = text_buffer.split(b"\n", 1)
                    line = raw_line.decode("utf-8", errors="ignore").strip()

                    if not line:
                        continue

                    try:
                        data = json.loads(line)
                        print(f"✅ JSON VÁLIDO: {data}")
                        asyncio.run_coroutine_threadsafe(
                            process_esp32_event(data),
                            loop
                        )
                    except Exception:
                        print(f"⚠️ Línea serial ignorada: {line}")

            time.sleep(0.005)

        except Exception as e:
            print(f"⚠️ Error serial: {e}")
            break


@app.on_event("startup")
async def startup_event():
    init_db()
    loop = asyncio.get_event_loop()
    threading.Thread(target=serial_listen_thread, args=(loop, ser), daemon=True).start()


# =============================
# WEBSOCKET VUE
# =============================
@app.websocket("/ws/VUE")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket, "VUE")

    try:
        while True:
            data = await websocket.receive_json()
            print(f"📥 Vue -> {data}")

            if data.get("type") == "UI_CAMBIO_FASE":
                fase = data.get("fase")

                audio_map = {
                    "REGISTRO": "1_bienvenida",
                    "TURNO_INICIO": "2_dado_turno",
                    "CATEGORIA_INICIO": "3_dado_carta",
                    "SELECCION_CARTA_INICIO": "4_seleccion",
                }

                if fase in audio_map:
                    await manager.send_to_esp32({
                        "type": "PLAY",
                        "id": audio_map[fase]
                    })

            elif data.get("type") == "UI_CONFIG_COMPLETA":
                manager.current_card = {
                    "categoria": data.get("categoria"),
                    "dificultad": data.get("dificultad"),
                    "silaba": data.get("silaba")
                }
                print(f"🃏 Carta fijada en Back: {manager.current_card}")

            elif data.get("type") == "START_ROUND":
                manager.timer_service.stop()
                await manager.send_to_esp32({"type": "START_TIC_TAC"})
                await manager.timer_service.start(10)

    except WebSocketDisconnect:
        manager.disconnect(websocket, "VUE")
    except Exception as e:
        print(f"❌ Error en websocket VUE: {e}")
        manager.disconnect(websocket, "VUE")


# =============================
# ENDPOINTS REST
# =============================
@app.get("/ranking")
async def get_ranking():
    return obtener_top_10()


@app.post("/registrar/")
async def registrar(datos: dict):
    return registrar_o_recuperar_jugador(datos["nickname"])


@app.get("/mazo/{categoria}")
async def get_mazo(categoria: str):
    return cargar_mazo_por_categoria(categoria)