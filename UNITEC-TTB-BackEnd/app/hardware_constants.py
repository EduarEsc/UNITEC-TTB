# app/hardware_constants.py

class HW_ACTIONS:
    """Acciones que el ESP32 envía hacia el Backend"""
    SELECT = "BTN_SELECT"      # Botón de navegación
    TURNO  = "BTN_TURNO"       # Nuevo: Botón específico de Dado Turno
    CAT    = "BTN_CATEGORIA"   # Nuevo: Botón específico de Dado Categoría
    MOVE   = "HW_MOVE"         # Joystick (Value: 'L' o 'R')
    TALK_START = "BTN_TALK_START" # Cuando se presiona P1 o P2 (Value: 'P1' o 'P2')
    TALK_STOP  = "BTN_TALK_STOP"  # Cuando se sueltan

class AUDIO_IDS:
    """Nombres de archivos MP3 que el ESP32 buscará en su tarjeta SD"""
    BIENVENIDA      = "1_bienvenida"
    DADO_TURNO      = "2_dado_turno"
    DADO_CATEGORIA  = "3_dado_carta"
    SELECCION_CARTA = "4_seleccion"
    TIC_TAC         = "5_0_0_tic_tac" # Nombre ajustado a la lógica de volumen ascendente
    EXPLOSION       = "7_explosion"
    ACIERTO         = "5_1_0_correct"
    ERROR           = "5_2_0_incorrect"
    PERDIDA_VIDA    = "5_2_2_perdida_vida"

class LED_COMMANDS:
    """Comandos que enviamos al ESP32 vía JSON"""
    TIC_TAC = "START_TIC_TAC" # Activa la sirena
    STOP    = "STOP_ALL"      # Apaga todo (IDLE)
    VALIDAR = "VALIDATION_RESULT" # Nuevo: Comando para enviar el resultado (True/False)