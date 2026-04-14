import json
import random
import os

# Ubicación del diccionario.json dentro de la carpeta 'app'
BASE_DIR = os.path.dirname(__file__)
DIC_PATH = os.path.join(BASE_DIR, "diccionario.json")

def cargar_mazo_por_categoria(categoria: str):
    """
    Extrae un mazo de 14 sílabas aleatorias basado en la categoría.
    Estructura: 4 Fáciles, 4 Medias, 6 Difíciles.
    """
    if not os.path.exists(DIC_PATH):
        print(f"❌ ERROR: No se encontró el diccionario en {DIC_PATH}")
        return []

    try:
        with open(DIC_PATH, "r", encoding="utf-8") as f:
            datos = json.load(f)

        # Normalización para coincidir con las llaves del JSON
        cat_upper = categoria.strip().upper()
        
        if cat_upper not in datos:
            print(f"⚠️ Categoría '{cat_upper}' no existe en el diccionario.")
            return []

        mazo = []
        # Distribución de dificultad para una ronda equilibrada
        config = {"FACIL": 4, "MEDIO": 4, "DIFICIL": 6}

        for dificultad, cantidad in config.items():
            if dificultad in datos[cat_upper]:
                # Obtenemos todas las sílabas de esta dificultad
                silabas_disponibles = list(datos[cat_upper][dificultad].keys())

                # Seleccionamos al azar sin repetir (sample)
                seleccion = random.sample(
                    silabas_disponibles, 
                    min(len(silabas_disponibles), cantidad)
                )

                for s in seleccion:
                    mazo.append({
                        "silaba": s.upper(), 
                        "dificultad": dificultad, 
                        "categoria": cat_upper
                    })

        # Mezclamos el mazo para que las dificultades estén intercaladas
        random.shuffle(mazo)
        print(f"🃏 Mazo creado con {len(mazo)} cartas para la categoría {cat_upper}.")
        return mazo

    except Exception as e:
        print(f"❌ Error crítico cargando mazo: {e}")
        return []