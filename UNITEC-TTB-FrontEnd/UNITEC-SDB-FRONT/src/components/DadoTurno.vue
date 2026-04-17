<script setup lang="ts">
import { ref, computed, onUnmounted } from 'vue';
import { useGameStore } from '@/stores/game';

const gameStore = useGameStore();
const emit = defineEmits(['finalizado']);

const rolling = ref(false);
const currentRotation = ref({ x: 0, y: 0 });
const dadoJ1 = ref<number | null>(null);
const dadoJ2 = ref<number | null>(null);
const turnoActualLanzando = ref(1); // 1 = J1, 2 = J2
const dueloTerminado = ref(false);

const rotations: Record<number, { x: number; y: number }> = {
    1: { x: 0, y: 0 },
    2: { x: 0, y: -90 },
    3: { x: -90, y: 0 },
    4: { x: 90, y: 0 },
    5: { x: 0, y: 90 },
    6: { x: 0, y: 180 }
};

const diceStyle = computed(() => ({
    transform: `rotateX(${currentRotation.value.x}deg) rotateY(${currentRotation.value.y}deg)`
}));

const rollDice = () => {
    // Bloqueo si ya está girando o si ya hay un ganador que no sea empate
    if (rolling.value || (dueloTerminado.value && dadoJ1.value !== dadoJ2.value)) return;

    rolling.value = true;
    const newResult = Math.floor(Math.random() * 6) + 1;

    const targetRotation = rotations[newResult];
    if (targetRotation) {
        // Añadimos muchas vueltas para el efecto visual
        currentRotation.value = {
            x: targetRotation.x + 1440,
            y: targetRotation.y + 1800
        };
    }

    setTimeout(() => {
        rolling.value = false;

        if (turnoActualLanzando.value === 1) {
            dadoJ1.value = newResult;
            // Pequeña pausa antes de pasar el turno al J2 para que vean el resultado
            setTimeout(() => {
                turnoActualLanzando.value = 2;
            }, 1500);
        } else {
            dadoJ2.value = newResult;
            dueloTerminado.value = true;
        }

        // Normalizamos grados para evitar números gigantes
        currentRotation.value.x %= 360;
        currentRotation.value.y %= 360;
    }, 2000);
};

const reiniciarDuelo = () => {
    dadoJ1.value = null;
    dadoJ2.value = null;
    turnoActualLanzando.value = 1;
    dueloTerminado.value = false;
};

const finalizarPaso = () => {
    if (dadoJ1.value && dadoJ2.value && dadoJ1.value !== dadoJ2.value) {
        const ganadorID = dadoJ1.value > dadoJ2.value ? 1 : 2;
        emit('finalizado', ganadorID);
    }
};

/**
 * ESCUCHA DE HARDWARE 🎮
 */
const unsubscribe = gameStore.$onAction(({ name }) => {
    if (name === 'lanzarDadosTurno') {
        rollDice();
    }

    if (name === 'confirmarSeleccionMazo') {
        // Si hay empate, SELECT limpia para repetir
        if (dadoJ1.value !== null && dadoJ1.value === dadoJ2.value) {
            reiniciarDuelo();
        }
        // Si hay ganador, SELECT avanza al siguiente paso (Categoría)
        else if (dueloTerminado.value) {
            finalizarPaso();
        }
    }
});

onUnmounted(() => unsubscribe());
</script>

<template>
    <div class="duelo-container">
        <div class="players-display">
            <div class="player-card" :class="{
                active: turnoActualLanzando === 1 && !dueloTerminado,
                winner: dueloTerminado && dadoJ1! > dadoJ2!
            }">
                <span class="nickname">{{ gameStore.jugador1.nickname }}</span>
                <div class="result-box" :class="{ empty: !dadoJ1 }">{{ dadoJ1 || '?' }}</div>
                <p v-if="turnoActualLanzando === 1 && !rolling && !dueloTerminado" class="hw-tip">Presiona TURNO 🎲</p>
            </div>

            <div class="dice-zone">
                <div class="scene">
                    <div class="dice" :style="diceStyle">
                        <div class="face front">1</div>
                        <div class="face back">6</div>
                        <div class="face right">2</div>
                        <div class="face left">5</div>
                        <div class="face top">3</div>
                        <div class="face bottom">4</div>
                    </div>
                </div>

                <div class="controls-overlay">
                    <button v-if="!rolling && !dueloTerminado" @click="rollDice" class="btn-roll">
                        Lanzar (Botón Turno 🎲)
                    </button>

                    <div v-if="dadoJ1 !== null && dadoJ1 === dadoJ2 && !rolling" class="empate-notice">
                        <p>¡EMPATE!</p>
                        <button @click="reiniciarDuelo" class="btn-retry">Repetir (🔘 SELECT)</button>
                    </div>
                </div>
            </div>

            <div class="player-card" :class="{
                active: turnoActualLanzando === 2 && !dueloTerminado,
                winner: dueloTerminado && dadoJ2! > dadoJ1!
            }">
                <span class="nickname">{{ gameStore.jugador2.nickname }}</span>
                <div class="result-box" :class="{ empty: !dadoJ2 }">{{ dadoJ2 || '?' }}</div>
                <p v-if="turnoActualLanzando === 2 && !rolling && !dueloTerminado" class="hw-tip">Presiona TURNO 🎲</p>
            </div>
        </div>

        <div v-if="dueloTerminado && dadoJ1 !== dadoJ2 && !rolling" class="action-area animate-up">
            <h3 class="winner-announcement">
                🏆 {{ dadoJ1! > dadoJ2! ? gameStore.jugador1.nickname : gameStore.jugador2.nickname }} empieza la ronda
            </h3>
            <button @click="finalizarPaso" class="btn-next">
                Continuar a Categoría (🔘 SELECT)
            </button>
        </div>
    </div>
</template>

<style scoped>
.hw-tip {
    font-size: 0.82rem;
    color: #f1c40f;
    font-weight: 900;
    margin-top: 8px;
    letter-spacing: 0.5px;
    animation: pulse 1.4s infinite;
    text-transform: uppercase;
}

@keyframes pulse {
    0% {
        opacity: 0.35;
    }

    50% {
        opacity: 1;
    }

    100% {
        opacity: 0.35;
    }
}

.duelo-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    width: 100%;
    max-width: 1100px;
    gap: 30px;
    margin: 0 auto;
}

.players-display {
    display: flex;
    justify-content: space-between;
    align-items: center;
    width: 100%;
    gap: 28px;
}

/* =========================
   TARJETAS DE JUGADOR
========================= */
.player-card {
    background: rgba(255, 255, 255, 0.06);
    padding: 22px 18px;
    border-radius: 24px;
    text-align: center;
    width: 220px;
    transition: all 0.35s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    border: 3px solid rgba(255, 255, 255, 0.08);
    box-shadow:
        0 18px 30px rgba(0, 0, 0, 0.22),
        0 0 14px rgba(255, 255, 255, 0.03);
    backdrop-filter: blur(8px);
}

.player-card.active {
    border-color: #f1c40f;
    transform: scale(1.08);
    box-shadow:
        0 0 20px rgba(241, 196, 15, 0.22),
        0 18px 30px rgba(0, 0, 0, 0.24);
}

.player-card.winner {
    border-color: #2ecc71;
    background: rgba(46, 204, 113, 0.12);
    box-shadow:
        0 0 18px rgba(46, 204, 113, 0.18),
        0 18px 30px rgba(0, 0, 0, 0.22);
}

.nickname {
    font-weight: 900;
    color: #ffffff;
    font-size: 1.2rem;
    margin-bottom: 14px;
    display: block;
    text-transform: capitalize;
    letter-spacing: 0.4px;
}

.result-box {
    font-size: 4rem;
    color: #f5f6fa;
    font-weight: 900;
    line-height: 1;
    text-shadow: 0 4px 14px rgba(0, 0, 0, 0.35);
}

.result-box.empty {
    color: rgba(255, 255, 255, 0.22);
}

/* =========================
   ZONA DEL DADO
========================= */
.dice-zone {
    flex: 1;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    gap: 18px;
}

.scene {
    width: 120px;
    height: 120px;
    perspective: 1100px;
    display: flex;
    justify-content: center;
    align-items: center;
    margin-bottom: 8px;
}

.dice {
    width: 110px;
    height: 110px;
    position: relative;
    transform-style: preserve-3d;
    transition: transform 2s cubic-bezier(0.2, 0.8, 0.3, 1);
}

.face {
    position: absolute;
    width: 110px;
    height: 110px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 2.4rem;
    font-weight: 900;
    border-radius: 18px;
    border: 3px solid rgba(255, 255, 255, 0.12);
    box-shadow:
        inset 0 0 20px rgba(0, 0, 0, 0.22),
        0 10px 24px rgba(0, 0, 0, 0.18);
    color: white;
    background: linear-gradient(135deg, #2c3e50, #1e272e);
}

.front {
    transform: rotateY(0deg) translateZ(55px);
}

.back {
    transform: rotateY(180deg) translateZ(55px);
}

.right {
    transform: rotateY(90deg) translateZ(55px);
}

.left {
    transform: rotateY(-90deg) translateZ(55px);
}

.top {
    transform: rotateX(90deg) translateZ(55px);
}

.bottom {
    transform: rotateX(-90deg) translateZ(55px);
}

/* =========================
   CONTROLES
========================= */
.controls-overlay {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 14px;
}

.btn-roll,
.btn-next,
.btn-retry {
    padding: 15px 34px;
    border: none;
    border-radius: 999px;
    font-weight: 900;
    cursor: pointer;
    font-size: 1rem;
    letter-spacing: 0.4px;
    transition: all 0.25s ease;
}

.btn-roll {
    background: linear-gradient(135deg, #3498db, #2980b9);
    color: white;
    box-shadow:
        0 10px 20px rgba(52, 152, 219, 0.24),
        0 0 14px rgba(52, 152, 219, 0.14);
}

.btn-roll:hover {
    transform: translateY(-2px) scale(1.02);
}

.btn-next {
    background: linear-gradient(135deg, #2ecc71, #27ae60);
    color: white;
    box-shadow:
        0 10px 20px rgba(46, 204, 113, 0.24),
        0 0 14px rgba(46, 204, 113, 0.14);
}

.btn-next:hover {
    transform: translateY(-2px) scale(1.02);
}

.btn-retry {
    background: linear-gradient(135deg, #f1c40f, #f39c12);
    color: #111;
    box-shadow:
        0 10px 20px rgba(241, 196, 15, 0.24),
        0 0 14px rgba(241, 196, 15, 0.14);
}

.btn-retry:hover {
    transform: translateY(-2px) scale(1.02);
}

/* =========================
   EMPATE / GANADOR
========================= */
.empate-notice {
    text-align: center;
    padding: 14px 20px;
    border-radius: 18px;
    background: rgba(241, 196, 15, 0.12);
    border: 2px solid rgba(241, 196, 15, 0.35);
    color: #fef9c3;
    box-shadow: 0 0 16px rgba(241, 196, 15, 0.12);
}

.empate-notice p {
    margin: 0 0 10px;
    font-size: 1.1rem;
    font-weight: 900;
    letter-spacing: 1px;
}

.action-area {
    text-align: center;
    animation: fadeIn 0.4s ease;
    background: rgba(255, 255, 255, 0.05);
    border: 1px solid rgba(46, 204, 113, 0.22);
    border-radius: 24px;
    padding: 24px 28px;
    box-shadow:
        0 18px 32px rgba(0, 0, 0, 0.2),
        0 0 18px rgba(46, 204, 113, 0.08);
}

.winner-announcement {
    margin: 0 0 18px;
    font-size: 1.4rem;
    color: #ffffff;
    font-weight: 900;
    text-shadow: 0 4px 14px rgba(0, 0, 0, 0.25);
}

.shaking {
    display: inline-block;
    animation: shake 0.5s infinite;
}

@keyframes shake {

    0%,
    100% {
        transform: translateX(0);
    }

    25% {
        transform: translateX(-2px);
    }

    75% {
        transform: translateX(2px);
    }
}

@keyframes fadeIn {
    from {
        opacity: 0;
        transform: translateY(12px);
    }

    to {
        opacity: 1;
        transform: translateY(0);
    }
}

/* =========================
   RESPONSIVE
========================= */
@media (max-width: 980px) {
    .players-display {
        flex-direction: column;
        gap: 24px;
    }

    .player-card {
        width: min(280px, 90vw);
    }

    .scene {
        width: 110px;
        height: 110px;
    }

    .dice,
    .face {
        width: 100px;
        height: 100px;
    }

    .front {
        transform: rotateY(0deg) translateZ(50px);
    }

    .back {
        transform: rotateY(180deg) translateZ(50px);
    }

    .right {
        transform: rotateY(90deg) translateZ(50px);
    }

    .left {
        transform: rotateY(-90deg) translateZ(50px);
    }

    .top {
        transform: rotateX(90deg) translateZ(50px);
    }

    .bottom {
        transform: rotateX(-90deg) translateZ(50px);
    }
}
</style>