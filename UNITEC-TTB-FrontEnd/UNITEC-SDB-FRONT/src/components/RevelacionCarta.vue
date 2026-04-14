<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useGameStore } from '@/stores/game'
import { sendEvent } from '@/services/websocket'

const gameStore = useGameStore()
const animarGiro = ref(false)
const aplicarZoom = ref(false)

onMounted(() => {
    // 1. Iniciamos la animación visual
    setTimeout(() => { animarGiro.value = true }, 500);
    setTimeout(() => { aplicarZoom.value = true }, 1500);

    // 2. Tiempo suficiente para que los jugadores lean la sílaba
    setTimeout(() => {
        if (gameStore.silabaActual) {
            // Notificamos al sistema completo que la configuración terminó
            sendEvent('UI_CONFIG_COMPLETA', {
                turno: gameStore.turnoActual,
                categoria: gameStore.categoriaActual,
                silaba: gameStore.silabaActual,
                dificultad: gameStore.dificultadActual
            });

            // Cambiamos a la vista de la bomba activa
            gameStore.vistaActual = 'JUEGO';
        }
    }, 4500); // 4.5 segundos de exposición es el "sweet spot"
})
</script>

<template>
    <div class="reveal-container">
        <div class="reveal-overlay"></div>

        <div class="card-scene" :class="{ 'zoom-effect': aplicarZoom }">
            <div class="card-3d" :class="{ 'is-flipped': animarGiro }">
                <div class="card-inner">

                    <div class="face front">
                        <div class="pattern-bg"></div>
                        <span class="bomb-reveal-icon">💣</span>
                    </div>

                    <div class="face back" :class="gameStore.dificultadActual?.toLowerCase()">
                        <div class="content">
                            <p class="category-label">{{ gameStore.categoriaActual }}</p>
                            <h1 class="syllable-display">{{ gameStore.silabaActual }}</h1>
                            <div class="footer-info">
                                <span class="difficulty-badge">{{ gameStore.dificultadActual }}</span>
                            </div>
                        </div>
                    </div>

                </div>
            </div>
        </div>

        <transition name="fade-up">
            <div v-if="aplicarZoom" class="instruction-box">
                <p class="instruction-text">¡La ronda está por comenzar!</p>
                <p class="player-turn-hint">Empieza: <strong>{{ gameStore.turnoActual === 1 ?
                    gameStore.jugador1.nickname : gameStore.jugador2.nickname }}</strong></p>
            </div>
        </transition>
    </div>
</template>

<style scoped>
.reveal-container {
    position: fixed;
    inset: 0;
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
    z-index: 999;
}

.overlay {
    position: absolute;
    inset: 0;
    background: rgba(0, 0, 0, 0.85);
    backdrop-filter: blur(8px);
    z-index: -1;
}

/* Escena y Zoom */
.card-scene {
    perspective: 2000px;
    transition: transform 1.2s cubic-bezier(0.34, 1.56, 0.64, 1);
}

.zoom-effect {
    transform: scale(1.8);
    /* El zoom que mencionaste */
}

/* Estructura 3D */
.card-3d {
    width: 240px;
    height: 340px;
    position: relative;
}

.card-inner {
    position: relative;
    width: 100%;
    height: 100%;
    transform-style: preserve-3d;
    transition: transform 0.8s cubic-bezier(0.4, 0, 0.2, 1);
}

.is-flipped .card-inner {
    transform: rotateY(180deg);
}

.face {
    position: absolute;
    inset: 0;
    backface-visibility: hidden;
    -webkit-backface-visibility: hidden;
    border-radius: 20px;
    border: 6px solid white;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    box-shadow: 0 20px 50px rgba(0, 0, 0, 0.5);
}

/* Diseño Boca Abajo */
.front {
    background: linear-gradient(135deg, #733131 0%, #3d1616 100%);
    color: white;
}

.pattern {
    position: absolute;
    inset: 0;
    opacity: 0.1;
    background-image: radial-gradient(#fff 2px, transparent 2px);
    background-size: 20px 20px;
}

.bomb-icon {
    font-size: 6rem;
    filter: drop-shadow(0 5px 15px rgba(0, 0, 0, 0.3));
}

/* Diseño Revelado */
.back {
    background: white;
    transform: rotateY(180deg);
    color: #2c3e50;
    padding: 20px;
}

.back.facil {
    border-color: #2ecc71;
}

.back.medio {
    border-color: #f1c40f;
}

.back.dificil {
    border-color: #e74c3c;
}

.content {
    text-align: center;
}

.category {
    font-size: 0.8rem;
    text-transform: uppercase;
    letter-spacing: 2px;
    color: #7f8c8d;
    margin: 0;
    font-weight: bold;
}

.syllable {
    font-size: 5rem;
    font-weight: 900;
    margin: 15px 0;
    color: #2c3e50;
    text-shadow: 2px 2px 0px rgba(0, 0, 0, 0.05);
}

.difficulty-tag {
    background: #f1f2f6;
    padding: 5px 15px;
    border-radius: 20px;
    font-size: 0.7rem;
    font-weight: 800;
    text-transform: uppercase;
    color: #95a5a6;
}

.instruction {
    margin-top: 40px;
    color: #f1c40f;
    font-weight: 900;
    font-size: 1.2rem;
    text-transform: uppercase;
    letter-spacing: 3px;
    animation: pulse 1.5s infinite;
}

@keyframes pulse {

    0%,
    100% {
        opacity: 1;
        transform: scale(1);
    }

    50% {
        opacity: 0.5;
        transform: scale(1.05);
    }
}

.fade-enter-active,
.fade-leave-active {
    transition: opacity 0.5s;
}

.fade-enter-from,
.fade-leave-to {
    opacity: 0;
}
</style>