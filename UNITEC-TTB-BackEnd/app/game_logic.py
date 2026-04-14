import json
import os
from app.speech_recognition import SpeechRecognizer
from app.hardware_constants import AUDIO_IDS, LED_COMMANDS

class GameEngine:
    def __init__(self):
        # Estado de la ronda
        self.categoria_actual = None
        self.dificultad_actual = None
        self.silaba_actual = None
        
        # Estado de los jugadores
        self.turno_jugador = 1 
        self.jugador_hablando = None 
        self.puntajes = {1: 0, 2: 0}
        self.vidas = {1: 3, 2: 3}
        
        # Componentes técnicos
        self.recognizer = SpeechRecognizer()
        self.base_dir = os.path.dirname(os.path.abspath(__file__))
        self.dict_path = os.path.join(self.base_dir, "diccionario.json")

    def iniciar_vidas_duelo(self, dificultad):
        """Ajusta las vidas iniciales según la dificultad seleccionada"""
        d = dificultad.upper()
        mapeo = {"FACIL": 3, "MEDIO": 2, "DIFICIL": 1}
        v = mapeo.get(d, 2)
        self.vidas[1] = v
        self.vidas[2] = v
        return v

    def guardar_configuracion_ronda(self, categoria, dificultad, silaba):
        """Configura los parámetros de la ronda actual tras el sorteo"""
        self.categoria_actual = categoria.upper()
        self.dificultad_actual = dificultad.upper()
        self.silaba_actual = silaba.upper()
        self.iniciar_vidas_duelo(self.dificultad_actual)

    def convertir_audio_a_texto(self, audio_buffer):
        """
        Procesa los bytes RAW recibidos del MicService del ESP32.
        Se requiere un buffer mínimo para evitar ruidos falsos.
        """
        if not audio_buffer or len(audio_buffer) < 4000:
            return ""
        return self.recognizer.recognize(audio_buffer)

    def validar_palabra(self, palabra_usuario):
        """Aplica las reglas del juego para validar la palabra"""
        if not self.silaba_actual or not palabra_usuario:
            return False

        # Limpieza de caracteres especiales y espacios
        palabra_limpia = palabra_usuario.lower().strip().replace(".", "").replace(",", "")
        
        # Regla 1: Debe contener la sílaba obligatoria
        if self.silaba_actual.lower() not in palabra_limpia:
            return False

        # Regla 2: Existencia en el diccionario (Categoría -> Dificultad -> Sílaba)
        try:
            with open(self.dict_path, "r", encoding="utf-8") as f:
                datos = json.load(f)
            
            opciones = datos.get(self.categoria_actual, {}).get(self.dificultad_actual, {}).get(self.silaba_actual, [])
            # Comparación contra lista normalizada
            return palabra_limpia in [p.lower().strip() for p in opciones]
        except Exception as e:
            print(f"Error crítico leyendo diccionario: {e}")
            return False

    def procesar_resultado(self, es_valida):
        """
        Genera el paquete de datos final.
        El campo 'type': 'VALIDATION_RESULT' es el que el bridge enviará al ESP32
        para activar los LEDs verdes o rojos automáticamente.
        """
        resultado = {
            "type": "VALIDATION_RESULT", 
            "correct": es_valida,
            "terminado": False, 
            "audio_id": None,
            "vidas": self.vidas.copy(),
            "puntajes": self.puntajes.copy()
        }

        if es_valida:
            # Suma de puntos según dificultad
            puntos_map = {"FACIL": 10, "MEDIO": 25, "DIFICIL": 50}
            self.puntajes[self.turno_jugador] += puntos_map.get(self.dificultad_actual, 25)
            resultado["audio_id"] = AUDIO_IDS.ACIERTO
        else:
            # Resta de vida
            self.vidas[self.turno_jugador] -= 1
            if self.vidas[self.turno_jugador] <= 0:
                resultado["terminado"] = True
                resultado["audio_id"] = AUDIO_IDS.EXPLOSION
            else:
                resultado["audio_id"] = AUDIO_IDS.PERDIDA_VIDA

        # Actualizamos el paquete con los valores finales tras el cálculo
        resultado["puntajes"] = self.puntajes
        resultado["vidas"] = self.vidas
        return resultado

# Instancia global para ser usada por el servidor FastAPI y el Bridge Serial
engine = GameEngine()