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
    gap: 30px;
}

.subtitle {
    font-size: 3rem;
    color: #3e8ebf;
    margin: 0;
}

.hw-notice {
    color: #7f8c8d;
    font-style: italic;
    margin-bottom: 20px;
}

.inputs-grid {
    display: flex;
    align-items: center;
    gap: 40px;
}

.player-card {
    background: #2c3e50;
    padding: 40px;
    border-radius: 25px;
    border: 4px solid transparent;
    transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    width: 280px;
}

.player-card.filled {
    border-color: #f1c40f;
    transform: scale(1.05);
    box-shadow: 0 10px 30px rgba(241, 196, 15, 0.2);
}

.avatar {
    width: 100px;
    height: 100px;
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
    margin: 0 auto 20px;
    font-size: 2rem;
    font-weight: 900;
    color: white;
}

.J1 {
    background: #e74c3c;
}

.J2 {
    background: #3498db;
}

input {
    margin-left: -17px !important;
    width: 120%;
    background: #1a252f;
    border: 2px solid #34495e;
    padding: 15px;
    border-radius: 12px;
    color: white;
    text-align: center;
    font-size: 1.3rem;
    outline: none;
    transition: border 0.3s;
}

input:focus {
    border-color: #3e8ebf;
}

.vs-circle {
    width: 60px;
    height: 60px;
    background: #e67e22;
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
    font-weight: bold;
    color: white;
    font-size: 1.2rem;
    box-shadow: 0 0 15px rgba(230, 126, 34, 0.5);
}

.btn-next {
    background: #27ae60;
    color: white;
    padding: 20px 60px;
    border: none;
    border-radius: 50px;
    font-size: 1.4rem;
    font-weight: bold;
    cursor: pointer;
    transition: all 0.3s;
    box-shadow: 0 8px 0 #1e8449;
}

.btn-next:active {
    transform: translateY(4px);
    box-shadow: 0 4px 0 #1e8449;
}

.btn-next:disabled {
    background: #95a5a6;
    box-shadow: 0 8px 0 #7f8c8d;
    cursor: not-allowed;
    opacity: 0.7;
}

.loading-text {
    display: inline-block;
    animation: pulse 1s infinite;
}

@keyframes pulse {
    0% {
        opacity: 1;
    }

    50% {
        opacity: 0.5;
    }

    100% {
        opacity: 1;
    }
}

.error-pulse {
    animation: pulse-red 1s infinite;
}

.error-text {
    color: #ff4d4d;
    font-weight: bold;
}

@keyframes pulse-red {
    0% {
        transform: scale(1);
    }

    50% {
        transform: scale(1.05);
    }

    100% {
        transform: scale(1);
    }
}
</style>