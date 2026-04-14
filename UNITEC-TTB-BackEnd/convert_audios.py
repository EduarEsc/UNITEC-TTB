import os
from pydub import AudioSegment

# Configuración de rutas
ORIGEN_PATH = r"C:\sc\UNITEC-TTB\UNITEC-TTB-Media"
DESTINO_PATH = r"C:\sc\UNITEC-TTB\UNITEC-TTB-Media\convertidos"

# Crear carpeta de destino si no existe
if not os.path.exists(DESTINO_PATH):
    os.makedirs(DESTINO_PATH)

def procesar_audios():
    print("🚀 Iniciando optimización de audios para ESP32...")
    
    for archivo in os.listdir(ORIGEN_PATH):
        if archivo.endswith(".mp3") or archivo.endswith(".wav"):
            try:
                path_completo = os.path.join(ORIGEN_PATH, archivo)
                print(f"--- Procesando: {archivo} ---")
                
                # Cargar el audio
                audio = AudioSegment.from_file(path_completo)
                
                # OPTIMIZACIONES CRÍTICAS PARA ESP32
                # 1. Convertir a Mono (reduce el peso a la mitad)
                audio = audio.set_channels(1)
                
                # 2. Sample Rate estándar (44.1kHz)
                audio = audio.set_frame_rate(44100)
                
                # 3. Exportar con Bitrate Constante (CBR) a 128kbps
                nombre_salida = os.path.splitext(archivo)[0] + ".mp3"
                audio.export(
                    os.path.join(DESTINO_PATH, nombre_salida),
                    format="mp3",
                    bitrate="128k",
                    parameters=["-write_id3v1", "1", "-id3v2_version", "3"] # Limpia metadatos pesados
                )
                
                print(f"✅ Éxito: {nombre_salida} optimizado.")
                
            except Exception as e:
                print(f"❌ Error con {archivo}: {e}")

    print("\n✨ ¡Proceso terminado! Los audios están en la carpeta /convertidos")
    print("Sustituye tus archivos originales con estos nuevos.")

if __name__ == "__main__":
    procesar_audios()