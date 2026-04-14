<script setup lang="ts">
import { useGameStore } from '@/stores/game';
import { computed, watch } from 'vue';

interface Props {
    mostrar: boolean;
    nombrePerdedor: string;
    puedeContinuar: boolean;
}

const props = defineProps<Props>();
const gameStore = useGameStore();
const emit = defineEmits(['continuar', 'nuevaPartida', 'irAInicio']);

// Reiniciar foco al mostrar el modal
watch(() => props.mostrar, (val) => {
    if (val) gameStore.resetFocoModal();
});

// Mapeo de acciones según el índice
const ejecutarAccionSeleccionada = () => {
    if (gameStore.indexBotonModal === 0 && props.puedeContinuar) emit('continuar');
    else if (gameStore.indexBotonModal === 1 || (!props.puedeContinuar && gameStore.indexBotonModal === 0)) emit('nuevaPartida');
    else emit('irAInicio');
};

// Exponemos la función para que el padre o el store la llamen al presionar SELECT
defineExpose({ ejecutarAccionSeleccionada });
</script>

<template>
    <Transition name="modal-bounce">
        <div v-if="props.mostrar" class="modal-overlay">
            <div class="modal-content">
                <h2 class="status-msg">¡BOOM!</h2>
                <p class="perdedor-info"><span>{{ props.nombrePerdedor }}</span> perdió una vida</p>

                <div class="button-group-vertical">
                    <button v-if="props.puedeContinuar" class="btn-game"
                        :class="{ 'focused': gameStore.indexBotonModal === 0 }">
                        SIGUIENTE RONDA
                    </button>

                    <button class="btn-game"
                        :class="{ 'focused': (props.puedeContinuar ? gameStore.indexBotonModal === 1 : gameStore.indexBotonModal === 0) }">
                        REINTENTAR
                    </button>

                    <button class="btn-game"
                        :class="{ 'focused': (props.puedeContinuar ? gameStore.indexBotonModal === 2 : gameStore.indexBotonModal === 1) }">
                        INICIO
                    </button>
                </div>
            </div>
        </div>
    </Transition>
</template>

<style scoped>
.modal-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: rgba(0, 0, 0, 0.85);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 100;
    backdrop-filter: blur(5px);
}

.modal-content {
    background: linear-gradient(145deg, #2c3e50, #1a1a1a);
    padding: 50px;
    border-radius: 30px;
    border: 4px solid #e74c3c;
    text-align: center;
    position: relative;
    overflow: hidden;
    box-shadow: 0 0 50px rgba(231, 76, 60, 0.4);
    max-width: 500px;
    width: 90%;
}

.boom-bg {
    position: absolute;
    top: -20px;
    right: -20px;
    font-size: 8rem;
    opacity: 0.1;
    transform: rotate(15deg);
    pointer-events: none;
}

.status-msg {
    font-size: 3.5rem;
    font-weight: 900;
    color: #e74c3c;
    margin: 0;
    text-shadow: 0 0 20px rgba(231, 76, 60, 0.5);
    letter-spacing: 5px;
}

.perdedor-info {
    font-size: 1.2rem;
    margin: 20px 0 40px;
    color: #bdc3c7;
}

.perdedor-info span {
    color: white;
    font-weight: 900;
    text-transform: uppercase;
    border-bottom: 2px solid #e74c3c;
}

/* --- ESTILOS DE BOTONES (Consistentes con tu diseño) --- */
.button-group {
    display: flex;
    flex-direction: column;
    gap: 15px;
}

.btn {
    padding: 18px;
    border: none;
    border-radius: 15px;
    font-size: 1.1rem;
    font-weight: 900;
    cursor: pointer;
    transition: all 0.2s ease;
    text-transform: uppercase;
    letter-spacing: 1px;
}

.btn-continue {
    background: #2ecc71;
    color: white;
    box-shadow: 0 6px 0 #27ae60;
}

.btn-new {
    background: #3498db;
    color: white;
    box-shadow: 0 6px 0 #2980b9;
}

.btn-home {
    background: #95a5a6;
    color: white;
    box-shadow: 0 6px 0 #7f8c8d;
}

.btn:active {
    transform: translateY(4px);
    box-shadow: 0 2px 0 transparent;
}

.btn:hover {
    filter: brightness(1.1);
}

/* --- ANIMACIÓN DEL MODAL --- */
.modal-fade-enter-active {
    animation: modal-in 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
}

.modal-fade-leave-active {
    animation: modal-in 0.3s reverse;
}

@keyframes modal-in {
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