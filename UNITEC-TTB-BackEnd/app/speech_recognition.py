import speech_recognition as sr
import io
import wave

class SpeechRecognizer:
    def __init__(self):
        self.recognizer = sr.Recognizer()
        # Parámetros optimizados para frases cortas (palabras del juego)
        self.recognizer.pause_threshold = 0.5  # Menos espera para detectar fin de palabra
        self.recognizer.non_speaking_duration = 0.3
        self.recognizer.dynamic_energy_threshold = True

    def recognize(self, audio_data_bytes):
        """
        Convierte bytes crudos (PCM 16bit 16kHz Mono) del ESP32 a texto.
        """
        if not audio_data_bytes or len(audio_data_bytes) < 1000:
            return ""

        try:
            byte_io = io.BytesIO()
            with wave.open(byte_io, "wb") as wav_file:
                # Sincronizado con el firmware del ESP32
                wav_file.setnchannels(1)      # Mono
                wav_file.setsampwidth(2)      # 16-bit (2 bytes por muestra)
                wav_file.setframerate(16000)  # 16kHz
                wav_file.writeframes(audio_data_bytes)

            byte_io.seek(0)

            with sr.AudioFile(byte_io) as source:
                # Ajuste de ruido ambiente breve para no comerse la palabra
                self.recognizer.adjust_for_ambient_noise(source, duration=0.2)
                audio_data = self.recognizer.record(source)
                
                # Transcripción con Google en español mexicano
                text = self.recognizer.recognize_google(audio_data, language="es-MX")
                
                # Limpieza de resultados
                resultado = text.lower().strip()
                print(f"🎙️ Google transcribió: '{resultado}'")
                return resultado

        except sr.UnknownValueError:
            print("❌ Google: No se entendió la palabra (silencio o ruido).")
            return ""
        except sr.RequestError as e:
            print(f"❌ Error de red con Google: {e}")
            return ""
        except Exception as e:
            print(f"❌ Error interno en SpeechRecognizer: {e}")
            return ""