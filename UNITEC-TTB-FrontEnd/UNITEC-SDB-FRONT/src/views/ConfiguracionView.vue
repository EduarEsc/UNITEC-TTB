<script setup lang="ts">
import { ref, computed, watch } from 'vue';
import { useGameStore } from '@/stores/game';
import { sendEvent } from '@/services/websocket';

// Importación de componentes modulares
import DadoTurno from '@/components/DadoTurno.vue';
import DadoCategoria from '@/components/DadoCategoria.vue';
import SeleccionCartas from '@/components/SeleccionCartas.vue';

const gameStore = useGameStore();

// Estados de la configuración
const pasoActual = ref<'TURNO' | 'CATEGORIA' | 'CARTA'>('TURNO');

/**
 * COMPUTED: Nombre del jugador que inicia (para feedback visual)
 */
const nombreGanadorTurno = computed(() => {
    if (gameStore.turnoActual === 1) return gameStore.jugador1.nickname;
    if (gameStore.turnoActual === 2) return gameStore.jugador2.nickname;
    return '...';
});

// --- MANEJADORES DE FLUJO ---

/**
 * 1. Fin del Duelo de Dados de Turno
 * Recibe el ID (1 o 2) y salta al dado de categoría.
 */
const manejarFinTurno = (ganadorID: number) => {
    gameStore.turnoActual = ganadorID;

    // Transición suave para que vean quién ganó antes de cambiar de pantalla
    setTimeout(() => {
        pasoActual.value = 'CATEGORIA';
    }, 1500);
};

/**
 * 2. Fin del Dado de Categoría
 * El dado ya guardó la categoría en el store y pidió el mazo al backend.
 * Solo saltamos al paso de las cartas.
 */
const manejarFinCategoria = (categoria: string) => {
    // La categoría ya viene guardada desde el componente hijo en el store
    pasoActual.value = 'CARTA';
};

/**
 * 3. Fin de Selección de Carta
 * Este evento se dispara cuando RevelacionCarta termina su animación.
 */
const manejarCartaRevelada = () => {
    // La lógica de navegación a 'JUEGO' ya ocurre dentro de SeleccionCartas/RevelacionCarta
    // pero podemos usar este emit para logs o analíticas locales.
    console.log("🚀 Configuración finalizada. Iniciando partida...");
};

/**
 * WATCH PARA AUDIOS Y EVENTOS DE HARDWARE
 * Cada vez que cambiamos de paso, le decimos al ESP32 qué audio reproducir
 * y en qué estado de espera debe estar.
 */
watch(pasoActual, (nuevoPaso) => {
    if (nuevoPaso === 'TURNO') {
        sendEvent('UI_CAMBIO_FASE', { fase: 'TURNO_INICIO' });
    } else if (nuevoPaso === 'CATEGORIA') {
        sendEvent('UI_CAMBIO_FASE', { fase: 'CATEGORIA_INICIO' });
    } else if (nuevoPaso === 'CARTA') {
        sendEvent('UI_CAMBIO_FASE', { fase: 'SELECCION_CARTA_INICIO' });
    }
}, { immediate: true }); // immediate para que el primer paso (TURNO) dispare su audio al cargar
</script>

<template>
    <div class="config-container">
        <header class="setup-header">
            <div class="progress-bar">
                <div class="step" :class="{ active: pasoActual === 'TURNO', done: pasoActual !== 'TURNO' }">
                    <span>{{ pasoActual === 'TURNO' ? '1' : '✓' }}</span>
                </div>

                <div class="connector" :class="{ filled: pasoActual !== 'TURNO' }"></div>

                <div class="step" :class="{ active: pasoActual === 'CATEGORIA', done: pasoActual === 'CARTA' }">
                    <span>{{ pasoActual === 'CARTA' ? '✓' : '2' }}</span>
                </div>

                <div class="connector" :class="{ filled: pasoActual === 'CARTA' }"></div>

                <div class="step" :class="{ active: pasoActual === 'CARTA' }">
                    <span>3</span>
                </div>
            </div>

            <h1 class="main-title">Preparación de Ronda</h1>
            <p class="hw-helper">Controla el juego con tu mando 🎮</p>
        </header>

        <main class="setup-content">
            <transition name="slide-fade" mode="out-in">

                <section v-if="pasoActual === 'TURNO'" key="turno" class="view-section">
                    <h2 class="section-title">¿Quién empieza?</h2>
                    <p class="section-desc">Presionen el botón <strong>TURNO</strong> en sus mandos.</p>
                    <DadoTurno @finalizado="manejarFinTurno" />
                </section>

                <section v-else-if="pasoActual === 'CATEGORIA'" key="categoria" class="view-section">
                    <h2 class="section-title">Modo de Juego</h2>
                    <p class="section-desc">Presiona el botón <strong>CATEGORÍA</strong> para girar.</p>
                    <DadoCategoria @seleccionado="manejarFinCategoria" />

                    <div v-if="gameStore.mazo.length > 0" class="mazo-ready-hint animate-bounce">
                        ✅ Mazo cargado. Pulsa <strong>SELECT</strong> para continuar.
                    </div>
                </section>

                <section v-else-if="pasoActual === 'CARTA'" key="carta" class="view-section">
                    <SeleccionCartas @cartaRevelada="manejarCartaRevelada" />
                </section>

            </transition>
        </main>

        <footer class="setup-footer" v-if="pasoActual !== 'TURNO'">
            <div class="badges-row">
                <div class="badge player">
                    👤 Inicia: <strong>{{ nombreGanadorTurno }}</strong>
                </div>
                <div class="badge category" v-if="gameStore.categoriaActual">
                    🎲 Modo: <strong>{{ gameStore.categoriaActual }}</strong>
                </div>
                <div class="badge difficulty" v-if="gameStore.dificultadActual">
                    🔥 Nivel: <strong>{{ gameStore.dificultadActual }}</strong>
                </div>
            </div>
        </footer>
    </div>
</template>

<style scoped>
.config-container {
    height: 660px;
    min-width: 1500px;
    display: flex;
    flex-direction: column;
    align-items: center;
    background-color: #fdf2f2;
    padding: 20px;
}

.hw-helper {
    color: #a64444;
    font-weight: bold;
    font-style: italic;
    opacity: 0.8;
    text-align: center;
}

.section-title {
    text-align: center;
}

.section-desc {
    color: #a64444;
    font-weight: bold;
    font-style: italic;
    opacity: 0.8;
    text-align: center;
    margin-bottom: 25px;
}

.progress-bar {
    display: flex;
    align-items: center;
    /* Quitamos gap para que el conector pegue */
    margin: 20px 0;
    margin-left: 10px;
}

.connector {
    width: 60px;
    height: 6px;
    background: #d99a9a;
    transition: background 0.5s;
}

.connector.filled {
    background: #a64444;
}

.step {
    width: 40px;
    height: 40px;
    border-radius: 50%;
    background: white;
    border: 4px solid #d99a9a;
    display: flex;
    align-items: center;
    justify-content: center;
    font-weight: 900;
    font-size: 1.2rem;
    color: #d99a9a;
    z-index: 2;
    margin-left: 5px;
}

.step.active {
    border-color: #3e8ebf;
    color: #3e8ebf;
    transform: scale(1.1);
    box-shadow: 0 0 15px rgba(62, 142, 191, 0.4);
}

.step.done {
    background: #a64444;
    border-color: #a64444;
    color: white;
}

.setup-content {
    min-width: 80%;
    max-width: 1400px;
    height: 20% !important;
    background: white;
    border-radius: 40px;
    padding: 20px;
    box-shadow: 0 30px 60px rgba(115, 49, 49, 0.15);
    min-height: 600px;
}

.setup-footer {
    display: flex;
    justify-content: center;
}

.badge {
    margin-right: 10px;
    padding: 5px 25px;
    border-radius: 30px;
    font-weight: 900;
    text-transform: uppercase;
    letter-spacing: 1px;
}

.badge.cat {
    background: #3e8ebf;
    color: white;
    width: 20%;
    text-align: center;
}

.badge.sil {
    background: #27ae60;
    color: white;
}

/* Transiciones suaves */
.slide-fade-enter-active,
.slide-fade-leave-active {
    transition: all 0.5s ease;
}

.slide-fade-enter-from {
    opacity: 0;
    transform: translateY(30px);
}

.slide-fade-leave-to {
    opacity: 0;
    transform: translateY(-30px);
}
</style>