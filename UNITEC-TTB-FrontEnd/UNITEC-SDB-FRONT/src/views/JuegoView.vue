<script setup lang="ts">
import { onMounted, onUnmounted, computed, watch, ref } from 'vue';
import { useGameStore } from '@/stores/game';
import { useRouter } from 'vue-router';

import PlayerCard from '@/components/game/PlayerCard.vue';
import GameTimer from '@/components/game/GameTimer.vue';
import GameCard from '@/components/game/GameCard.vue';
import BombaVisual from '@/components/game/BombaVisual.vue';
import GameControls from '@/components/game/GameControls.vue';
import WinnerControls from '@/components/game/WinnerControls.vue';
import InteractionOverlay from '@/components/game/InteractionOverlay.vue';
import { send } from 'vite';
import { sendEvent } from '@/services/websocket';

const gameStore = useGameStore();
const router = useRouter();

const tiempoRestante = ref(10);
let timerInterval: number | null = null;

const mostrarGanador = ref(false);
const vidaDescontadaEnExplosion = ref(false);
let siguienteRondaTimeout: number | null = null;

const estadoVozJugador1 = computed(() => {
    if (gameStore.micPlayer !== 1) return 'none';
    if (gameStore.micOn) return 'listening';
    if (gameStore.micProcessing) return 'processing';
    return 'none';
});

const estadoVozJugador2 = computed(() => {
    if (gameStore.micPlayer !== 2) return 'none';
    if (gameStore.micOn) return 'listening';
    if (gameStore.micProcessing) return 'processing';
    return 'none';
});

const cronometroPausado = computed(() => {
    return (
        gameStore.timerState === 'paused' ||
        gameStore.micOn ||
        gameStore.micProcessing ||
        gameStore.bombaExplotada ||
        gameStore.modalActivo === 'WINNER'
    );
});

const syncTiempoDesdeStore = () => {
    tiempoRestante.value = gameStore.tiempoRestanteSegundos;
};

const detenerCronometroLocal = () => {
    if (timerInterval) {
        clearInterval(timerInterval);
        timerInterval = null;
    }
};

const iniciarTickVisual = () => {
    detenerCronometroLocal();

    timerInterval = window.setInterval(() => {
        if (gameStore.timerState !== 'started' && gameStore.timerState !== 'resumed') return;

        const elapsed = Date.now() - gameStore.timerLastSyncAt;
        const remainingMs = Math.max(0, gameStore.timerRemainingMs - elapsed);

        tiempoRestante.value = Math.max(0, Math.ceil(remainingMs / 1000));
    }, 120);
};

const aplicarSyncTimer = () => {
    syncTiempoDesdeStore();

    if (gameStore.timerState === 'started' || gameStore.timerState === 'resumed') {
        iniciarTickVisual();
    } else {
        detenerCronometroLocal();
    }
};

const validarEstadoPartida = () => {
    if (gameStore.jugador1.vidas <= 0 || gameStore.jugador2.vidas <= 0) {
        setTimeout(() => {
            mostrarGanador.value = true;
            gameStore.abrirModalWinner();
        }, 1200);
    }
};

const continuarRonda = () => {
    mostrarGanador.value = false;
    vidaDescontadaEnExplosion.value = false;

    gameStore.resetEstadoRonda();
    gameStore.prepararSiguienteRonda();
};

const irADados = () => {
    gameStore.reiniciarPartidaCompleta();
    router.push('/dados');
};

const irAInicio = () => {
    gameStore.reiniciarPartidaCompleta();
    router.push('/reglas');
};

// =======================
// WATCHERS DE SINCRONÍA
// =======================
watch(
    () => [gameStore.timerState, gameStore.timerRemainingMs, gameStore.timerLastSyncAt],
    () => {
        aplicarSyncTimer();
    },
    { deep: true }
);

watch(() => gameStore.bombaExplotada, (exploto) => {
    if (!exploto) return;

    detenerCronometroLocal();
    gameStore.descontarVida();
    validarEstadoPartida();
});

onMounted(() => {
    syncTiempoDesdeStore();
    aplicarSyncTimer();
});

onUnmounted(() => {
    detenerCronometroLocal();

    if (siguienteRondaTimeout) {
        clearTimeout(siguienteRondaTimeout);
        siguienteRondaTimeout = null;
    }
});
</script>

<template>
    <div class="game-layout" :class="{ 'bg-danger': gameStore.bombaExplotada }">
        <header class="game-header">
            <PlayerCard :nickname="gameStore.jugador1.nickname" :puntos="gameStore.jugador1.puntos"
                :vidas="gameStore.jugador1.vidas" :esSuTurno="gameStore.turnoActual === 1" :posicion="1"
                :estadoVoz="estadoVozJugador1" />

            <GameTimer :tiempo="tiempoRestante" :explotada="gameStore.bombaExplotada"
                :pausado="gameStore.micOn || gameStore.timerState === 'paused'" :procesando="gameStore.micProcessing" />

            <PlayerCard :nickname="gameStore.jugador2.nickname" :puntos="gameStore.jugador2.puntos"
                :vidas="gameStore.jugador2.vidas" :esSuTurno="gameStore.turnoActual === 2" :posicion="2"
                :estadoVoz="estadoVozJugador2" />
        </header>

        <main class="game-main">
            <div class="play-zone">
                <GameCard :categoria="gameStore.categoriaActual" :silaba="gameStore.silabaActual"
                    :dificultad="gameStore.dificultadActual" />

                <div class="vs-divider"></div>

                <BombaVisual :tiempo="tiempoRestante" :explotada="gameStore.bombaExplotada"
                    :pausada="gameStore.micOn || gameStore.micProcessing || gameStore.timerState === 'paused'" />
            </div>
        </main>

        <footer class="game-footer">
            <InteractionOverlay :micOn="gameStore.micOn" :micProcessing="gameStore.micProcessing"
                :nombreJugadorActivo="gameStore.nombreJugadorMicActivo"
                :ultimaPalabraEscuchada="gameStore.ultimaPalabraEscuchada"
                :ultimaPalabraValidada="gameStore.ultimaPalabraValidada" :esPalabraValida="gameStore.esPalabraValida"
                :ultimoMotivoError="gameStore.ultimoMotivoError" />
        </footer>

        <GameControls :mostrar="gameStore.bombaExplotada && !mostrarGanador"
            :nombrePerdedor="gameStore.turnoActual === 1 ? gameStore.jugador1.nickname : gameStore.jugador2.nickname"
            :puedeContinuar="gameStore.jugador1.vidas > 0 && gameStore.jugador2.vidas > 0" @continuar="continuarRonda"
            @nuevaPartida="irADados" @irAInicio="irAInicio" />

        <WinnerControls :mostrar="mostrarGanador"
            :ganadorNickname="gameStore.jugador1.vidas <= 0 ? gameStore.jugador2.nickname : gameStore.jugador1.nickname"
            @reiniciar="irADados" @salir="irAInicio" />
    </div>
</template>

<style scoped>
.game-layout {
    width: 100%;
    height: 100vh;
    background: radial-gradient(circle at center, #1e272e 0%, #050505 100%);
    display: flex;
    flex-direction: column;
    padding: 40px;
    overflow: hidden;
    transition: background 0.8s ease;
}

.bg-danger {
    background: radial-gradient(circle at center, #4a1a1a 0%, #050505 100%);
}

.game-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    width: 100%;
    max-width: 1400px;
    margin: 0 auto;
}

.game-main {
    flex: 1;
    display: flex;
    align-items: center;
    justify-content: center;
}

.play-zone {
    display: flex;
    align-items: center;
    gap: 60px;
    position: relative;
}

.vs-divider {
    font-size: 2rem;
    font-weight: 900;
    color: rgba(255, 255, 255, 0.1);
    font-style: italic;
}

.game-footer {
    width: 100%;
    display: flex;
    justify-content: center;
    padding-bottom: 20px;
}

/* Responsividad básica */
@media (max-width: 1100px) {
    .play-zone {
        flex-direction: column;
        gap: 30px;
    }

    .vs-divider {
        display: none;
    }

    .game-header {
        gap: 10px;
    }
}
</style>