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
/* Tus estilos base se mantienen, añadimos feedback de hardware */
.hw-tip {
    font-size: 0.8rem;
    color: #3e8ebf;
    font-weight: bold;
    margin-top: 5px;
    animation: pulse 1.5s infinite;
}

@keyframes pulse {
    0% {
        opacity: 0.4;
    }

    50% {
        opacity: 1;
    }

    100% {
        opacity: 0.4;
    }
}

.duelo-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    width: 1000px;
    gap: 30px;
}


.btn-retry {
    background: #f1c40f;
    color: #333;
    padding: 15px 25px;
    border-radius: 30px;
    border: none;
    font-weight: bold;
}

.btn-next {
    background: #27ae60;
    color: white;
    padding: 20px 50px;
    font-size: 1.4rem;
    border-radius: 50px;
    border: none;
    cursor: pointer;
    box-shadow: 0 8px 15px rgba(39, 174, 96, 0.3);
}

.duelo-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    width: 1000px;
    gap: 30px;
}

.players-display {
    display: flex;
    justify-content: space-around;
    align-items: center;
    width: 100%;
    margin-left: 170px;
}

.player-card {
    background: white;
    padding: 5px;
    border-radius: 20px;
    text-align: center;
    width: 180px;
    transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    border: 4px solid #fdf2f2;
    box-shadow: 0 10px 20px rgba(0, 0, 0, 0.05);
}

.player-card.active {
    border-color: #3e8ebf;
    transform: scale(1.1);
    box-shadow: 0 15px 30px rgba(62, 142, 191, 0.2);
}

.player-card.winner {
    border-color: #42b883;
    background: #f0fff4;
}

.nickname {
    font-weight: 900;
    color: #733131;
    font-size: 1.2rem;
    margin-bottom: 15px;
    display: block;
}

.result-box {
    font-size: 4rem;
    color: #a64444;
    font-weight: 900;
    line-height: 1;
}

.dice-zone {
    display: flex;
    flex-direction: column;
    align-items: center;
    /* Centra horizontalmente todo el contenido (dado y botón) */
    width: 100%;
}

/* DISEÑO DADO IGUAL AL CATEGORIA */
.scene {
    width: 100px;
    height: 100px;
    perspective: 1000px;
    margin-bottom: 20px;
    /* Eliminamos márgenes laterales automáticos si el padre ya usa Flexbox, 
       pero podemos asegurar con: */
    display: flex;
    justify-content: center;
}

.dice {
    width: 100px;
    /* Es mejor que tenga el mismo ancho que la escena */
    height: 100px;
    position: relative;
    transform-style: preserve-3d;
    transition: transform 2s cubic-bezier(0.2, 0.8, 0.3, 1);
}

.face {
    position: absolute;
    width: 100px;
    height: 100px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 2.2rem;
    font-weight: 900;
    border-radius: 15px;
    border: 3px solid #fdf2f2;
    box-shadow: inset 0 0 15px rgba(0, 0, 0, 0.2);
    color: white;
    background: #2c3e50;
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

.btn-roll,
.btn-next,
.btn-retry {
    padding: 15px 35px;
    border: none;
    border-radius: 30px;
    font-weight: bold;
    cursor: pointer;
    font-size: 1.1rem;
    transition: all 0.3s;
}

.btn-roll {
    background: #3e8ebf;
    color: white;
}

.btn-next {
    background: #a64444;
    color: white;
    margin-top: 1px;
    box-shadow: 0 5px 15px rgba(166, 68, 68, 0.3);
}

.btn-retry {
    background: #f1c40f;
    color: #333;
    margin-left: -40px;
}

.winner-text {
    font-size: 1.5rem;
    color: #a64444;
    font-weight: bold;
}

.action-area {
    margin-left: 200px;
    text-align: center;
    animation: fadeIn 0.5s ease;
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
        transform: translateY(10px);
    }

    to {
        opacity: 1;
        transform: translateY(0);
    }
}
</style>