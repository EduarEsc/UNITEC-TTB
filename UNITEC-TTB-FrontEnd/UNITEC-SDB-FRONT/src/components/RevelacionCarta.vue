<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { useGameStore } from '@/stores/game'

const gameStore = useGameStore()
const animarGiro = ref(false)
const aplicarZoom = ref(false)
const listaParaContinuar = ref(false)

const continuarAlJuego = () => {
    if (listaParaContinuar.value) {
        gameStore.iniciarJuegoTrasRevelacion()
    }
}

const unsubscribe = gameStore.$onAction(({ name }) => {
    if (name === 'confirmarSeleccionMazo') {
        if (gameStore.configStep === 'REVELACION') {
            continuarAlJuego()
        }
    }
})

onMounted(() => {
    setTimeout(() => { animarGiro.value = true }, 500)
    setTimeout(() => { aplicarZoom.value = true }, 1500)

    setTimeout(() => {
        if (gameStore.silabaActual) {
            gameStore.marcarCartaListaParaIniciar()
            listaParaContinuar.value = true
        }
    }, 4500)
})

onUnmounted(() => {
    unsubscribe()
})
</script>

<template>
    <div class="reveal-container">
        <div class="reveal-overlay"></div>
        <div class="reveal-glow glow-1"></div>
        <div class="reveal-glow glow-2"></div>

        <div class="reveal-content">
            <div class="card-scene" :class="{ 'zoom-effect': aplicarZoom }">
                <div class="card-3d" :class="{ 'is-flipped': animarGiro }">
                    <div class="card-inner">

                        <div class="face front">
                            <div class="pattern-bg"></div>
                            <div class="front-shine"></div>
                            <span class="bomb-reveal-icon">💣</span>
                            <p class="front-label">Carta secreta</p>
                        </div>

                        <div class="face back" :class="gameStore.dificultadActual?.toLowerCase()">
                            <div class="back-gloss"></div>

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

            <transition name="fade-up" class="fade-up">
                <div v-if="aplicarZoom" class="instruction-box">
                    <p class="instruction-text">¡La ronda está por comenzar!</p>
                    <p class="player-turn-hint">
                        Empieza:
                        <strong>{{ gameStore.turnoActual === 1 ? gameStore.jugador1.nickname :
                            gameStore.jugador2.nickname }}</strong>
                    </p>

                    <div v-if="listaParaContinuar" class="continue-box">
                        <p class="continue-hint">Pulsa <strong>SELECT</strong> para comenzar la ronda</p>
                        <button class="btn-next-round" @click="continuarAlJuego">
                            Iniciar Juego (🔘 SELECT)
                        </button>
                    </div>
                </div>
            </transition>
        </div>
    </div>
</template>

<style scoped>
.reveal-container {
    position: fixed;
    inset: 0;
    width: 100vw;
    height: 100vh;
    z-index: 99999;
    overflow: hidden;
    display: flex;
    align-items: center;
    justify-content: center;
}

.reveal-overlay {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    background:
        radial-gradient(circle at center, rgba(255, 255, 255, 0.06) 0%, rgba(7, 10, 18, 0.48) 38%, rgba(4, 6, 12, 0.78) 100%);
    backdrop-filter: blur(20px) saturate(1.15);
    -webkit-backdrop-filter: blur(20px) saturate(1.15);
    z-index: 0;
}

.reveal-glow {
    position: absolute;
    border-radius: 999px;
    filter: blur(80px);
    opacity: 0.5;
    pointer-events: none;
    z-index: 0;
}

.glow-1 {
    width: 380px;
    height: 380px;
    background: rgba(46, 204, 113, 0.18);
    top: 12%;
    left: 18%;
}

.glow-2 {
    width: 420px;
    height: 420px;
    background: rgba(241, 196, 15, 0.16);
    right: 14%;
    bottom: 10%;
}

.reveal-content {
    position: relative;
    z-index: 2;
    width: 100%;
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: 42px;
    padding: 40px 24px;
}

/* =========================
   ESCENA Y ZOOM
========================= */
.card-scene {
    perspective: 2600px;
    transition: transform 1.15s cubic-bezier(0.34, 1.56, 0.64, 1);
}

.zoom-effect {
    transform: scale(1.72);
}

.card-3d {
    width: 280px;
    height: 400px;
    position: relative;
}

.card-inner {
    position: relative;
    width: 100%;
    height: 100%;
    transform-style: preserve-3d;
    transition: transform 0.95s cubic-bezier(0.22, 1, 0.36, 1);
}

.is-flipped .card-inner {
    transform: rotateY(180deg);
}

/* =========================
   CARAS DE LA CARTA
========================= */
.face {
    position: absolute;
    inset: 0;
    backface-visibility: hidden;
    -webkit-backface-visibility: hidden;
    border-radius: 30px;
    overflow: hidden;
    display: flex;
    align-items: center;
    justify-content: center;
    box-shadow:
        0 30px 80px rgba(0, 0, 0, 0.45),
        0 0 0 1px rgba(255, 255, 255, 0.08),
        inset 0 1px 0 rgba(255, 255, 255, 0.08);
}

.front {
    background:
        linear-gradient(145deg, #243447 0%, #1d2938 50%, #111926 100%);
    color: white;
    border: 2px solid rgba(255, 255, 255, 0.1);
    flex-direction: column;
    gap: 16px;
}

.pattern-bg {
    position: absolute;
    inset: 0;
    opacity: 0.18;
    background-image:
        radial-gradient(rgba(255, 255, 255, 0.22) 1.3px, transparent 1.3px);
    background-size: 18px 18px;
}

.front-shine {
    position: absolute;
    top: -20%;
    left: -30%;
    width: 70%;
    height: 160%;
    transform: rotate(18deg);
    background: linear-gradient(90deg,
            transparent 0%,
            rgba(255, 255, 255, 0.03) 30%,
            rgba(255, 255, 255, 0.16) 50%,
            rgba(255, 255, 255, 0.03) 70%,
            transparent 100%);
}

.bomb-reveal-icon {
    font-size: 6.4rem;
    z-index: 2;
    filter: drop-shadow(0 12px 24px rgba(0, 0, 0, 0.35));
}

.front-label {
    z-index: 2;
    margin: 0;
    font-size: 0.9rem;
    font-weight: 800;
    letter-spacing: 2px;
    text-transform: uppercase;
    color: rgba(255, 255, 255, 0.82);
}

.back {
    transform: rotateY(180deg);
    color: #1f2937;
    padding: 24px;
    background:
        linear-gradient(180deg, rgba(255, 255, 255, 0.98) 0%, rgba(245, 247, 251, 0.98) 100%);
    border: 3px solid rgba(255, 255, 255, 0.38);
}

.back-gloss {
    position: absolute;
    inset: 0;
    background:
        radial-gradient(circle at top left, rgba(255, 255, 255, 0.72) 0%, rgba(255, 255, 255, 0.18) 22%, transparent 50%),
        linear-gradient(180deg, rgba(255, 255, 255, 0.22), transparent 30%);
    pointer-events: none;
}

.back.facil {
    box-shadow:
        0 0 40px rgba(46, 204, 113, 0.28),
        0 30px 80px rgba(0, 0, 0, 0.42);
    border-color: rgba(46, 204, 113, 0.72);
}

.back.medio {
    box-shadow:
        0 0 40px rgba(241, 196, 15, 0.28),
        0 30px 80px rgba(0, 0, 0, 0.42);
    border-color: rgba(241, 196, 15, 0.75);
}

.back.dificil {
    box-shadow:
        0 0 40px rgba(231, 76, 60, 0.28),
        0 30px 80px rgba(0, 0, 0, 0.42);
    border-color: rgba(231, 76, 60, 0.72);
}

.content {
    position: relative;
    z-index: 2;
    text-align: center;
    width: 100%;
}

.category-label {
    margin: 0 0 12px;
    font-size: 0.84rem;
    text-transform: uppercase;
    letter-spacing: 2.5px;
    color: #6b7280;
    font-weight: 900;
}

.syllable-display {
    font-size: 5.8rem;
    font-weight: 900;
    margin: 10px 0 22px;
    color: #111827;
    text-transform: uppercase;
    line-height: 1;
    letter-spacing: 2px;
    text-shadow:
        0 4px 0 rgba(255, 255, 255, 0.8),
        0 10px 22px rgba(0, 0, 0, 0.12);
}

.footer-info {
    display: flex;
    justify-content: center;
}

.difficulty-badge {
    background: rgba(238, 241, 245, 0.95);
    padding: 10px 18px;
    border-radius: 999px;
    font-size: 0.74rem;
    font-weight: 900;
    text-transform: uppercase;
    letter-spacing: 1.4px;
    color: #6b7280;
    box-shadow:
        inset 0 0 0 1px rgba(0, 0, 0, 0.05),
        0 8px 16px rgba(0, 0, 0, 0.06);
}

/* =========================
   INSTRUCCIONES
========================= */
.instruction-box {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
    min-width: 340px;
    max-width: 520px;
    padding: 15px 28px;
    border-radius: 28px;
    background: rgba(255, 255, 255, 0.09);
    border: 1px solid rgba(255, 255, 255, 0.14);
    box-shadow:
        0 20px 40px rgba(0, 0, 0, 0.28),
        0 0 30px rgba(255, 255, 255, 0.04);
    backdrop-filter: blur(14px);
    -webkit-backdrop-filter: blur(14px);
}

.instruction-text {
    margin: 0;
    color: #ffffff;
    font-weight: 900;
    font-size: 1.2rem;
    letter-spacing: 0.8px;
    text-transform: uppercase;
    text-align: center;
}

.player-turn-hint {
    margin: 0;
    color: #dbe4ee;
    font-weight: 700;
    font-size: 1rem;
    text-align: center;
}

.player-turn-hint strong {
    color: #facc15;
    text-transform: capitalize;
}

.continue-box {
    margin-top: 10px;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 12px;
}

.continue-hint {
    margin: 0;
    color: #fef08a;
    font-weight: 900;
    font-size: 0.92rem;
    letter-spacing: 1px;
    text-transform: uppercase;
    animation: pulseHint 1.4s infinite;
    text-align: center;
}

.btn-next-round {
    background: linear-gradient(135deg, #22c55e, #16a34a);
    color: white;
    border: none;
    padding: 14px 30px;
    border-radius: 999px;
    font-size: 1rem;
    font-weight: 900;
    letter-spacing: 0.4px;
    cursor: pointer;
    transition: transform 0.25s ease, box-shadow 0.25s ease;
    box-shadow:
        0 14px 26px rgba(34, 197, 94, 0.3),
        0 0 18px rgba(34, 197, 94, 0.15);
}

.btn-next-round:hover {
    transform: translateY(-2px) scale(1.02);
    box-shadow:
        0 18px 32px rgba(34, 197, 94, 0.34),
        0 0 24px rgba(34, 197, 94, 0.18);
}

/* =========================
   ANIMACIONES
========================= */
@keyframes pulseHint {

    0%,
    100% {
        opacity: 1;
        transform: scale(1);
    }

    50% {
        opacity: 0.58;
        transform: scale(1.03);
    }
}

.fade-up {
    backdrop-filter: blur(12px);
    /* background: rgba(20, 25, 35, 0.65);
    box-shadow: 0 20px 50px rgba(0, 0, 0, 0.4); */
    margin-top: 120px;
}

.fade-up-enter-active,
.fade-up-leave-active {
    transition: all 0.45s ease;
}

.fade-up-enter-from {
    opacity: 0;
    transform: translateY(18px);
}

.fade-up-leave-to {
    opacity: 0;
    transform: translateY(8px);
}

/* =========================
   RESPONSIVE
========================= */
@media (max-width: 768px) {
    .zoom-effect {
        transform: scale(1.18);
    }

    .card-3d {
        width: 230px;
        height: 330px;
    }

    .syllable-display {
        font-size: 4.6rem;
    }

    .instruction-box {
        min-width: auto;
        width: min(92vw, 420px);
        padding: 20px 22px;
    }

    .instruction-text {
        font-size: 1rem;
    }

    .player-turn-hint {
        font-size: 0.92rem;
    }

    .btn-next-round {
        width: 100%;
    }
}
</style>