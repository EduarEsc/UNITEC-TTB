import speech_recognition as sr
import io
import wave
import os
from datetime import datetime


class SpeechRecognizer:
    def __init__(self):
        self.recognizer = sr.Recognizer()
        self.recognizer.pause_threshold = 0.5
        self.recognizer.non_speaking_duration = 0.3
        self.recognizer.dynamic_energy_threshold = True

    def pcm_to_wav_bytes(self, audio_data_bytes: bytes) -> bytes:
        byte_io = io.BytesIO()

        with wave.open(byte_io, "wb") as wav_file:
            wav_file.setnchannels(1)
            wav_file.setsampwidth(2)
            wav_file.setframerate(16000)
            wav_file.writeframes(audio_data_bytes)

        return byte_io.getvalue()

    def save_debug_wav(self, wav_bytes: bytes, prefix: str = "test_audio") -> str:
        debug_dir = os.path.join(os.path.dirname(__file__), "..", "debug_audio")
        debug_dir = os.path.abspath(debug_dir)
        os.makedirs(debug_dir, exist_ok=True)

        filename = f"{prefix}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.wav"
        wav_path = os.path.join(debug_dir, filename)

        with open(wav_path, "wb") as f:
            f.write(wav_bytes)

        return wav_path

    def recognize(self, audio_data_bytes: bytes, save_debug_wav: bool = False) -> str:
        if not audio_data_bytes or len(audio_data_bytes) < 1000:
            print("⚠️ Audio demasiado corto para reconocer.")
            return ""

        try:
            if len(audio_data_bytes) % 2 != 0:
                audio_data_bytes = audio_data_bytes[:-1]

            wav_bytes = self.pcm_to_wav_bytes(audio_data_bytes)

            if save_debug_wav:
                wav_path = self.save_debug_wav(wav_bytes)
                print(f"💾 WAV debug guardado en: {wav_path}")

            return self.recognize_wav_bytes(wav_bytes)

        except Exception as e:
            print(f"❌ Error interno en SpeechRecognizer: {e}")
            return ""

    def recognize_wav_bytes(self, wav_bytes: bytes, save_debug_wav: bool = False) -> str:
        if not wav_bytes or len(wav_bytes) < 100:
            print("⚠️ WAV demasiado corto para reconocer.")
            return ""

        try:
            if save_debug_wav:
                wav_path = self.save_debug_wav(wav_bytes)
                print(f"💾 WAV debug guardado en: {wav_path}")

            byte_io = io.BytesIO(wav_bytes)

            with sr.AudioFile(byte_io) as source:
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