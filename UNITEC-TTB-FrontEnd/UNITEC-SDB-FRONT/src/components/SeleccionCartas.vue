<script setup lang="ts">
import { onMounted, onUnmounted } from 'vue'
import { useGameStore } from '@/stores/game'
import RevelacionCarta from './RevelacionCarta.vue'

const gameStore = useGameStore()

const handleKeyDown = (e: KeyboardEvent) => {
    if (gameStore.cartaConfirmada) return
    if (gameStore.configStep !== 'CARTA') return

    if (["ArrowLeft", "ArrowRight", "Enter"].includes(e.key)) {
        e.preventDefault()
    }

    if (e.key === 'ArrowRight') {
        gameStore.cambiarFocoMazo('NEXT')
    } else if (e.key === 'ArrowLeft') {
        gameStore.cambiarFocoMazo('PREV')
    } else if (e.key === 'Enter') {
        confirmarSeleccion()
    }
}

const confirmarSeleccion = () => {
    if (gameStore.configStep !== 'CARTA') return
    if (gameStore.cartaConfirmada) return
    if (!gameStore.mazo.length) return

    gameStore.confirmarCartaFinal()
    console.log("🃏 Carta seleccionada:", gameStore.silabaActual)
}

const unsubscribe = gameStore.$onAction(({ name }) => {
    if (name === 'confirmarSeleccionMazo') {
        if (gameStore.configStep === 'CARTA' && !gameStore.cartaConfirmada) {
            confirmarSeleccion()
        }
    }
})

onMounted(async () => {
    if (gameStore.mazo.length === 0 && gameStore.categoriaActual) {
        await gameStore.prepararMazoTrasDado(gameStore.categoriaActual)
        gameStore.setConfigStep('CARTA')
    }

    gameStore.indexEnFoco = 0
    window.addEventListener('keydown', handleKeyDown)
})

onUnmounted(() => {
    window.removeEventListener('keydown', handleKeyDown)
    unsubscribe()
})

const hoverCarta = (index: number) => {
    if (!gameStore.cartaConfirmada && gameStore.configStep === 'CARTA') {
        gameStore.indexEnFoco = index
    }
}
</script>

<template>
    <div class="contenedor-principal" v-if="!gameStore.cartaConfirmada">
        <h2 class="setup-title">Selecciona una carta</h2>
        <p class="hw-helper">Mueve la palanca 🕹️ y pulsa SELECT 🔘</p>

        <div class="grid-cartas">
            <div v-for="(carta, i) in gameStore.mazo" :key="i" class="card-item"
                :class="{ 'is-focused': i === gameStore.indexEnFoco }" @mouseenter="hoverCarta(i)"
                @click="confirmarSeleccion">
                <div class="card-face">
                    <div class="card-pattern"></div>
                    <img src="@/assets/bomba_icon.png" alt="bomb" class="bomb-img" />
                    <div class="card-back-design">?</div>
                </div>
            </div>
        </div>

        <div class="info-footer">
            <div class="indicator">
                <span class="dot" v-for="(_, i) in gameStore.mazo" :key="i"
                    :class="{ active: i === gameStore.indexEnFoco }"></span>
            </div>
            <p class="contador-txt">Opción {{ gameStore.indexEnFoco + 1 }} de {{ gameStore.mazo.length }}</p>
        </div>
    </div>

    <Teleport to="body">
        <RevelacionCarta v-if="gameStore.cartaConfirmada" />
    </Teleport>
</template>

<style scoped>
.contenedor-principal {
    width: 100%;
    min-height: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 12px 12px 20px;
    color: #ecf0f1;
    border-radius: 28px;
}

.setup-title {
    margin: 0 0 8px;
    font-size: 2rem;
    font-weight: 900;
    color: #ffffff;
    letter-spacing: 0.5px;
    text-align: center;
    text-shadow: 0 4px 14px rgba(0, 0, 0, 0.3);
}

.hw-helper {
    margin: 0 0 20px;
    color: #bdc3c7;
    font-weight: 700;
    font-style: italic;
    text-align: center;
}

.grid-cartas {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: 18px;
    width: 100%;
    max-width: 1000px;
    margin: 18px 0 24px;
}

.card-item {
    width: 96px;
    height: 128px;
    background: linear-gradient(145deg, #2c3e50, #1f2a36);
    border: 3px solid rgba(255, 255, 255, 0.08);
    border-radius: 18px;
    transition: all 0.22s ease-out;
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    position: relative;
    box-shadow:
        0 12px 20px rgba(0, 0, 0, 0.24),
        inset 0 0 10px rgba(255, 255, 255, 0.02);
    overflow: hidden;
}

.card-item::before {
    content: "";
    position: absolute;
    inset: 0;
    background: linear-gradient(180deg, rgba(255, 255, 255, 0.08), rgba(255, 255, 255, 0));
    pointer-events: none;
}

.card-item.is-focused {
    transform: scale(1.14) translateY(-10px);
    border-color: #f1c40f !important;
    box-shadow:
        0 0 22px rgba(241, 196, 15, 0.22),
        0 16px 28px rgba(0, 0, 0, 0.3);
    z-index: 10;
}

.card-item:hover:not(.is-focused) {
    transform: translateY(-4px);
    border-color: rgba(241, 196, 15, 0.26);
    box-shadow:
        0 14px 22px rgba(0, 0, 0, 0.26),
        0 0 14px rgba(241, 196, 15, 0.08);
}

.card-face {
    width: 100%;
    height: 100%;
    position: relative;
    display: flex;
    justify-content: center;
    align-items: center;
}

.card-pattern {
    position: absolute;
    inset: 0;
    background-image:
        radial-gradient(rgba(255, 255, 255, 0.08) 1px, transparent 1px);
    background-size: 16px 16px;
    opacity: 0.5;
    z-index: 1;
}

.bomb-img {
    width: 60%;
    z-index: 2;
    filter: drop-shadow(0 5px 10px rgba(0, 0, 0, 0.35));
}

.card-back-design {
    position: absolute;
    bottom: 8px;
    right: 10px;
    font-size: 1.3rem;
    font-weight: 900;
    color: rgba(255, 255, 255, 0.25);
    z-index: 2;
}

.info-footer {
    text-align: center;
    color: #bdc3c7;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
}

.indicator {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: 8px;
    max-width: 520px;
}

.dot {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: rgba(255, 255, 255, 0.16);
    transition: all 0.22s ease;
    box-shadow: 0 0 0 transparent;
}

.dot.active {
    background: #f1c40f;
    transform: scale(1.25);
    box-shadow: 0 0 12px rgba(241, 196, 15, 0.3);
}

.contador-txt {
    font-size: 1rem;
    font-weight: 800;
    margin: 0;
    color: #ecf0f1;
    letter-spacing: 0.4px;
}

@media (max-width: 768px) {
    .setup-title {
        font-size: 1.6rem;
    }

    .grid-cartas {
        gap: 14px;
    }

    .card-item {
        width: 82px;
        height: 112px;
        border-radius: 16px;
    }

    .contador-txt {
        font-size: 0.92rem;
    }
}
</style>