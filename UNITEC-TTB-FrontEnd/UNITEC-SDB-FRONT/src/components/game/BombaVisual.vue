<script setup lang="ts">
import { computed } from 'vue';

interface Props {
    tiempo: number;
    explotada: boolean;
    pausada?: boolean;
}

const props = withDefaults(defineProps<Props>(), {
    pausada: false
});

const escalaBomba = computed(() => {
    if (props.explotada) return 1.5;

    if (props.tiempo <= 7) {
        const progreso = (7 - props.tiempo) / 7;
        return 1 + (progreso * 0.4);
    }

    return 1;
});

const estaVibrando = computed(() => props.tiempo <= 7 && !props.explotada && !props.pausada);
const esCritico = computed(() => props.tiempo <= 3 && !props.explotada && !props.pausada);
</script>

<template>
    <div class="bomb-wrapper">
        <div class="bomb-container" :class="{
            'is-vibrating': estaVibrando,
            'is-critical': esCritico,
            'has-exploded': props.explotada,
            'is-paused': props.pausada
        }">
            <Transition name="zoom-boom" mode="out-in">
                <img v-if="!props.explotada" key="bomb" src="@/assets/bomba.png" class="bomb-img"
                    :style="{ transform: `scale(${escalaBomba})` }" />

                <img v-else key="explosion" src="@/assets/explosion.png" class="explosion-img" />
            </Transition>
        </div>

        <div v-if="!props.explotada" class="bomb-shadow"
            :style="{ transform: `scale(${escalaBomba * 0.7})`, opacity: 0.3 + (escalaBomba - 1) }"></div>
    </div>
</template>

<style scoped>
.bomb-wrapper {
    position: relative;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    width: 350px;
    height: 350px;
}

.bomb-container {
    z-index: 2;
    display: flex;
    align-items: center;
    justify-content: center;
    transition: transform 0.1s linear;
}

.bomb-img {
    width: 250px;
    filter: drop-shadow(0 20px 40px rgba(0, 0, 0, 0.6));
    will-change: transform;
}

.explosion-img {
    width: 320px;
    filter: drop-shadow(0 0 50px #f39c12);
    /* La animación se maneja con el Transition de Vue */
}

.bomb-shadow {
    position: absolute;
    bottom: 20px;
    width: 150px;
    height: 20px;
    background: rgba(0, 0, 0, 0.4);
    border-radius: 50%;
    filter: blur(8px);
    z-index: 1;
    transition: transform 0.1s linear;
}

/* --- ANIMACIONES DE VIBRACIÓN --- */

.is-vibrating {
    animation: shake-normal 0.2s infinite;
}

.is-critical {
    animation: shake-heavy 0.1s infinite;
}

@keyframes shake-normal {

    0%,
    100% {
        transform: translate(0, 0) rotate(0deg);
    }

    25% {
        transform: translate(2px, -2px) rotate(1deg);
    }

    75% {
        transform: translate(-2px, 2px) rotate(-1deg);
    }
}

@keyframes shake-heavy {

    0%,
    100% {
        transform: translate(0, 0) rotate(0deg);
    }

    20% {
        transform: translate(-4px, 4px) rotate(-2deg);
    }

    40% {
        transform: translate(-4px, -4px) rotate(2deg);
    }

    60% {
        transform: translate(4px, 4px) rotate(-2deg);
    }

    80% {
        transform: translate(4px, -4px) rotate(2deg);
    }
}

/* --- TRANSICIÓN ZOOM-BOOM (CORRECCIÓN CSS) --- */

.zoom-boom-enter-active {
    animation: explosion-sequence 0.7s cubic-bezier(0.175, 0.885, 0.32, 1.275) forwards;
}

@keyframes explosion-sequence {
    0% {
        transform: scale(0) rotate(-20deg);
        opacity: 0;
    }

    50% {
        transform: scale(1.6) rotate(10deg);
        opacity: 1;
    }

    100% {
        transform: scale(1.4) rotate(0deg);
        /* Se mantiene ligeramente grande */
    }
}

.zoom-boom-leave-active {
    transition: opacity 0.2s ease;
}

.zoom-boom-leave-to {
    opacity: 0;
}
</style>