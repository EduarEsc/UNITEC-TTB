<script setup lang="ts">
import { onMounted, onUnmounted } from 'vue'
import { useGameStore } from '@/stores/game'
import { sendEvent } from '@/services/websocket'
import RevelacionCarta from './RevelacionCarta.vue'

const gameStore = useGameStore()

/**
 * MANEJADOR DE TECLADO (Para pruebas en PC)
 */
const handleKeyDown = (e: KeyboardEvent) => {
    if (gameStore.cartaConfirmada) return

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

/**
 * PROCESO DE CONFIRMACIÓN
 */
const confirmarSeleccion = () => {
    if (!gameStore.cartaConfirmada) {
        gameStore.confirmarSeleccionMazo()
        // Opcional: Sonido local de "carta seleccionada"
        console.log("🃏 Carta seleccionada:", gameStore.silabaActual)
    }
}

/**
 * ESCUCHA DE HARDWARE 🎮
 */
const unsubscribe = gameStore.$onAction(({ name }) => {
    // Escuchamos el botón físico SELECT
    if (name === 'confirmarSeleccionMazo') {
        confirmarSeleccion()
    }
    // Nota: El movimiento del Joystick (cambiarFocoMazo) se maneja
    // directamente en el Store, y como 'indexEnFoco' es reactivo,
    // la UI se actualizará sola aquí.
})

onMounted(async () => {
    // Aseguramos que el mazo exista (por si se recarga la página)
    if (gameStore.mazo.length === 0 && gameStore.categoriaActual) {
        await gameStore.prepararMazoTrasDado(gameStore.categoriaActual)
    }

    gameStore.indexEnFoco = 0
    window.addEventListener('keydown', handleKeyDown)
})

onUnmounted(() => {
    window.removeEventListener('keydown', handleKeyDown)
    unsubscribe()
})

const hoverCarta = (index: number) => {
    if (!gameStore.cartaConfirmada) {
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

    <RevelacionCarta v-else />
</template>

<style scoped>
.contenedor-principal {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    background: #f9f9f9;
    border-radius: 30px;
    margin-bottom: 20px;
}

.grid-cartas {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: 15px;
    max-width: 950px;
    margin: 20px 0;
}

.card-item {
    width: 90px;
    height: 115px;
    background: #733131;
    border: 4px solid transparent;
    border-radius: 12px;
    transition: all 0.2s ease-out;
    /* Transición más rápida para respuesta inmediata */
    display: flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    position: relative;
}

/* Forzamos que el estilo de "foco" sea el único que resalte la carta */
.card-item.is-focused {
    transform: scale(1.15) translateY(-10px);
    border-color: #f1c40f !important;
    box-shadow: 0 10px 25px rgba(241, 196, 15, 0.4);
    z-index: 10;
}

/* Quitamos efectos de hover nativos para que no choquen con el foco del store */
.card-item:hover:not(.is-focused) {
    border-color: rgba(241, 196, 15, 0.3);
}

.card-face {
    display: flex;
    justify-content: center;
    align-items: center;
}

.bomb-img {
    width: 65%;
    filter: drop-shadow(0 4px 6px rgba(0, 0, 0, 0.2));
}

.info-footer {
    text-align: center;
    color: #555;
}

.contador-txt {
    font-size: 1.2rem;
    font-weight: bold;
    margin: 0;
}
</style>