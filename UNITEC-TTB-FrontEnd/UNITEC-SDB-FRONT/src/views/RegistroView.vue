<script setup lang="ts">
import { ref, watch } from 'vue';
import axios from 'axios';
import { useGameStore } from '@/stores/game';
import { sendEvent } from '@/services/websocket';

const gameStore = useGameStore();
const cargando = ref(false);
const errorRegistro = ref(false);

/**
 * Proceso de registro en DB. 
 * Se dispara tanto por el botón físico (vía watch) como por el click en pantalla.
 */
const confirmarJugadores = async () => {
    const n1 = gameStore.jugador1.nickname.trim();
    const n2 = gameStore.jugador2.nickname.trim();

    if (!n1 || !n2) {
        errorRegistro.value = true;
        return;
    }

    cargando.value = true;
    errorRegistro.value = false;

    try {
        // 1. Registro simultáneo en la DB (Python)
        // Usamos Promise.all para que sea más rápido
        const [res1, res2] = await Promise.all([
            axios.post('http://localhost:8000/registrar/', { nickname: n1 }),
            axios.post('http://localhost:8000/registrar/', { nickname: n2 })
        ]);

        // Guardamos récords si existen
        gameStore.jugador1.puntos = res1.data.record_personal || 0;
        gameStore.jugador2.puntos = res2.data.record_personal || 0;

        console.log("✅ Jugadores registrados con éxito");

        // 2. Éxito: Saltamos a la fase de dados
        gameStore.vistaActual = 'CONFIGURACION';

        // 3. Notificamos al Backend para que el ESP32 cambie el audio a "DADOS"
        sendEvent('UI_CAMBIO_FASE', { fase: 'TURNO_INICIO' });

    } catch (error) {
        console.error("❌ Error al registrar en DB:", error);
        // Si falla la DB, permitimos pasar para no bloquear el juego, 
        // pero podrías manejar un aviso aquí.
        gameStore.vistaActual = 'CONFIGURACION';
    } finally {
        cargando.value = false;
    }
};

/**
 * INTERCEPTOR DE HARDWARE
 * El store cambia vistaActual a 'CONFIGURACION' cuando presionas SELECT.
 * Aquí "frenamos" ese cambio si los nombres no están listos.
 */
watch(() => gameStore.vistaActual, (nuevaVista) => {
    if (nuevaVista === 'CONFIGURACION' && !cargando.value) {
        const n1 = gameStore.jugador1.nickname.trim();
        const n2 = gameStore.jugador2.nickname.trim();

        if (n1 && n2) {
            // Si el hardware intentó pasar y hay nombres, ejecutamos el registro real
            confirmarJugadores();
        } else {
            // Si no hay nombres, revertimos la vista para obligar al registro
            gameStore.vistaActual = 'REGISTRO';
            errorRegistro.value = true;
            console.warn("⚠️ Intento de avance sin nombres completos.");
        }
    }
});
</script>

<template>
    <div class="registro-container">
        <h2 class="subtitle">Asignen sus nicknames</h2>

        <p class="hw-notice" :class="{ 'error-pulse': errorRegistro }">
            <span v-if="!errorRegistro">
                Escriban con el teclado y pulsen <strong>🔘 SELECT</strong> en el mando.
            </span>
            <span v-else class="error-text">
                ⚠️ ¡Faltan nicknames! Ambos jugadores deben tener un nombre.
            </span>
        </p>

        <div class="inputs-grid">
            <div class="player-card" :class="{ filled: gameStore.jugador1.nickname.length > 0 }">
                <div class="avatar J1">J1</div>
                <input v-model="gameStore.jugador1.nickname" type="text" placeholder="Nickname J1" maxlength="12"
                    :disabled="cargando" />
                <p v-if="gameStore.jugador1.puntos > 0" class="record">
                    Récord: {{ gameStore.jugador1.puntos }} pts
                </p>
            </div>

            <div class="vs-circle">VS</div>

            <div class="player-card" :class="{ filled: gameStore.jugador2.nickname.length > 0 }">
                <div class="avatar J2">J2</div>
                <input v-model="gameStore.jugador2.nickname" type="text" placeholder="Nickname J2" maxlength="12"
                    :disabled="cargando" />
                <p v-if="gameStore.jugador2.puntos > 0" class="record">
                    Récord: {{ gameStore.jugador2.puntos }} pts
                </p>
            </div>
        </div>

        <button class="btn-next" :disabled="cargando" @click="confirmarJugadores">
            <span v-if="!cargando">Continuar (🔘 SELECT)</span>
            <span v-else class="loading-spinner">Guardando...</span>
        </button>

        <p class="footer-hint">La partida no iniciará hasta que ambos se registren.</p>
    </div>
</template>

<style scoped>
.registro-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    min-height: 70vh;
    gap: 26px;
    width: 100%;
    color: #ecf0f1;
}

.subtitle {
    font-size: 2.8rem;
    font-weight: 900;
    letter-spacing: 1px;
    color: #f5f6fa;
    margin: 0;
    text-align: center;
    text-shadow: 0 4px 16px rgba(0, 0, 0, 0.35);
}

.hw-notice {
    color: #bdc3c7;
    font-style: italic;
    margin-bottom: 10px;
    text-align: center;
    font-size: 1rem;
    letter-spacing: 0.2px;
}

.inputs-grid {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 34px;
    width: 100%;
    flex-wrap: wrap;
}

.player-card {
    background: rgba(255, 255, 255, 0.06);
    padding: 34px 28px;
    border-radius: 26px;
    border: 3px solid transparent;
    transition: all 0.35s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    width: 290px;
    box-shadow:
        0 18px 34px rgba(0, 0, 0, 0.25),
        0 0 12px rgba(255, 255, 255, 0.03);
    backdrop-filter: blur(10px);
}

.player-card:hover {
    transform: translateY(-4px);
    box-shadow:
        0 22px 38px rgba(0, 0, 0, 0.32),
        0 0 18px rgba(255, 255, 255, 0.06);
}

.player-card.filled {
    border-color: #f1c40f;
    transform: scale(1.05);
    box-shadow:
        0 20px 40px rgba(0, 0, 0, 0.32),
        0 0 24px rgba(241, 196, 15, 0.22);
}

.avatar {
    width: 92px;
    height: 92px;
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
    margin: 0 auto 18px;
    font-size: 1.9rem;
    font-weight: 900;
    color: white;
    box-shadow: 0 10px 22px rgba(0, 0, 0, 0.24);
    letter-spacing: 1px;
}

.J1 {
    background: linear-gradient(135deg, #e74c3c, #c0392b);
}

.J2 {
    background: linear-gradient(135deg, #3498db, #2980b9);
}

input {
    width: 100%;
    background: rgba(0, 0, 0, 0.28);
    border: 2px solid rgba(255, 255, 255, 0.08);
    padding: 16px 18px;
    border-radius: 14px;
    color: white;
    text-align: center;
    font-size: 1.15rem;
    font-weight: 700;
    outline: none;
    transition: all 0.25s ease;
    box-sizing: border-box;
}

input::placeholder {
    color: rgba(255, 255, 255, 0.45);
    font-weight: 600;
}

input:focus {
    border-color: #3e8ebf;
    box-shadow: 0 0 16px rgba(62, 142, 191, 0.2);
    background: rgba(0, 0, 0, 0.38);
}

input:disabled {
    opacity: 0.7;
    cursor: not-allowed;
}

.record {
    margin-top: 14px;
    margin-bottom: 0;
    text-align: center;
    color: #f1c40f;
    font-weight: 800;
    font-size: 0.95rem;
    letter-spacing: 0.3px;
}

.vs-circle {
    width: 74px;
    height: 74px;
    background: linear-gradient(135deg, #f39c12, #e67e22);
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
    font-weight: 900;
    color: white;
    font-size: 1.25rem;
    letter-spacing: 1px;
    box-shadow:
        0 0 20px rgba(230, 126, 34, 0.35),
        0 10px 20px rgba(0, 0, 0, 0.28);
    flex-shrink: 0;
}

.btn-next {
    background: linear-gradient(135deg, #2ecc71, #27ae60);
    color: white;
    padding: 18px 56px;
    border: none;
    border-radius: 999px;
    font-size: 1.2rem;
    font-weight: 900;
    letter-spacing: 0.6px;
    cursor: pointer;
    transition: all 0.25s ease;
    box-shadow:
        0 12px 24px rgba(39, 174, 96, 0.28),
        0 0 14px rgba(46, 204, 113, 0.18);
}

.btn-next:hover:not(:disabled) {
    transform: translateY(-2px) scale(1.02);
    box-shadow:
        0 16px 28px rgba(39, 174, 96, 0.35),
        0 0 20px rgba(46, 204, 113, 0.24);
}

.btn-next:active:not(:disabled) {
    transform: translateY(1px) scale(0.99);
}

.btn-next:disabled {
    background: linear-gradient(135deg, #7f8c8d, #6c7a7a);
    box-shadow: 0 10px 18px rgba(127, 140, 141, 0.2);
    cursor: not-allowed;
    opacity: 0.75;
}

.loading-spinner {
    display: inline-block;
    animation: pulse 1s infinite;
}

.footer-hint {
    margin: 0;
    color: #95a5a6;
    font-size: 0.95rem;
    text-align: center;
    letter-spacing: 0.2px;
}

.error-pulse {
    animation: pulse-red 1s infinite;
}

.error-text {
    color: #ff6b6b;
    font-weight: 900;
    letter-spacing: 0.3px;
}

@keyframes pulse {
    0% {
        opacity: 1;
    }

    50% {
        opacity: 0.45;
    }

    100% {
        opacity: 1;
    }
}

@keyframes pulse-red {
    0% {
        transform: scale(1);
        opacity: 1;
    }

    50% {
        transform: scale(1.03);
        opacity: 0.75;
    }

    100% {
        transform: scale(1);
        opacity: 1;
    }
}

@media (max-width: 900px) {
    .inputs-grid {
        flex-direction: column;
        gap: 24px;
    }

    .vs-circle {
        width: 66px;
        height: 66px;
    }

    .subtitle {
        font-size: 2.2rem;
    }

    .player-card {
        width: min(320px, 92vw);
    }
}
</style>