<script setup lang="ts">
import { onMounted, onUnmounted, computed, watch, ref } from 'vue';
import { useGameStore } from '@/stores/game';
import { useRouter } from 'vue-router';

// Importación de tus nuevos componentes
import PlayerCard from '@/components/game/PlayerCard.vue';
import GameTimer from '@/components/game/GameTimer.vue';
import GameCard from '@/components/game/GameCard.vue';
import BombaVisual from '@/components/game/BombaVisual.vue';
import GameControls from '@/components/game/GameControls.vue';
import WinnerControls from '@/components/game/WinnerControls.vue';
import InteractionOverlay from '@/components/game/InteractionOverlay.vue';

const gameStore = useGameStore();
const router = useRouter();

// Lógica de tiempo local
const tiempoRestante = ref(10);
let timerInterval: number | null = null;
const mostrarGanador = ref(false);

// --- LÓGICA DE CONTROL ---

const iniciarCronometro = () => {
    detenerCronometro();
    tiempoRestante.value = 10;
    timerInterval = window.setInterval(() => {
        if (tiempoRestante.value > 0 && !gameStore.bombaExplotada) {
            tiempoRestante.value--;
        } else if (tiempoRestante.value === 0) {
            procesarFallo();
        }
    }, 1000);
};

const detenerCronometro = () => {
    if (timerInterval) clearInterval(timerInterval);
};

const procesarFallo = () => {
    detenerCronometro();
    gameStore.detonarBomba(); // Activa bombaExplotada = true
    gameStore.descontarVida(); // Descuenta vida al turno actual
    validarEstadoPartida();
};

const validarEstadoPartida = () => {
    if (gameStore.jugador1.vidas <= 0 || gameStore.jugador2.vidas <= 0) {
        setTimeout(() => { mostrarGanador.value = true; }, 1200);
    }
};

// --- NAVEGACIÓN ---

const continuarRonda = () => {
    gameStore.bombaExplotada = false;
    gameStore.cambiarTurno();
    iniciarCronometro();
};

const irADados = () => router.push('/dados');
const irAInicio = () => router.push('/reglas');

// Escuchar cambios de turno (por acierto en hardware)
watch(() => gameStore.turnoActual, () => {
    if (!gameStore.bombaExplotada) iniciarCronometro();
});

onMounted(() => {
    iniciarCronometro();
});

onUnmounted(() => {
    detenerCronometro();
});
</script>

<template>
    <div class="game-layout" :class="{ 'bg-danger': gameStore.bombaExplotada }">

        <header class="game-header">
            <PlayerCard :nickname="gameStore.jugador1.nickname" :puntos="gameStore.jugador1.puntos"
                :vidas="gameStore.jugador1.vidas" :esSuTurno="gameStore.turnoActual === 1" :posicion="1" />

            <GameTimer :tiempo="tiempoRestante" :explotada="gameStore.bombaExplotada" />

            <PlayerCard :nickname="gameStore.jugador2.nickname" :puntos="gameStore.jugador2.puntos"
                :vidas="gameStore.jugador2.vidas" :esSuTurno="gameStore.turnoActual === 2" :posicion="2" />
        </header>

        <main class="game-main">
            <div class="play-zone">
                <GameCard :categoria="gameStore.categoriaActual" :silaba="gameStore.silabaActual"
                    :dificultad="gameStore.dificultadActual" />

                <div class="vs-divider"></div>

                <BombaVisual :tiempo="tiempoRestante" :explotada="gameStore.bombaExplotada" />
            </div>
        </main>

        <footer class="game-footer">
            <InteractionOverlay :micOn="gameStore.micOn"
                :nombreJugadorActivo="gameStore.turnoActual === 1 ? gameStore.jugador1.nickname : gameStore.jugador2.nickname"
                :ultimaPalabraEscuchada="gameStore.ultimaPalabraEscuchada"
                :esPalabraValida="gameStore.esPalabraValida" />
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