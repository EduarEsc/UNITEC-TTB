<script setup lang="ts">
import { useGameStore } from '@/stores/game';
import { computed, watch, onUnmounted } from 'vue';

interface Props {
    mostrar: boolean;
    nombrePerdedor: string;
    puedeContinuar: boolean;
}

const props = defineProps<Props>();
const gameStore = useGameStore();
const emit = defineEmits(['continuar', 'nuevaPartida', 'irAInicio']);

watch(() => props.mostrar, (val) => {
    if (val) gameStore.resetFocoModal();
});

const opciones = computed(() => {
    const items = [];

    if (props.puedeContinuar) {
        items.push({
            key: 'continuar',
            label: 'SIGUIENTE RONDA',
            variant: 'continue'
        });
    }

    items.push(
        {
            key: 'reintentar',
            label: 'REINTENTAR',
            variant: 'retry'
        },
        {
            key: 'inicio',
            label: 'INICIO',
            variant: 'home'
        }
    );

    return items;
});

const ejecutarAccionSeleccionada = () => {
    const opcion = opciones.value[gameStore.indexBotonModal];
    if (!opcion) return;

    if (opcion.key === 'continuar') emit('continuar');
    else if (opcion.key === 'reintentar') emit('nuevaPartida');
    else if (opcion.key === 'inicio') emit('irAInicio');
};

const seleccionarPorClick = (index: number) => {
    gameStore.indexBotonModal = index;
    ejecutarAccionSeleccionada();
};

const unsubscribe = gameStore.$onAction(({ name }) => {
    if (name === 'confirmarSeleccionMazo' && props.mostrar && gameStore.modalActivo === 'BOOM') {
        ejecutarAccionSeleccionada();
    }
});

onUnmounted(() => {
    unsubscribe();
});

defineExpose({ ejecutarAccionSeleccionada });
</script>

<template>
    <Transition name="modal-bounce">
        <div v-if="props.mostrar" class="modal-overlay">
            <div class="modal-content">
                <div class="boom-badge">💥</div>

                <h2 class="status-msg">¡BOOM!</h2>
                <p class="perdedor-info">
                    <span>{{ props.nombrePerdedor }}</span> perdió una vida
                </p>

                <p class="controls-hint">
                    Usa la palanca 🕹️ para moverte y pulsa <strong>SELECT</strong> para elegir
                </p>

                <div class="button-group-vertical">
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
.modal-overlay {
    position: fixed;
    inset: 0;
    background: rgba(3, 6, 12, 0.82);
    backdrop-filter: blur(10px);
    -webkit-backdrop-filter: blur(10px);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 10000;
    padding: 24px;
}

.modal-content {
    position: relative;
    width: min(92vw, 560px);
    padding: 42px 34px 34px;
    border-radius: 30px;
    text-align: center;
    background:
        linear-gradient(145deg, rgba(29, 37, 52, 0.98), rgba(12, 16, 24, 0.98));
    border: 2px solid rgba(231, 76, 60, 0.45);
    box-shadow:
        0 28px 60px rgba(0, 0, 0, 0.42),
        0 0 30px rgba(231, 76, 60, 0.16);
    overflow: hidden;
}

.boom-badge {
    font-size: 3.8rem;
    margin-bottom: 8px;
    filter: drop-shadow(0 6px 14px rgba(231, 76, 60, 0.35));
}

.status-msg {
    font-size: 3rem;
    font-weight: 900;
    color: #ff6b57;
    margin: 0;
    text-shadow: 0 0 20px rgba(231, 76, 60, 0.3);
    letter-spacing: 3px;
}

.perdedor-info {
    font-size: 1.12rem;
    margin: 14px 0 12px;
    color: #cfd8e3;
}

.perdedor-info span {
    color: #ffffff;
    font-weight: 900;
    text-transform: uppercase;
    border-bottom: 2px solid #ff6b57;
}

.controls-hint {
    margin: 0 0 24px;
    color: #9fb0c7;
    font-size: 0.96rem;
    font-weight: 700;
}

.controls-hint strong {
    color: #ffe082;
}

.button-group-vertical {
    display: flex;
    flex-direction: column;
    gap: 14px;
}

.btn-game {
    position: relative;
    width: 100%;
    min-height: 68px;
    border: none;
    border-radius: 18px;
    display: flex;
    align-items: center;
    gap: 14px;
    padding: 16px 18px;
    cursor: pointer;
    font-size: 1rem;
    font-weight: 900;
    text-transform: uppercase;
    letter-spacing: 0.9px;
    transition:
        transform 0.2s ease,
        box-shadow 0.2s ease,
        border-color 0.2s ease,
        background 0.2s ease;
    border: 2px solid transparent;
}

.btn-index {
    width: 34px;
    height: 34px;
    border-radius: 999px;
    display: inline-flex;
    align-items: center;
    justify-content: center;
    font-size: 0.92rem;
    font-weight: 900;
    background: rgba(255, 255, 255, 0.12);
    color: white;
    flex-shrink: 0;
}

.btn-label {
    flex: 1;
    text-align: left;
}

.btn-continue {
    background: linear-gradient(135deg, #22c55e, #15803d);
    color: white;
    box-shadow: 0 10px 20px rgba(34, 197, 94, 0.18);
}

.btn-retry {
    background: linear-gradient(135deg, #3b82f6, #1d4ed8);
    color: white;
    box-shadow: 0 10px 20px rgba(59, 130, 246, 0.18);
}

.btn-home {
    background: linear-gradient(135deg, #6b7280, #374151);
    color: white;
    box-shadow: 0 10px 20px rgba(107, 114, 128, 0.18);
}

.btn-game:hover {
    transform: translateY(-2px);
}

.btn-game.focused {
    transform: scale(1.03);
    border-color: #ffd54f;
    box-shadow:
        0 0 0 4px rgba(255, 213, 79, 0.18),
        0 14px 28px rgba(0, 0, 0, 0.28);
}

.btn-game.focused .btn-index {
    background: #ffd54f;
    color: #1f2937;
    box-shadow: 0 0 14px rgba(255, 213, 79, 0.4);
}

.modal-bounce-enter-active {
    animation: modalBounceIn 0.35s ease;
}

.modal-bounce-leave-active {
    animation: modalBounceOut 0.24s ease;
}

@keyframes modalBounceIn {
    0% {
        opacity: 0;
        transform: scale(0.86);
    }

    100% {
        opacity: 1;
        transform: scale(1);
    }
}

@keyframes modalBounceOut {
    0% {
        opacity: 1;
        transform: scale(1);
    }

    100% {
        opacity: 0;
        transform: scale(0.92);
    }
}

@media (max-width: 768px) {
    .modal-content {
        padding: 34px 20px 24px;
    }

    .status-msg {
        font-size: 2.3rem;
    }

    .btn-game {
        min-height: 62px;
        padding: 14px 14px;
    }

    .btn-label {
        font-size: 0.92rem;
    }
}
</style>