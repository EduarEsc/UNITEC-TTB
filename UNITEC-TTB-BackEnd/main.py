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
from rapidfuzz import process, utils
import speech_recognition as sr

# Importaciones de lógica local
from app.utils import cargar_mazo_por_categoria
from app.database import init_db
from app.models import obtener_top_10, registrar_o_recuperar_jugador
from app.game_logic import engine
from app.timer_service import TimerService
from app.hardware_constants import HW_ACTIONS

app = FastAPI()
recognizer = Recognizer()# Instanciamos el traductor

# Ruta absoluta a tus audios
PATH_AUDIOS = r"C:\sc\UNITEC-TTB\UNITEC-TTB-Media"

# Montamos la carpeta para que sea accesible vía http://192.168.1.80:8000/audios/nombre.mp3
app.mount("/audios", StaticFiles(directory=PATH_AUDIOS), name="audios")
print(f"✅ Servidor de audios activo en: {PATH_AUDIOS}")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], 
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# --- CONFIGURACIÓN SERIAL ---
PUERTO_SERIAL = 'COM9' 

try:
    # Configuración robusta para ESP32-S3
    ser = serial.Serial(
        port=PUERTO_SERIAL,
        baudrate=921600,
        timeout=0.1,
        write_timeout=0.1,
        rtscts=False,
        dsrdtr=False
    )
    # El "Electroshock": Forzamos las líneas de datos
    ser.setDTR(True) 
    ser.setRTS(True)
    time.sleep(0.5) # Pausa pequeña para que el S3 reaccione
    ser.flushInput()
    ser.flushOutput()
    
    print(f"✅ Conexión forzada exitosa en {PUERTO_SERIAL}")
except Exception as e:
    print(f"❌ Error al forzar Serial: {e}")
    ser = None

class ConnectionManager:
    def __init__(self):
        self.active_connections: Dict[str, List[WebSocket]] = {"VUE": []}
        self.audio_buffer = bytearray()
        self.timer_service = TimerService(self.trigger_explosion)
        self.is_mic_streaming = False 
        self.current_card = {}  # <--- AGREGAR ESTO PARA GUARDAR LA SELECCIÓN
        try:
            dict_path = os.path.join(os.path.dirname(__file__), 'app', 'diccionario.json')
            with open(dict_path, 'r', encoding='utf-8') as f:
                self.diccionario = json.load(f)
            print(f"✅ Diccionario cargado desde: {dict_path}")
        except Exception as e:
            print(f"❌ Error fatal al cargar diccionario: {e}")
            self.diccionario = {}

    async def connect(self, websocket: WebSocket, client_type: str):
        await websocket.accept()
        self.active_connections[client_type].append(websocket)

    def disconnect(self, websocket: WebSocket, client_type: str):
        if websocket in self.active_connections[client_type]:
            self.active_connections[client_type].remove(websocket)

    async def send_to_vue(self, message: dict):
        for connection in self.active_connections["VUE"]:
            try: await connection.send_json(message)
            except: pass

    async def send_to_esp32(self, message: dict):
        if ser and ser.is_open:
            try:
                # Si enviamos un comando de audio antiguo, lo traducimos al nuevo formato
                if message.get("type") == "PLAY":
                    message = {"type": "CMD_AUDIO", "file": message.get("id"), "loop": False}
                elif message.get("type") == "START_TIC_TAC":
                    message = {"type": "CMD_AUDIO", "file": "6_tictac", "loop": True}
                elif message.get("type") == "BOOM":
                    # Enviamos audio de explosión y cambiamos LED a modo BOMBA (9)
                    await self.send_to_esp32({"type": "CMD_LED", "state": 9})
                    message = {"type": "CMD_AUDIO", "file": "7_explosion", "loop": False}
                elif message.get("type") == "STOP_ALL":
                    message = {"type": "CMD_STOP_ALL"}

                msg = json.dumps(message) + "\n"
                ser.write(msg.encode('utf-8'))
                ser.flush()
                print(f"📤 Enviado al ESP32: {msg.strip()}")
            except Exception as e:
                print(f"❌ Error enviando al ESP32: {e}")

    async def trigger_explosion(self):
        """El tiempo se agotó en el servidor"""
        # Ordenamos al ESP32 que haga sonar la explosión y ponga leds rojos
        await self.send_to_esp32({"type": "BOOM"})
        # Avisamos a la interfaz de Vue
        await self.send_to_vue({"type": "GAME_OVER_TIMEOUT"})

    async def handle_voice_validation(self):
        print(f"🎤 Procesando {len(self.audio_buffer)} bytes de audio...")
        
        # 1. TRANSFORMACIÓN Y RECONOCIMIENTO ---
        try:
            # Creamos el objeto AudioData
            # sample_rate: 16000 (lo que configuramos en el MicService del ESP32)
            # sample_width: 2 (porque es 16-bit / 2 bytes por muestra)
            audio_data = sr.AudioData(bytes(self.audio_buffer), 16000, 4)
            with open("test_audio.wav", "wb") as f:
                f.write(audio_data.get_wav_data())
            
            # Llamamos al motor de Google (requiere internet)
            # Si usas la opción B de mi respuesta anterior, usa SpeechRecognizer.recognize_google
            palabra_raw = recognizer.recognize_google(audio_data, language="es-MX")
            print(f"📡 Google escuchó: {palabra_raw}")
            
        except sr.UnknownValueError:
            print("❌ Google no entendió el audio (ruido o silencio).")
            await self.send_to_esp32({"type": "CMD_LED", "state": 11})
            await self.send_to_vue({"type": "WORD_INCORRECT", "reason": "No se entendió la palabra"})
            self.timer_service.resume()
            return
        except sr.RequestError as e:
            print(f"❌ Error de conexión con el servicio de voz: {e}")
            self.timer_service.resume()
            return
        except Exception as e:
            print(f"❌ Error inesperado: {e}")
            self.timer_service.resume()
            return

        palabra_escuchada = palabra_raw.lower().strip()

        # 2. Datos de la carta que guardamos en UI_CONFIG_COMPLETA
        categoria = self.current_card.get("categoria") 
        silaba = self.current_card.get("silaba", "").lower()
        dificultad = self.current_card.get("dificultad", "").upper() 
        
        if not categoria or not silaba or not dificultad:
            print("❌ Error: No hay datos de carta cargados.")
            self.timer_service.resume()
            return

        # 3. Obtener palabras válidas del diccionario
        try:
            palabras_validas = self.diccionario.get(categoria, {}).get(dificultad, {}).get(silaba.upper(), [])
            palabras_validas = [p.lower() for p in palabras_validas]
        except Exception as e:
            print(f"❌ Error en diccionario: {e}")
            palabras_validas = []

        # 4. Validación Robusta con RapidFuzz (Autocorrección)
        # score_cutoff=85 permite errores leves de pronunciación
        match = process.extractOne(palabra_escuchada, palabras_validas, score_cutoff=85)

        es_valida_final = False
        palabra_final = palabra_escuchada

        if match:
            palabra_corregida, score, index = match
            palabra_final = palabra_corregida # Usamos la palabra oficial del mazo
            
            # Check gramatical final por seguridad
            if categoria == "TIC" and palabra_corregida.startswith(silaba):
                es_valida_final = True
            elif categoria == "TAC" and silaba in palabra_corregida:
                es_valida_final = True
            elif categoria == "BUM" and palabra_corregida.endswith(silaba):
                es_valida_final = True

        # 5. Respuesta
        if es_valida_final:
            print(f"✅ ¡VÁLIDA! Escuchado: '{palabra_escuchada}' -> Match: '{palabra_final}'")
            self.timer_service.stop()
            await self.send_to_esp32({"type": "CMD_LED", "state": 10})
            await self.send_to_vue({"type": "WORD_CORRECT", "word": palabra_final.capitalize()})
        else:
            motivo = f"'{palabra_escuchada}' no cumple la regla de {categoria} o no está en el mazo {dificultad}."
            print(f"❌ RECHAZADA: {motivo}")
            await self.send_to_esp32({"type": "CMD_LED", "state": 11})
            await self.send_to_vue({"type": "WORD_INCORRECT", "reason": motivo})
            self.timer_service.resume()

manager = ConnectionManager()
last_event_time = 0

# --- PROCESAMIENTO DE HARDWARE ---
async def process_esp32_event(data: dict):
    global last_event_time
    action = data.get("action")
    value = data.get("value")
    
    # --- MANEJO DE MICRÓFONO Y PAUSA DE TIEMPO ---
    if action == HW_ACTIONS.TALK_START:
        print(f"🎙️ Jugador {value} empezó a hablar. Pausando temporizador...")
        manager.timer_service.pause() # Detenemos el reloj
        manager.is_mic_streaming = True
        manager.audio_buffer.clear()
        return

    elif action == HW_ACTIONS.TALK_STOP:
        print("🛑 Fin de captura. Procesando voz...")
        manager.is_mic_streaming = False
        # Disparamos la validación (esta función debería decidir si reanudar el tiempo o no)
        asyncio.create_task(manager.handle_voice_validation())
        return

    # --- DEBOUNCE PARA OTROS BOTONES ---
    current_time = time.time()
    if (current_time - last_event_time) < 0.2: return
    last_event_time = current_time

    # --- MAPEO DE EVENTOS RESTANTES ---
    if data.get("type") == "HW_EVENT":
        if action == HW_ACTIONS.SELECT:
            await manager.send_to_vue({"type": "HW_SELECT_CLICK"})            
            # El LED ya hace flash rosa solo en el ESP32, aquí solo disparamos audio si es necesario
        
        elif action == HW_ACTIONS.TURNO:
            await manager.send_to_vue({"type": "HW_ROLL_DICE_TURNS"})
            # Cambiamos a animación de giro de fuego (3)
            await manager.send_to_esp32({"type": "CMD_LED", "state": 3})
            
        elif action == HW_ACTIONS.CAT:
            await manager.send_to_vue({"type": "HW_ROLL_DICE_CAT"})
            # Cambiamos a animación de giro de fuego inverso (4)
            await manager.send_to_esp32({"type": "CMD_LED", "state": 4})
            
        elif action == HW_ACTIONS.MOVE:
            # El ESP32 ya maneja internamente MOVE_LEFT/RIGHT para feedback visual rápido
            await manager.send_to_vue({"type": "HW_MAZO_MOVE", "dir": value})

# --- HILO DE ESCUCHA SERIAL ---
def serial_listen_thread(loop, serial_conn):
    if not serial_conn or not serial_conn.is_open: return
    
    while True:
        try:
            if serial_conn.in_waiting > 0:
                # Leemos todo lo que haya en el puerto
                chunk = serial_conn.read(serial_conn.in_waiting)
                
                if manager.is_mic_streaming:
                    # Guardamos los bytes para la IA
                    manager.audio_buffer.extend(chunk)
                    
                    # --- CRÍTICO: BUSCAR EL CIERRE DE JSON DENTRO DEL AUDIO ---
                    # Buscamos el mensaje de STOP para salir del modo streaming
                    if b"BTN_TALK_STOP" in chunk:
                        print("📝 Detectado STOP en flujo binario.")
                        manager.is_mic_streaming = False
                        # Disparamos la validación
                        asyncio.run_coroutine_threadsafe(manager.handle_voice_validation(), loop)
                else:
                    # Procesamiento normal de JSON
                    try:
                        raw_line = chunk.decode('utf-8', errors='ignore').strip()
                        if "{" in raw_line and "}" in raw_line:
                            start = raw_line.find("{")
                            end = raw_line.rfind("}") + 1
                            data = json.loads(raw_line[start:end])
                            print(f"✅ JSON VÁLIDO: {data}")
                            asyncio.run_coroutine_threadsafe(process_esp32_event(data), loop)
                    except Exception as e:
                        pass # Ruido en la línea
            time.sleep(0.001)
        except Exception as e:
            print(f"⚠️ Error serial: {e}")
            break

@app.on_event("startup")
async def startup_event():
    init_db()
    loop = asyncio.get_event_loop()
    threading.Thread(target=serial_listen_thread, args=(loop, ser), daemon=True).start()

# --- WEBSOCKET VUE ---
@app.websocket("/ws/VUE")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket, "VUE")
    try:
        while True:
            data = await websocket.receive_json()
            # Cuando Vue cambia de fase, le avisa al ESP32 para que este reproduzca
            # el audio correspondiente desde su propia SD.
            if data.get("type") == "UI_CAMBIO_FASE":
                fase = data.get("fase")
                audio_map = {
                    "REGISTRO": "1_bienvenida",
                    "TURNO_INICIO": "2_dado_turno",
                    "CATEGORIA_INICIO": "3_dado_carta",
                    "SELECCION_CARTA_INICIO": "4_seleccion",
                    "PARTIDA_LISTA": "5_comenzar"
                }
                if fase in audio_map:
                    await manager.send_to_esp32({"type": "PLAY", "id": audio_map[fase]})
            if data.get("type") == "UI_CONFIG_COMPLETA":
                manager.current_card = {
                    "categoria": data.get("categoria"),
                    "dificultad": data.get("dificultad"),
                    "silaba": data.get("silaba")
                }
                print(f"🃏 Carta fijada en Back: {manager.current_card}")

            elif data.get("type") == "START_ROUND":
                # CORRECCIÓN: Usar timer_service
                manager.timer_service.stop() 
                
                # El ESP32 inicia el Tic-Tac localmente
                await manager.send_to_esp32({"type": "START_TIC_TAC"})
                
                # El Backend inicia la cuenta regresiva (Usar timer_service.start)
                await manager.timer_service.start(10)

    except WebSocketDisconnect:
        manager.disconnect(websocket, "VUE")

# --- ENDPOINTS REST ---
@app.get("/ranking")
async def get_ranking(): return obtener_top_10()

@app.post("/registrar/")
async def registrar(datos: dict): return registrar_o_recuperar_jugador(datos["nickname"])

@app.get("/mazo/{categoria}")
async def get_mazo(categoria: str): return cargar_mazo_por_categoria(categoria)
