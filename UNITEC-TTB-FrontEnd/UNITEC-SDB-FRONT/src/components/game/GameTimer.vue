<script setup lang="ts">
import { computed } from 'vue';

interface Props {
    tiempo: number;
    explotada: boolean;
}

const props = defineProps<Props>();

// Determina si estamos en los últimos 3 segundos para activar la alerta visual
const esCritico = computed(() => props.tiempo <= 3 && props.tiempo > 0);

// Define la etiqueta de texto según el estado de la bomba
const etiquetaEtado = computed(() => props.explotada ? '¡DETONADA!' : 'SEGUNDOS');
</script>

<template>
    <div class="timer-section">
        <div class="timer-circle" :class="{
            'near-explosion': esCritico,
            'exploded': props.explotada
        }">
            <Transition name="pop" mode="out-in">
                <span :key="props.explotada ? 'boom' : props.tiempo" class="seconds">
                    {{ props.explotada ? '💥' : props.tiempo }}
                </span>
            </Transition>
        </div>

        <p class="timer-label" :class="{ 'label-exploded': props.explotada }">
            {{ etiquetaEtado }}
        </p>
    </div>
</template>

<style scoped>
.timer-section {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    min-width: 150px;
}

.timer-circle {
    width: 120px;
    height: 120px;
    border-radius: 50%;
    border: 8px solid #2ecc71;
    /* Verde inicial */
    display: flex;
    justify-content: center;
    align-items: center;
    background: rgba(0, 0, 0, 0.3);
    transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    position: relative;
    box-shadow: 0 0 15px rgba(46, 204, 113, 0.2);
}

/* Estado de alerta: Rojo y pulsante */
.near-explosion {
    border-color: #e74c3c;
    animation: pulse-red 0.5s infinite;
    box-shadow: 0 0 25px rgba(231, 76, 60, 0.5);
}

/* Estado final: Círculo lleno de color tras la explosión */
.exploded {
    background: #e74c3c;
    border-color: #fff;
    transform: scale(1.2);
    box-shadow: 0 0 40px #e74c3c;
}

.seconds {
    font-size: 3.5rem;
    font-weight: 900;
    color: white;
    text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
}

.timer-label {
    margin-top: 10px;
    font-weight: 900;
    letter-spacing: 2px;
    font-size: 0.8rem;
    color: #bdc3c7;
    transition: color 0.3s ease;
}

.label-exploded {
    color: #e74c3c;
    animation: shake-text 0.2s infinite;
}

/* --- Animaciones y Transiciones --- */

@keyframes pulse-red {
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

@keyframes shake-text {

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

/* Animación de "salto" para los números al cambiar */
.pop-enter-active {
    animation: pop-in 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
}

.pop-leave-active {
    animation: pop-in 0.3s reverse;
}

@keyframes pop-in {
    0% {
        transform: scale(0.5);
        opacity: 0;
    }

    100% {
        transform: scale(1);
        opacity: 1;
    }
}
</style>