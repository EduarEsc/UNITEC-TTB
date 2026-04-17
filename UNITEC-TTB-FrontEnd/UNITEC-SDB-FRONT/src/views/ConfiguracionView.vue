<script setup lang="ts">
import { computed, onMounted } from 'vue';
import { useGameStore } from '@/stores/game';
import { sendEvent } from '@/services/websocket';

import DadoTurno from '@/components/DadoTurno.vue';
import DadoCategoria from '@/components/DadoCategoria.vue';
import SeleccionCartas from '@/components/SeleccionCartas.vue';

const gameStore = useGameStore();

const pasoActual = computed(() => {
    if (gameStore.configStep === 'TURNO') return 'TURNO';
    if (gameStore.configStep === 'CATEGORIA') return 'CATEGORIA';
    if (gameStore.configStep === 'CARTA') return 'CARTA';
    if (gameStore.configStep === 'REVELACION') return 'CARTA';
    return 'TURNO';
});

const nombreGanadorTurno = computed(() => {
    if (gameStore.turnoActual === 1) return gameStore.jugador1.nickname;
    if (gameStore.turnoActual === 2) return gameStore.jugador2.nickname;
    return '...';
});

const cambiarPaso = (nuevoPaso: 'TURNO' | 'CATEGORIA' | 'CARTA') => {
    gameStore.setConfigStep(nuevoPaso);

    if (nuevoPaso === 'TURNO') {
        sendEvent('UI_CAMBIO_FASE', { fase: 'TURNO_INICIO' });
    } else if (nuevoPaso === 'CATEGORIA') {
        sendEvent('UI_CAMBIO_FASE', { fase: 'CATEGORIA_INICIO' });
    } else if (nuevoPaso === 'CARTA') {
        gameStore.prepararVistaCartas();
        gameStore.setConfigStep('CARTA');
        gameStore.bloquearSelectPor(2000);
        sendEvent('UI_CAMBIO_FASE', { fase: 'SELECCION_CARTA_INICIO' });
    }
};

const manejarFinTurno = (ganadorID: number) => {
    gameStore.turnoActual = ganadorID;

    setTimeout(() => {
        cambiarPaso('CATEGORIA');
    }, 1500);
};

const manejarFinCategoria = () => {
    // SIN delay para evitar perder SELECT en transición
    cambiarPaso('CARTA');
};

const manejarCartaRevelada = () => {
    console.log("🚀 Configuración finalizada. Carta revelada.");
};

onMounted(() => {
    cambiarPaso('TURNO');
});
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
    width: 100%;
    min-height: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 12px 12px 24px;
    color: #ecf0f1;
}

.setup-header {
    width: 100%;
    max-width: 1280px;
    display: flex;
    flex-direction: column;
    align-items: center;
    margin-bottom: 22px;
}

.main-title {
    margin: 8px 0 10px;
    font-size: 2.6rem;
    font-weight: 900;
    color: #f5f6fa;
    letter-spacing: 1px;
    text-align: center;
    text-shadow: 0 4px 16px rgba(0, 0, 0, 0.35);
}

.hw-helper {
    color: #bdc3c7;
    font-weight: 700;
    font-style: italic;
    opacity: 0.95;
    text-align: center;
    margin: 0;
}

/* =========================
   BARRA DE PROGRESO
========================= */
.progress-bar {
    display: flex;
    align-items: center;
    margin: 12px 0 18px;
}

.connector {
    width: 72px;
    height: 8px;
    border-radius: 999px;
    background: rgba(255, 255, 255, 0.08);
    transition: all 0.35s ease;
    box-shadow: inset 0 0 0 1px rgba(255, 255, 255, 0.04);
}

.connector.filled {
    background: linear-gradient(90deg, #f1c40f, #f39c12);
    box-shadow: 0 0 14px rgba(241, 196, 15, 0.22);
}

.step {
    width: 48px;
    height: 48px;
    border-radius: 50%;
    background: rgba(255, 255, 255, 0.06);
    border: 3px solid rgba(255, 255, 255, 0.12);
    display: flex;
    align-items: center;
    justify-content: center;
    font-weight: 900;
    font-size: 1.1rem;
    color: #bdc3c7;
    z-index: 2;
    transition: all 0.3s ease;
    box-shadow: 0 10px 18px rgba(0, 0, 0, 0.18);
}

.step.active {
    border-color: #f1c40f;
    color: #111;
    background: linear-gradient(135deg, #f1c40f, #f39c12);
    transform: scale(1.08);
    box-shadow:
        0 0 20px rgba(241, 196, 15, 0.28),
        0 10px 18px rgba(0, 0, 0, 0.22);
}

.step.done {
    background: linear-gradient(135deg, #2ecc71, #27ae60);
    border-color: #2ecc71;
    color: white;
    box-shadow:
        0 0 18px rgba(46, 204, 113, 0.22),
        0 10px 18px rgba(0, 0, 0, 0.2);
}

/* =========================
   CONTENIDO PRINCIPAL
========================= */
.setup-content {
    width: 100%;
    max-width: 1320px;
    min-height: 620px;
    background: rgba(255, 255, 255, 0.05);
    border-radius: 34px;
    padding: 28px;
    box-shadow:
        0 24px 42px rgba(0, 0, 0, 0.28),
        0 0 20px rgba(255, 255, 255, 0.04);
    border: 1px solid rgba(255, 255, 255, 0.08);
    backdrop-filter: blur(10px);
    overflow: hidden;
}

.view-section {
    width: 100%;
    height: 100%;
}

.section-title {
    text-align: center;
    margin: 8px 0 8px;
    font-size: 2rem;
    font-weight: 900;
    color: #ffffff;
    letter-spacing: 0.5px;
}

.section-desc {
    color: #d5d8dc;
    font-weight: 700;
    font-style: italic;
    text-align: center;
    margin-bottom: 28px;
    font-size: 1rem;
}

.section-desc strong,
.hw-helper strong {
    color: #f1c40f;
}

/* =========================
   HINT DEL MAZO LISTO
========================= */
.mazo-ready-hint {
    margin-top: 22px;
    text-align: center;
    background: rgba(46, 204, 113, 0.12);
    border: 2px solid rgba(46, 204, 113, 0.35);
    color: #d5f5e3;
    padding: 12px 18px;
    border-radius: 18px;
    font-weight: 800;
    letter-spacing: 0.3px;
    box-shadow: 0 0 16px rgba(46, 204, 113, 0.12);
}

/* =========================
   FOOTER / BADGES
========================= */
.setup-footer {
    display: flex;
    justify-content: center;
    width: 100%;
    margin-top: 18px;
}

.badges-row {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: 12px;
}

.badge {
    padding: 10px 22px;
    border-radius: 999px;
    font-weight: 900;
    text-transform: uppercase;
    letter-spacing: 1px;
    font-size: 0.88rem;
    color: white;
    box-shadow: 0 8px 18px rgba(0, 0, 0, 0.18);
    border: 2px solid transparent;
}

.badge.player {
    background: rgba(241, 196, 15, 0.12);
    color: #fef9c3;
    border-color: rgba(241, 196, 15, 0.45);
}

.badge.category {
    background: rgba(52, 152, 219, 0.14);
    color: #d6ecff;
    border-color: rgba(52, 152, 219, 0.38);
}

.badge.difficulty {
    background: rgba(231, 76, 60, 0.14);
    color: #ffd8d3;
    border-color: rgba(231, 76, 60, 0.35);
}

/* =========================
   TRANSICIONES
========================= */
.slide-fade-enter-active,
.slide-fade-leave-active {
    transition: all 0.4s ease;
}

.slide-fade-enter-from {
    opacity: 0;
    transform: translateY(24px);
}

.slide-fade-leave-to {
    opacity: 0;
    transform: translateY(-24px);
}

/* =========================
   RESPONSIVE
========================= */
@media (max-width: 1024px) {
    .setup-content {
        min-height: 560px;
        padding: 22px;
    }

    .main-title {
        font-size: 2.2rem;
    }

    .section-title {
        font-size: 1.7rem;
    }
}

@media (max-width: 768px) {
    .config-container {
        padding: 8px 8px 20px;
    }

    .setup-content {
        min-height: 520px;
        padding: 18px;
        border-radius: 24px;
    }

    .progress-bar {
        transform: scale(0.92);
    }

    .connector {
        width: 44px;
    }

    .main-title {
        font-size: 1.9rem;
    }

    .section-title {
        font-size: 1.45rem;
    }

    .badges-row {
        gap: 8px;
    }

    .badge {
        font-size: 0.78rem;
        padding: 8px 14px;
    }
}
</style>