<script setup lang="ts">
import { useGameStore } from '@/stores/game';
import { computed, watch, onUnmounted } from 'vue';
import { useRouter } from 'vue-router';

interface Props {
    mostrar: boolean;
    ganadorNickname: string;
}

const props = defineProps<Props>();
const gameStore = useGameStore();
const router = useRouter();

watch(() => props.mostrar, (val) => {
    if (val) gameStore.resetFocoModal();
});

const opciones = computed(() => [
    { key: 'reiniciar', label: 'OTRA PARTIDA', variant: 'gold' },
    { key: 'salir', label: 'SALIR', variant: 'dark' }
]);

const ejecutarAccion = () => {
    const opcion = opciones.value[gameStore.indexBotonModal];
    if (!opcion) return;

    gameStore.reiniciarPartidaCompleta();

    if (opcion.key === 'reiniciar') {
        router.push('/dados');
    } else {
        router.push('/reglas');
    }
};

const seleccionarPorClick = (index: number) => {
    gameStore.indexBotonModal = index;
    ejecutarAccion();
};

const unsubscribe = gameStore.$onAction(({ name }) => {
    if (name === 'confirmarSeleccionMazo' && props.mostrar && gameStore.modalActivo === 'WINNER') {
        ejecutarAccion();
    }
});

onUnmounted(() => {
    unsubscribe();
});

defineExpose({ ejecutarAccion });
</script>

<template>
    <Transition name="winner-zoom">
        <div v-if="props.mostrar" class="winner-overlay">
            <div class="winner-content">

                <div class="trophy">🏆</div>

                <h1 class="victory-title">VICTORIA</h1>
                <h2 class="winner-name">{{ props.ganadorNickname }}</h2>

                <p class="controls-hint">
                    Usa la palanca 🕹️ y pulsa <strong>SELECT</strong>
                </p>

                <div class="button-group">
                    <button v-for="(opcion, index) in opciones" :key="opcion.key" class="btn-game" :class="[
                        `btn-${opcion.variant}`,
                        { focused: gameStore.indexBotonModal === index }
                    ]" @click="seleccionarPorClick(index)">
                        <span class="btn-index">{{ index + 1 }}</span>
                        <span class="btn-label">{{ opcion.label }}</span>
                    </button>
                </div>

            </div>
        </div>
    </Transition>
</template>

<style scoped>
.winner-overlay {
    position: fixed;
    inset: 0;
    background: rgba(3, 6, 12, 0.9);
    backdrop-filter: blur(12px);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 11000;
    padding: 24px;
}

.winner-content {
    width: min(92vw, 600px);
    padding: 50px 30px 30px;
    border-radius: 34px;
    text-align: center;
    background: linear-gradient(145deg, #0f172a, #020617);
    border: 2px solid rgba(241, 196, 15, 0.5);
    box-shadow:
        0 30px 70px rgba(0, 0, 0, 0.5),
        0 0 40px rgba(241, 196, 15, 0.2);
    position: relative;
}

.trophy {
    font-size: 5rem;
    margin-bottom: 10px;
    animation: bounce 2s infinite;
    filter: drop-shadow(0 0 20px rgba(241, 196, 15, 0.4));
}

.victory-title {
    font-size: 1.2rem;
    letter-spacing: 6px;
    color: #cbd5e1;
    margin: 0;
}

.winner-name {
    font-size: 3.5rem;
    font-weight: 900;
    color: #facc15;
    margin: 10px 0 20px;
    text-transform: uppercase;
    text-shadow: 0 0 25px rgba(250, 204, 21, 0.5);
}

.controls-hint {
    margin-bottom: 24px;
    color: #94a3b8;
    font-size: 0.95rem;
}

.controls-hint strong {
    color: #fde047;
}

/* BOTONES */
.button-group {
    display: flex;
    gap: 16px;
}

.btn-game {
    flex: 1;
    height: 68px;
    border-radius: 18px;
    border: 2px solid transparent;
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 0 16px;
    font-weight: 900;
    cursor: pointer;
    transition: all 0.2s ease;
}

.btn-index {
    width: 34px;
    height: 34px;
    border-radius: 999px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-weight: 900;
    background: rgba(255, 255, 255, 0.1);
}

.btn-label {
    flex: 1;
    text-align: left;
}

/* VARIANTES */
.btn-gold {
    background: linear-gradient(135deg, #facc15, #ca8a04);
    color: #000;
}

.btn-dark {
    background: linear-gradient(135deg, #334155, #020617);
    color: white;
}

/* FOCO HARDWARE */
.btn-game.focused {
    transform: scale(1.05);
    border-color: #fde047;
    box-shadow:
        0 0 0 4px rgba(253, 224, 71, 0.2),
        0 10px 30px rgba(0, 0, 0, 0.3);
}

.btn-game.focused .btn-index {
    background: #fde047;
    color: black;
}

/* ANIMACIONES */
@keyframes bounce {

    0%,
    100% {
        transform: translateY(0);
    }

    50% {
        transform: translateY(-12px);
    }
}

.winner-zoom-enter-active {
    animation: zoomIn 0.4s ease;
}

@keyframes zoomIn {
    0% {
        transform: scale(0.7);
        opacity: 0;
    }

    100% {
        transform: scale(1);
        opacity: 1;
    }
}
</style>