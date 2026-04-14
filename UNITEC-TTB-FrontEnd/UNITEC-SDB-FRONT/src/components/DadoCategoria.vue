<script setup lang="ts">
import { ref, computed, onUnmounted } from 'vue';
import { useGameStore } from '@/stores/game';

const gameStore = useGameStore();
const emit = defineEmits(['seleccionado']);

const rolling = ref(false);
const result = ref<string | null>(null);
const currentRotation = ref({ x: 0, y: 0 });

// Mapeo de caras para que coincidan con el diseño visual del cubo
const faceValues: Record<number, string> = {
    1: 'TIC', 2: 'BUM', 3: 'TAC',
    4: 'BUM', 5: 'TIC', 6: 'TAC'
};

const rotations: Record<number, { x: number; y: number }> = {
    1: { x: 0, y: 0 },       // TIC
    2: { x: 0, y: -90 },     // BUM
    3: { x: -90, y: 0 },     // TAC
    4: { x: 90, y: 0 },      // BUM
    5: { x: 0, y: 90 },      // TIC
    6: { x: 0, y: 180 }      // TAC
};

const diceStyle = computed(() => ({
    transform: `rotateX(${currentRotation.value.x}deg) rotateY(${currentRotation.value.y}deg)`
}));

const rollDice = () => {
    if (rolling.value || result.value) return;

    rolling.value = true;

    // Generamos un resultado aleatorio
    const randomFace = Math.floor(Math.random() * 6) + 1;
    const targetRotation = rotations[randomFace];

    // Aplicamos rotación con múltiples giros para efecto dramático
    if (targetRotation) { // <--- Esta validación elimina el error
        currentRotation.value = {
            x: targetRotation.x + 1440,
            y: targetRotation.y + 1800
        };
    }

    setTimeout(() => {
        rolling.value = false;
        result.value = faceValues[randomFace] ?? null;

        if (result.value) {
            // Guardamos la categoría en el store de inmediato
            gameStore.categoriaActual = result.value;
            // Pedimos al backend que prepare las sílabas (mazo) basadas en esta categoría
            gameStore.prepararMazoTrasDado(result.value);
        }

        // Limpiamos grados
        currentRotation.value.x %= 360;
        currentRotation.value.y %= 360;
    }, 2000);
};

const confirmarCategoria = () => {
    if (result.value && !rolling.value) {
        // Al confirmar, avisamos al padre (ConfiguracionView) para cambiar al paso 'CARTA'
        emit('seleccionado', result.value);
    }
};

/**
 * --- ESCUCHA DE HARDWARE 🎮 ---
 */
const unsubscribe = gameStore.$onAction(({ name }) => {
    // El botón físico CAT lanza el dado
    if (name === 'lanzarDadoCategoria') {
        rollDice();
    }

    // El botón físico SELECT confirma y avanza
    if (name === 'confirmarSeleccionMazo') {
        confirmarCategoria();
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
                        <span class="cat-name">{{ result }}</span>
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
/* Mantengo tu estilo original y optimizo para visibilidad remota */
.categoria-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 25px;
    padding: 20px;
}

.scene {
    width: 150px;
    height: 130px;
    perspective: 1200px;
}

.dice {
    width: 100%;
    height: 100%;
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
    font-size: 2.2rem;
    font-weight: 900;
    border-radius: 20px;
    border: 5px solid rgba(255, 255, 255, 0.3);
    box-shadow: inset 0 0 30px rgba(0, 0, 0, 0.3);
    color: white;
}

/* Colores temáticos para diferenciar los modos visualmente */
.front,
.left {
    background: #e74c3c;
}

/* TIC */
.top,
.back {
    background: #3498db;
}

/* TAC */
.right,
.bottom {
    background: #2c3e50;
}

/* BUM */

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

.btn-roll {
    margin-top: 10px;
    background: #3e8ebf;
    color: white;
    padding: 15px 40px;
    font-size: 1.3rem;
    border-radius: 50px;
    border: none;
    box-shadow: 0 6px 0 #2980b9;
}

.btn-next {
    background: #27ae60;
    color: white;
    padding: 10px 40px;
    font-size: 1.3rem;
    border-radius: 50px;
    border: none;
    margin-top: 20px;
    box-shadow: 0 6px 0 #1e8449;
    animation: pulse-button 1.5s infinite;
}

@keyframes pulse-button {
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

.result-display {
    margin-left: 75px;
    margin-top: 10px;
    width: 200px;
    padding: 5px 20px;
    border-radius: 20px;
    background: #d0d2d3;
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
    border-left: 10px solid #ccc;
}

.result-display.tic {
    border-color: #e74c3c;
}

.result-display.tac {
    border-color: #3498db;
}

.result-display.bum {
    border-color: #2c3e50;
}

.category-desc {
    font-size: 1rem;
    font-weight: 800;
    color: #333;
    margin: 0;
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
        transform: translateX(-3px);
    }

    75% {
        transform: translateX(3px);
    }
}
</style>
