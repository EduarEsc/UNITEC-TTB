<script setup lang="ts">
import { useGameStore } from '@/stores/game';
import { watch } from 'vue';

interface Props {
    mostrar: boolean;
    ganadorNickname: string;
}

const props = defineProps<Props>();
const gameStore = useGameStore();
const emit = defineEmits(['reiniciar', 'salir']);

watch(() => props.mostrar, (val) => {
    if (val) gameStore.resetFocoModal();
});

const ejecutarAccion = () => {
    if (gameStore.indexBotonModal === 0) emit('reiniciar');
    else emit('salir');
};

defineExpose({ ejecutarAccion });
</script>

<template>
    <Transition name="winner-zoom">
        <div v-if="props.mostrar" class="winner-overlay">
            <div class="winner-content">
                <h1 class="victory-title">¡VICTORIA!</h1>
                <h2 class="winner-name">{{ props.ganadorNickname }}</h2>

                <div class="button-group">
                    <button class="btn-game" :class="{ 'focused': gameStore.indexBotonModal === 0 }">
                        OTRA PARTIDA
                    </button>
                    <button class="btn-game" :class="{ 'focused': gameStore.indexBotonModal === 1 }">
                        SALIR
                    </button>
                </div>
            </div>
        </div>
    </Transition>
</template>

<style scoped>
.winner-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: rgba(0, 0, 0, 0.9);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 200;
    backdrop-filter: blur(8px);
}

.winner-content {
    text-align: center;
    background: linear-gradient(145deg, #1a1a1a, #000);
    padding: 60px;
    border-radius: 40px;
    border: 4px solid #f1c40f;
    box-shadow: 0 0 60px rgba(241, 196, 15, 0.4);
    position: relative;
    overflow: hidden;
    max-width: 550px;
    width: 90%;
}

/* Efecto de rayos de sol girando detrás del trofeo */
.sunburst-bg {
    position: absolute;
    top: 50%;
    left: 50%;
    width: 200%;
    height: 200%;
    background: conic-gradient(from 0deg,
            transparent,
            rgba(241, 196, 15, 0.1),
            transparent 20deg);
    transform: translate(-50%, -50%);
    animation: rotate-bg 10s linear infinite;
    z-index: 1;
}

.winner-header,
.winner-body,
.button-group {
    position: relative;
    z-index: 2;
}

.trophy {
    font-size: 7rem;
    margin-bottom: 10px;
    animation: bounce-trophy 2s infinite;
    filter: drop-shadow(0 0 20px rgba(241, 196, 15, 0.5));
}

.victory-title {
    color: white;
    font-size: 1.5rem;
    letter-spacing: 5px;
    margin-bottom: 10px;
    font-weight: 900;
}

.winner-name {
    font-size: 4rem;
    color: #f1c40f;
    margin: 10px 0;
    text-transform: uppercase;
    text-shadow: 0 0 20px rgba(241, 196, 15, 0.6);
    font-weight: 900;
}

.victory-msg {
    color: #bdc3c7;
    font-size: 1.1rem;
    margin-bottom: 40px;
}

/* --- BOTONES DE VICTORIA --- */
.button-group {
    display: flex;
    gap: 20px;
}

.btn {
    flex: 1;
    padding: 18px;
    border: none;
    border-radius: 15px;
    font-size: 1rem;
    font-weight: 900;
    cursor: pointer;
    transition: all 0.2s ease;
    text-transform: uppercase;
}

.btn-new {
    background: #f1c40f;
    color: #000;
    box-shadow: 0 6px 0 #d4ac0d;
}

.btn-home {
    background: #34495e;
    color: white;
    box-shadow: 0 6px 0 #2c3e50;
}

.btn:active {
    transform: translateY(4px);
    box-shadow: 0 2px 0 transparent;
}

/* --- ANIMACIONES --- */
@keyframes rotate-bg {
    from {
        transform: translate(-50%, -50%) rotate(0deg);
    }

    to {
        transform: translate(-50%, -50%) rotate(360deg);
    }
}

@keyframes bounce-trophy {

    0%,
    100% {
        transform: translateY(0) scale(1);
    }

    50% {
        transform: translateY(-20px) scale(1.1);
    }
}

.winner-zoom-enter-active {
    animation: winner-pop 0.6s cubic-bezier(0.175, 0.885, 0.32, 1.275);
}

@keyframes winner-pop {
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