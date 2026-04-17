<script setup lang="ts">
import { ref, computed, onUnmounted } from 'vue';
import { useGameStore } from '@/stores/game';

const gameStore = useGameStore();
const emit = defineEmits(['seleccionado']);

const rolling = ref(false);
const result = ref<string | null>(null);
const currentRotation = ref({ x: 0, y: 0 });

const faceValues: Record<number, string> = {
    1: 'TIC', 2: 'BUM', 3: 'TAC',
    4: 'BUM', 5: 'TIC', 6: 'TAC'
};

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
    if (rolling.value || result.value) return;

    rolling.value = true;

    const randomFace = Math.floor(Math.random() * 6) + 1;
    const targetRotation = rotations[randomFace];

    if (targetRotation) {
        currentRotation.value = {
            x: targetRotation.x + 1440,
            y: targetRotation.y + 1800
        };
    }

    setTimeout(async () => {
        rolling.value = false;
        result.value = faceValues[randomFace] ?? null;

        if (result.value) {
            gameStore.categoriaActual = result.value;
            await gameStore.prepararMazoTrasDado(result.value);
            // OJO: NO cambiamos configStep aquí, eso lo hace el padre al confirmar
        }

        currentRotation.value.x %= 360;
        currentRotation.value.y %= 360;
    }, 2000);
};

const confirmarCategoria = () => {
    if (rolling.value) return
    if (!result.value) return
    if (!gameStore.mazo.length) return
    if (gameStore.configStep !== 'CATEGORIA') return

    emit('seleccionado', result.value)
}

const unsubscribe = gameStore.$onAction(({ name }) => {
    if (name === 'lanzarDadoCategoria') {
        rollDice();
    }

    if (name === 'confirmarSeleccionMazo') {
        // SOLO confirmar categoría si el subpaso actual es CATEGORIA
        if (gameStore.configStep === 'CATEGORIA') {
            confirmarCategoria();
        }
    }
});

onUnmounted(() => unsubscribe());
</script>

<template>
    <div class="categoria-container">
        <h2 class="step-title">Gira para conocer tu destino</h2>

        <div class="dice-wrapper">
            <div class="scene">
                <div class="dice" :style="diceStyle">
                    <div class="face front">TIC</div>
                    <div class="face back">TAC</div>
                    <div class="face right">BUM</div>
                    <div class="face left">TIC</div>
                    <div class="face top">TAC</div>
                    <div class="face bottom">BUM</div>
                </div>
            </div>
        </div>

        <div class="ui-overlay">
            <button v-if="!result" @click="rollDice" :disabled="rolling" class="btn-roll-cat">
                <span v-if="!rolling">Girar Dado (Botón Categoría 🎲)</span>
                <span v-else class="shaking">¡Eligiendo...!</span>
            </button>

            <transition name="scale-fade">
                <div v-if="result && !rolling" class="result-box-cat">
                    <div class="category-badge" :class="result.toLowerCase()">
                        <p class="cat-explanation">
                            {{ result === 'TIC' ? 'La sílaba debe ir al inicio' :
                                result === 'TAC' ? 'La sílaba debe estar al centro' :
                                    'La sílaba debe ir al final' }}
                        </p>
                    </div>

                    <button @click="confirmarCategoria" class="btn-confirm-cat">
                        Siguiente (🔘 SELECT)
                    </button>
                </div>
            </transition>
        </div>
    </div>
</template>

<style scoped>
.categoria-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
    padding: 1px 12px;
    width: 100%;
    color: #ecf0f1;
}

.step-title {
    margin: 0;
    font-size: 2rem;
    font-weight: 900;
    color: #ffffff;
    letter-spacing: 0.5px;
    text-align: center;
    text-shadow: 0 4px 14px rgba(0, 0, 0, 0.3);
}

.dice-wrapper {
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 180px;
}

.scene {
    width: 150px;
    height: 150px;
    perspective: 1200px;
    display: flex;
    justify-content: center;
    align-items: center;
}

.dice {
    width: 150px;
    height: 150px;
    position: relative;
    transform-style: preserve-3d;
    transition: transform 2s cubic-bezier(0.2, 0.8, 0.3, 1);
}

.face {
    position: absolute;
    width: 150px;
    height: 150px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 2.3rem;
    font-weight: 900;
    border-radius: 22px;
    border: 4px solid rgba(255, 255, 255, 0.14);
    box-shadow:
        inset 0 0 22px rgba(0, 0, 0, 0.24),
        0 12px 26px rgba(0, 0, 0, 0.2);
    color: white;
    text-shadow: 0 3px 8px rgba(0, 0, 0, 0.28);
}

/* TIC = rojo */
.front,
.left {
    background: linear-gradient(135deg, #e74c3c, #c0392b);
}

/* TAC = azul */
.top,
.back {
    background: linear-gradient(135deg, #3498db, #2980b9);
}

/* BUM = oscuro */
.right,
.bottom {
    background: linear-gradient(135deg, #2c3e50, #1f2a36);
}

.front {
    transform: rotateY(0deg) translateZ(75px);
}

.back {
    transform: rotateY(180deg) translateZ(75px);
}

.right {
    transform: rotateY(90deg) translateZ(75px);
}

.left {
    transform: rotateY(-90deg) translateZ(75px);
}

.top {
    transform: rotateX(90deg) translateZ(75px);
}

.bottom {
    transform: rotateX(-90deg) translateZ(75px);
}

/* =========================
   UI DE CONTROLES
========================= */
.ui-overlay {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 18px;
}

.btn-roll-cat,
.btn-confirm-cat {
    padding: 15px 34px;
    border: none;
    border-radius: 999px;
    font-weight: 900;
    cursor: pointer;
    font-size: 1rem;
    letter-spacing: 0.4px;
    transition: all 0.25s ease;
}

.btn-roll-cat {
    background: linear-gradient(135deg, #3498db, #2980b9);
    color: white;
    box-shadow:
        0 10px 20px rgba(52, 152, 219, 0.24),
        0 0 14px rgba(52, 152, 219, 0.14);
}

.btn-roll-cat:hover:not(:disabled) {
    transform: translateY(-2px) scale(1.02);
}

.btn-roll-cat:disabled {
    opacity: 0.75;
    cursor: not-allowed;
}

.btn-confirm-cat {
    background: linear-gradient(135deg, #2ecc71, #27ae60);
    color: white;
    box-shadow:
        0 10px 20px rgba(46, 204, 113, 0.24),
        0 0 14px rgba(46, 204, 113, 0.14);
}

.btn-confirm-cat:hover {
    transform: translateY(-2px) scale(1.02);
}

/* =========================
   RESULTADO
========================= */
.result-box-cat {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 18px;
    animation: fadeUp 0.35s ease;
}

.category-badge {
    min-width: 260px;
    max-width: 340px;
    text-align: center;
    padding: 20px 24px;
    border-radius: 22px;
    color: white;
    box-shadow:
        0 16px 30px rgba(0, 0, 0, 0.22),
        0 0 18px rgba(255, 255, 255, 0.04);
    border: 2px solid rgba(255, 255, 255, 0.1);
    transition: transform 0.25s ease;
}

.category-badge:hover {
    transform: translateY(-3px);
}

.category-badge.tic {
    background: linear-gradient(135deg, rgba(231, 76, 60, 0.95), rgba(192, 57, 43, 0.95));
}

.category-badge.tac {
    background: linear-gradient(135deg, rgba(52, 152, 219, 0.95), rgba(41, 128, 185, 0.95));
}

.category-badge.bum {
    background: linear-gradient(135deg, rgba(44, 62, 80, 0.95), rgba(31, 42, 54, 0.95));
}

.cat-name {
    display: block;
    font-size: 2rem;
    font-weight: 900;
    letter-spacing: 2px;
    margin-bottom: 8px;
}

.cat-explanation {
    margin: 0;
    font-size: 0.98rem;
    font-weight: 700;
    line-height: 1.4;
    color: rgba(255, 255, 255, 0.92);
}

/* =========================
   ANIMACIONES
========================= */
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
        transform: translateX(-3px);
    }

    75% {
        transform: translateX(3px);
    }
}

@keyframes fadeUp {
    from {
        opacity: 0;
        transform: translateY(14px);
    }

    to {
        opacity: 1;
        transform: translateY(0);
    }
}

/* transición del resultado */
.scale-fade-enter-active,
.scale-fade-leave-active {
    transition: all 0.3s ease;
}

.scale-fade-enter-from {
    opacity: 0;
    transform: scale(0.9) translateY(10px);
}

.scale-fade-leave-to {
    opacity: 0;
    transform: scale(0.95) translateY(-8px);
}

/* =========================
   RESPONSIVE
========================= */
@media (max-width: 768px) {
    .step-title {
        font-size: 1.6rem;
    }

    .scene,
    .dice,
    .face {
        width: 120px;
        height: 120px;
    }

    .front {
        transform: rotateY(0deg) translateZ(60px);
    }

    .back {
        transform: rotateY(180deg) translateZ(60px);
    }

    .right {
        transform: rotateY(90deg) translateZ(60px);
    }

    .left {
        transform: rotateY(-90deg) translateZ(60px);
    }

    .top {
        transform: rotateX(90deg) translateZ(60px);
    }

    .bottom {
        transform: rotateX(-90deg) translateZ(60px);
    }

    .category-badge {
        min-width: 220px;
        padding: 18px 20px;
    }

    .cat-name {
        font-size: 1.6rem;
    }
}
</style>