from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker

# 1. Definimos la ubicación de la base de datos local
# El archivo 'ranking.db' se creará automáticamente en la raíz de tu proyecto
SQLALCHEMY_DATABASE_URL = "sqlite:///./ranking.db"

# 2. Creamos el motor de la base de datos
# 'check_same_thread': False es necesario específicamente para SQLite en FastAPI
engine = create_engine(
    SQLALCHEMY_DATABASE_URL, connect_args={"check_same_thread": False}
)

# 3. Creamos una fábrica de sesiones
# Cada vez que necesitemos guardar o leer datos, usaremos una instancia de SessionLocal
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

# 4. Clase base para que nuestros modelos de datos hereden de ella
Base = declarative_base()


# 5. Función para inicializar la base de datos
# Esta función es llamada por main.py al iniciar el servidor
def init_db():
    from .models import Ranking  # Importación local para evitar importación circular

    Base.metadata.create_all(bind=engine)
    print("Base de datos de Ranking inicializada correctamente.")
