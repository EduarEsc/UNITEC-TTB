from fastapi import FastAPI, WebSocket, WebSocketDisconnect, UploadFile, File, Form, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
import asyncio
import json
import serial
import threading
import os
from typing import List, Dict, Optional
import time
import audioop
from datetime import datetime
from rapidfuzz import process
import io
import wave

# Importaciones de lógica local
from app.utils import cargar_mazo_por_categoria
from app.database import init_db
from app.models import obtener_top_10, registrar_o_recuperar_jugador
from app.timer_service import TimerService
from app.hardware_constants import HW_ACTIONS
from app.speech_recognition import SpeechRecognizer

app = FastAPI()

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
PUERTO_SERIAL = "COM9"
BAUDRATE_SERIAL = 921600

ROUND_SECONDS = 10.0

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

        self.timer_service = TimerService(self.trigger_explosion)
        self.speech_recognizer = SpeechRecognizer()

        self.is_mic_streaming = False
        self.current_mic_player: int = 0
        self.pending_upload_player: int = 0
        self.current_card = {}
        self.voice_task_running = False

        self.last_mic_start_time = 0.0
        self.last_mic_stop_time = 0.0

        self.last_audio_command = ""
        self.last_audio_time = 0.0

        self.round_total_seconds = ROUND_SECONDS
        self.current_player: int = 1
        self.round_total_ms: int = int(ROUND_SECONDS * 1000)
        self.round_remaining_ms: int = int(ROUND_SECONDS * 1000)

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

    # =============================
    # TIMER SYNC HACIA FRONT
    # =============================
    async def emit_timer_sync(self, state: str, reason: str = ""):
        remaining_ms = int(self.timer_service.get_remaining_time() * 1000)
        total_ms = int(self.round_total_seconds * 1000)

        await self.send_to_vue({
            "type": "TIMER_SYNC",
            "state": state,
            "reason": reason,
            "totalMs": total_ms,
            "remainingMs": remaining_ms
        })

    async def start_round_timer(self, seconds: float = ROUND_SECONDS, reason: str = "manual"):
        self.round_total_seconds = float(seconds)
        self.round_total_ms = int(self.round_total_seconds * 1000)
        self.round_remaining_ms = self.round_total_ms

        await self.timer_service.start(self.round_total_seconds)
        await self.emit_timer_sync("started", reason)

        print(f"⏱️ Timer reiniciado para jugador {self.current_player} por {self.round_total_seconds}s")

    async def pause_round_timer(self, reason: str = "pause"):
        self.timer_service.pause()
        await self.emit_timer_sync("paused", reason)

    async def resume_round_timer(self, reason: str = "resume"):
        if self.timer_service.get_remaining_time() <= 0:
            print("⚠️ No se reanuda timer porque ya está en 0.")
            await self.emit_timer_sync("stopped", "timer_already_zero")
            return

        self.timer_service.resume()
        await self.emit_timer_sync("resumed", reason)

    async def stop_round_timer(self, reason: str = "stop"):
        self.timer_service.stop()
        await self.emit_timer_sync("stopped", reason)

    async def trigger_explosion(self):
        await self.stop_round_timer("timeout")
        print("💥 ¡TIEMPO AGOTADO!")
        await self.send_to_esp32({"type": "BOOM"})
        await self.send_to_vue({"type": "GAME_OVER_TIMEOUT"})

    # =============================
    # DEBUG / UTILIDADES AUDIO
    # =============================
    def save_debug_wav_bytes(self, wav_bytes: bytes, prefix: str = "test_audio") -> Optional[str]:
        try:
            debug_dir = os.path.join(os.path.dirname(__file__), "debug_audio")
            os.makedirs(debug_dir, exist_ok=True)

            filename = f"{prefix}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.wav"
            wav_path = os.path.join(debug_dir, filename)

            with open(wav_path, "wb") as f:
                f.write(wav_bytes)

            return wav_path
        except Exception as e:
            print(f"❌ Error guardando WAV debug: {e}")
            return None

    def inspect_wav_bytes(self, wav_bytes: bytes):
        with wave.open(io.BytesIO(wav_bytes), "rb") as wf:
            channels = wf.getnchannels()
            sample_width = wf.getsampwidth()
            sample_rate = wf.getframerate()
            nframes = wf.getnframes()
            pcm_bytes = wf.readframes(nframes)

        return {
            "channels": channels,
            "sample_width": sample_width,
            "sample_rate": sample_rate,
            "nframes": nframes,
            "pcm_bytes": pcm_bytes
        }

    # =============================
    # VALIDACIÓN DE VOZ DESDE FRONT
    # =============================
    async def handle_voice_validation_from_wav(self, player: int, wav_bytes: bytes):
        if self.voice_task_running:
            print("⚠️ Validación de voz ya en curso, ignorando duplicado.")
            return

        self.voice_task_running = True

        try:
            await self.send_to_vue({
                "type": "MIC_PROCESSING",
                "player": player
            })

            # Mientras se valida, el tiempo sigue congelado.
            # No se reanuda aquí.

            if not wav_bytes:
                print("❌ No llegó audio al backend.")
                await self.send_to_esp32({"type": "CMD_LED", "state": 11})
                await self.send_to_vue({
                    "type": "WORD_INCORRECT",
                    "player": player,
                    "reason": "No llegó audio al backend"
                })
                await self.resume_round_timer("voice_empty")
                return

            info = self.inspect_wav_bytes(wav_bytes)
            pcm_bytes = info["pcm_bytes"]
            audio_len = len(pcm_bytes)

            print(f"🎤 Procesando audio jugador {player}: {audio_len} bytes PCM")

            if audio_len == 0:
                print("❌ WAV sin datos PCM.")
                await self.send_to_esp32({"type": "CMD_LED", "state": 11})
                await self.send_to_vue({
                    "type": "WORD_INCORRECT",
                    "player": player,
                    "reason": "El audio llegó vacío"
                })
                await self.resume_round_timer("voice_empty_pcm")
                return

            try:
                if info["sample_width"] == 2:
                    rms = audioop.rms(pcm_bytes, 2)
                    peak = audioop.max(pcm_bytes, 2)
                else:
                    rms = 0
                    peak = 0

                duration_sec = 0.0
                if info["sample_rate"] > 0 and info["sample_width"] > 0 and info["channels"] > 0:
                    duration_sec = audio_len / (info["sample_rate"] * info["sample_width"] * info["channels"])

                print(
                    f"📊 DEBUG AUDIO -> bytes={audio_len}, dur={duration_sec:.2f}s, "
                    f"rate={info['sample_rate']}, channels={info['channels']}, width={info['sample_width']}, "
                    f"rms={rms}, peak={peak}"
                )
            except Exception as e:
                print(f"⚠️ No se pudieron calcular métricas de audio: {e}")

            wav_path = self.save_debug_wav_bytes(wav_bytes, prefix=f"player_{player}")
            if wav_path:
                print(f"💾 WAV guardado en: {wav_path}")

            palabra_escuchada = self.speech_recognizer.recognize_wav_bytes(
                wav_bytes=wav_bytes,
                save_debug_wav=False
            )

            if not palabra_escuchada:
                print("❌ No se pudo transcribir la palabra.")
                await self.send_to_esp32({"type": "CMD_LED", "state": 11})
                await self.send_to_esp32({
                    "type": "CMD_SEQUENCE",
                    "files": ["5_2_0_incorrect", "5_2_1_otra_oportunidad"]
                })
                await self.send_to_vue({
                    "type": "WORD_INCORRECT",
                    "player": player,
                    "reason": "No se entendió la palabra"
                })
                await self.resume_round_timer("voice_not_understood")
                return

            print(f"📡 Google escuchó: {palabra_escuchada}")

            categoria = self.current_card.get("categoria")
            silaba = self.current_card.get("silaba", "").lower()
            dificultad = self.current_card.get("dificultad", "").upper()

            if not categoria or not silaba or not dificultad:
                print("❌ Error: No hay datos de carta cargados.")
                await self.send_to_vue({
                    "type": "WORD_INCORRECT",
                    "player": player,
                    "reason": "No hay datos de carta cargados"
                })
                await self.resume_round_timer("missing_card")
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
                await self.send_to_vue({
                    "type": "WORD_INCORRECT",
                    "player": player,
                    "reason": motivo
                })
                await self.resume_round_timer("empty_dictionary")
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
                print(f"✅ ¡VÁLIDA! Jugador {player} | Escuchado: '{palabra_escuchada}' -> Match: '{palabra_final}'")

                # El tiempo se detiene por completo.
                await self.stop_round_timer("word_correct")

                await self.send_to_esp32({"type": "CMD_LED", "state": 10})
                await self.send_to_esp32({"type": "PLAY", "id": "5_1_0_correct"})
                await asyncio.sleep(1.2)
                await self.send_to_esp32({"type": "PLAY", "id": "5_1_1_es_correcto"})
                await self.send_to_vue({
                    "type": "WORD_CORRECT",
                    "player": player,
                    "word": palabra_final.capitalize(),
                    "heard": palabra_escuchada
                })
            else:
                motivo = f"'{palabra_escuchada}' no cumple la regla de {categoria} o no está en el mazo {dificultad}."
                print(f"❌ RECHAZADA: {motivo}")

                await self.send_to_esp32({"type": "CMD_LED", "state": 11})
                await self.send_to_esp32({"type": "PLAY", "id": "5_2_0_incorrect"})
                await asyncio.sleep(1.2)
                await self.send_to_esp32({"type": "PLAY", "id": "5_2_1_otra_oportunidad"})

                await self.send_to_vue({
                    "type": "WORD_INCORRECT",
                    "player": player,
                    "reason": motivo,
                    "heard": palabra_escuchada
                })

                # Si es incorrecta, continúa desde el tiempo que sobraba.
                await self.resume_round_timer("word_incorrect")

        except Exception as e:
            print(f"❌ Error inesperado procesando voz: {e}")
            await self.resume_round_timer("voice_exception")

        finally:
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
    event = data.get("event", "")
    now = time.time()

    if msg_type == "SYSTEM":
        print(f"📥 ESP32 -> {data}")

        if event == "MIC_STREAM_STARTED":
            player = int(data.get("player", 0))

            if player not in (1, 2):
                print("⚠️ MIC_STREAM_STARTED con player inválido.")
                return
            
            if player != manager.current_player:
                print(f"⚠️ MIC_STREAM_STARTED ignorado: jugador {player} no tiene el turno actual ({manager.current_player}).")
                return

            if (now - manager.last_mic_start_time) < 0.15:
                print("⚠️ MIC_STREAM_STARTED rebotado ignorado.")
                return

            if manager.timer_service.get_remaining_time() <= 0:
                print("⚠️ MIC_STREAM_STARTED ignorado porque la ronda ya terminó.")
                return

            if manager.voice_task_running:
                print("⚠️ MIC_STREAM_STARTED ignorado porque hay validación en curso.")
                return

            manager.last_mic_start_time = now
            manager.is_mic_streaming = True
            manager.current_mic_player = player
            manager.pending_upload_player = 0

            # Al activar mic, el tiempo se congela.
            await manager.pause_round_timer("voice_capture")

            await manager.send_to_vue({
                "type": "MIC_ACTIVE",
                "player": player
            })
            return

        if event == "MIC_STREAM_STOPPED":
            player = int(data.get("player", 0))

            if (now - manager.last_mic_stop_time) < 0.15:
                print("⚠️ MIC_STREAM_STOPPED rebotado ignorado.")
                return

            manager.last_mic_stop_time = now
            manager.is_mic_streaming = False
            manager.pending_upload_player = player
            manager.current_mic_player = 0

            # El tiempo sigue congelado mientras se sube/valida.
            await manager.send_to_vue({
                "type": "MIC_INACTIVE",
                "player": player,
                "bytes": 0
            })
            return

        return

    if msg_type in ("DEBUG", "ERROR", "WARN"):
        print(f"📥 ESP32 -> {data}")
        return

    if msg_type != "HW_EVENT":
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
# HILO DE ESCUCHA SERIAL (solo JSON)
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
                if chunk:
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

            time.sleep(0.003)

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
                manager.current_player = int(data.get("turno", 1))
                print(f"🃏 Carta fijada en Back: {manager.current_card} | turno inicial: {manager.current_player}")

            elif data.get("type") == "START_ROUND":
                await manager.stop_round_timer("restart_round")
                await manager.send_to_esp32({"type": "START_TIC_TAC"})
                await manager.start_round_timer(ROUND_SECONDS, "start_round")

            elif data.get("type") == "UI_RESET_GAME":
                print("🔄 Reset global solicitado desde frontend")

                manager.current_card = {}
                manager.is_mic_streaming = False
                manager.current_mic_player = 0
                manager.pending_upload_player = 0
                manager.voice_task_running = False

                await manager.stop_round_timer("ui_reset")
                await manager.send_to_esp32({"type": "STOP_ALL"})
            
            elif data.get("type") == "NEXT_TURN":
                next_player = int(data.get("player", 1))
                total_ms = int(data.get("totalMs", 10000))
                total_seconds = max(1.0, total_ms / 1000.0)

                manager.current_player = next_player
                manager.is_mic_streaming = False
                manager.current_mic_player = 0
                manager.pending_upload_player = 0

                print(f"🔁 NEXT_TURN recibido -> player={next_player}, totalMs={total_ms}")

                await manager.stop_round_timer("next_turn_reset")
                await manager.send_to_esp32({"type": "START_TIC_TAC"})
                await manager.start_round_timer(total_seconds, "next_turn")

    except WebSocketDisconnect:
        manager.disconnect(websocket, "VUE")
    except Exception as e:
        print(f"❌ Error en websocket VUE: {e}")
        manager.disconnect(websocket, "VUE")


# =============================
# ENDPOINT AUDIO DESDE FRONT
# =============================
@app.post("/voice/upload")
async def upload_voice_audio(
    player: int = Form(...),
    audio_file: UploadFile = File(...)
):
    try:
        if player not in (1, 2):
            raise HTTPException(status_code=400, detail="Player inválido")

        file_bytes = await audio_file.read()
        if not file_bytes:
            raise HTTPException(status_code=400, detail="Archivo de audio vacío")

        filename = (audio_file.filename or "").lower()
        content_type = (audio_file.content_type or "").lower()

        if not filename.endswith(".wav") and "wav" not in content_type:
            raise HTTPException(
                status_code=400,
                detail="El backend espera audio WAV desde el frontend"
            )

        print(
            f"📥 Audio recibido desde front: "
            f"player={player}, bytes={len(file_bytes)}, "
            f"filename={audio_file.filename}, content_type={audio_file.content_type}"
        )

        await manager.handle_voice_validation_from_wav(player, file_bytes)
        manager.pending_upload_player = 0

        return {
            "ok": True,
            "player": player,
            "bytes": len(file_bytes)
        }

    except HTTPException:
        raise
    except Exception as e:
        print(f"❌ Error en /voice/upload: {e}")
        raise HTTPException(status_code=500, detail=str(e))


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