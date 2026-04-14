from sqlalchemy import Column, Integer, String, desc
from .database import Base, SessionLocal

class Ranking(Base):
    __tablename__ = "ranking"
    id = Column(Integer, primary_key=True, index=True)
    nickname = Column(String, unique=True, index=True)
    puntaje = Column(Integer, default=0)

def registrar_o_recuperar_jugador(nick: str):
    db = SessionLocal()
    try:
        # Normalizamos el nick a mayúsculas para consistencia
        nick = nick.strip().upper()
        usuario = db.query(Ranking).filter(Ranking.nickname == nick).first()
        if not usuario:
            usuario = Ranking(nickname=nick, puntaje=0)
            db.add(usuario)
            db.commit()
            db.refresh(usuario)
        return {
            "id": usuario.id,
            "nickname": usuario.nickname,
            "record_personal": usuario.puntaje,
        }
    except Exception as e:
        print(f"❌ Error DB (Registro): {e}")
        return None
    finally:
        db.close()

def guardar_puntaje(nick: str, nuevos_puntos: int):
    """Actualiza el puntaje si es mayor al récord anterior."""
    db = SessionLocal()
    hubo_record = False
    try:
        nick = nick.strip().upper()
        usuario = db.query(Ranking).filter(Ranking.nickname == nick).first()
        if usuario:
            if nuevos_puntos > usuario.puntaje:
                usuario.puntaje = nuevos_puntos
                db.commit()
                hubo_record = True
                print(f"🏆 ¡Nuevo récord para {nick}: {nuevos_puntos}!")
        return hubo_record
    except Exception as e:
        print(f"❌ Error al guardar puntaje: {e}")
        db.rollback()
        return False
    finally:
        db.close()

def obtener_top_10():
    db = SessionLocal()
    try:
        top = db.query(Ranking).order_by(desc(Ranking.puntaje)).limit(10).all()
        return [{"nickname": r.nickname, "puntaje": r.puntaje} for r in top]
    except Exception as e:
        print(f"❌ Error obteniendo Ranking: {e}")
        return []
    finally:
        db.close()