import speech_recognition as sr
import io
import wave
from typing import Optional


class SpeechRecognizer:
    def __init__(self):
        self.recognizer = sr.Recognizer()

        # Ajustes pensados para palabras cortas
        self.recognizer.pause_threshold = 0.5
        self.recognizer.non_speaking_duration = 0.3
        self.recognizer.dynamic_energy_threshold = True

    def recognize(self, audio_data_bytes: bytes, save_debug_wav: bool = False) -> str:
        """
        Convierte bytes crudos PCM 16-bit, 16kHz, mono a texto usando Google.
        Retorna cadena vacía si no se reconoce nada.
        """

        if not audio_data_bytes or len(audio_data_bytes) < 1000:
            print("⚠️ Audio demasiado corto para reconocer.")
            return ""

        try:
            # Asegurar alineación correcta para 16-bit PCM
            if len(audio_data_bytes) % 2 != 0:
                audio_data_bytes = audio_data_bytes[:-1]

            byte_io = io.BytesIO()

            with wave.open(byte_io, "wb") as wav_file:
                wav_file.setnchannels(1)      # Mono
                wav_file.setsampwidth(2)      # 16-bit PCM
                wav_file.setframerate(16000)  # 16 kHz
                wav_file.writeframes(audio_data_bytes)

            wav_bytes = byte_io.getvalue()

            if save_debug_wav:
                with open("test_audio.wav", "wb") as f:
                    f.write(wav_bytes)

            byte_io = io.BytesIO(wav_bytes)

            with sr.AudioFile(byte_io) as source:
                # NO usar adjust_for_ambient_noise aquí:
                # este audio ya fue capturado y puede cortar la palabra.
                audio_data = self.recognizer.record(source)

            text = self.recognizer.recognize_google(audio_data, language="es-MX")
            resultado = text.lower().strip()

            print(f"🎙️ Google transcribió: '{resultado}'")
            return resultado

        except sr.UnknownValueError:
            print("❌ Google: No se entendió la palabra.")
            return ""

        except sr.RequestError as e:
            print(f"❌ Error de red con Google: {e}")
            return ""

        except Exception as e:
            print(f"❌ Error interno en SpeechRecognizer: {e}")
            return ""