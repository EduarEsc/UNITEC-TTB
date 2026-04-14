<script setup lang="ts">
import { computed } from 'vue';

interface Props {
    nickname: string;
    puntos: number;
    vidas: number;
    esSuTurno: boolean;
    posicion: 1 | 2; // Para saber si es el jugador de la izquierda o derecha
}

const props = defineProps<Props>();

// Determinamos si el jugador ha perdido (para efectos visuales de derrota)
const haPerdido = computed(() => props.vidas <= 0);
</script>

<template>
    <div class="player-box" :class="{
        'is-turn': props.esSuTurno,
        'is-inactive': !props.esSuTurno && !haPerdido,
        'is-loser': haPerdido
    }">
        <Transition name="fade">
            <div class="turn-tag" v-if="props.esSuTurno">TU TURNO</div>
        </Transition>

        <div class="player-card">
            <span class="avatar">👤</span>
            <div class="info">
                <h3>{{ props.nickname }}</h3>

                <p class="score">
                    Puntos: {{ props.puntos }}
                </p>

                <div class="hearts">
                    <TransitionGroup name="list">
                        <span v-for="n in props.vidas" :key="`heart-${props.posicion}-${n}`" class="heart">
                            ❤️
                        </span>
                    </TransitionGroup>
                    <span v-if="props.vidas === 0" class="no-vidas">💀 ELIMINADO</span>
                </div>
            </div>
        </div>
    </div>
</template>

<style scoped>
.player-box {
    background: rgba(255, 255, 255, 0.05);
    padding: 20px 30px;
    border-radius: 20px;
    border: 3px solid transparent;
    transition: all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    position: relative;
    min-width: 250px;
}

/* --- ESTADOS DINÁMICOS --- */

/* Brillo y escala para el jugador que tiene el turno */
.is-turn {
    background: rgba(241, 196, 15, 0.15);
    border-color: #f1c40f;
    transform: scale(1.1);
    box-shadow: 0 0 30px rgba(241, 196, 15, 0.3);
    z-index: 5;
}

/* Opacidad reducida para el jugador en espera */
.is-inactive {
    opacity: 0.4;
    filter: grayscale(0.5);
    transform: scale(0.95);
}

/* Estilo para cuando un jugador pierde todas las vidas */
.is-loser {
    background: rgba(231, 76, 60, 0.2);
    border-color: #e74c3c;
    opacity: 0.8;
}

.turn-tag {
    position: absolute;
    top: -15px;
    left: 50%;
    transform: translateX(-50%);
    background: #f1c40f;
    color: #000;
    padding: 4px 15px;
    border-radius: 10px;
    font-size: 0.75rem;
    font-weight: 900;
    letter-spacing: 1px;
    box-shadow: 0 4px 10px rgba(0, 0, 0, 0.3);
}

.player-card {
    display: flex;
    align-items: center;
    gap: 15px;
}

.avatar {
    font-size: 2.5rem;
    background: rgba(255, 255, 255, 0.1);
    padding: 10px;
    border-radius: 50%;
}

.info h3 {
    margin: 0;
    font-size: 1.4rem;
    text-transform: capitalize;
}

.score {
    margin: 5px 0;
    font-weight: bold;
    color: #bdc3c7;
}

.hearts {
    display: flex;
    gap: 5px;
    height: 25px;
    /* Altura fija para evitar saltos en la UI */
}

.heart {
    font-size: 1.3rem;
    filter: drop-shadow(0 0 5px rgba(231, 76, 60, 0.5));
    display: inline-block;
}

.no-vidas {
    color: #e74c3c;
    font-size: 0.8rem;
    font-weight: 900;
}

/* --- ANIMACIONES VUE --- */
.fade-enter-active,
.fade-leave-active {
    transition: opacity 0.3s ease, transform 0.3s ease;
}

.fade-enter-from,
.fade-leave-to {
    opacity: 0;
    transform: translateX(-50%) translateY(10px);
}

.list-enter-active,
.list-leave-active {
    transition: all 0.5s ease;
}

.list-enter-from,
.list-leave-to {
    opacity: 0;
    transform: scale(0);
}
</style>