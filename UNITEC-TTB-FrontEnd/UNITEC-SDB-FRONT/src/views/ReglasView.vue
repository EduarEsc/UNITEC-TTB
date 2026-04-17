<script setup lang="ts">
import { onMounted } from 'vue';
import { useGameStore } from '@/stores/game';

const gameStore = useGameStore();

// Aseguramos que el total de pestañas sea correcto al montar el componente
onMounted(() => {
    gameStore.TOTAL_TABS_REGLAS = 4;
    gameStore.tabReglasActual = 0;
});

// Esta función se usa para el click manual (opcional), 
// pero el hardware ya llama a confirmarSeleccionMazo() en el store.
const irAlRegistroManualmente = () => {
    gameStore.confirmarSeleccionMazo();
};
</script>

<template>
    <div class="reglas-page-wrapper">
        <h1 class="reglamento-title">Reglamento Oficial</h1>

        <div class="main-card">
            <div class="tabs-header">
                <button v-for="idx in [0, 1, 2, 3]" :key="idx" @click="gameStore.tabReglasActual = idx"
                    :class="{ active: gameStore.tabReglasActual === idx }">
                    {{ idx + 1 }}. {{ ['Inicio', 'Dinámica', 'Modos', 'Puntuación'][idx] }}
                </button>
            </div>

            <div class="tabs-body-scroll">
                <div class="padding-container">

                    <div v-if="gameStore.tabReglasActual === 0" class="tab-pane animate-fade">
                        <div class="info-grid">
                            <div class="info-card">
                                <div class="icon-circle">👥</div>
                                <h3>Registro</h3>
                                <p>Se requieren <strong>2 jugadores</strong> con nicknames únicos.</p>
                            </div>
                            <div class="info-card">
                                <div class="icon-circle">🎲</div>
                                <h3>Turnos</h3>
                                <p>Duelo de dados digital: el valor más alto inicia la partida.</p>
                            </div>
                        </div>
                    </div>

                    <div v-if="gameStore.tabReglasActual === 1" class="tab-pane animate-fade">
                        <p class="intro-text">Ubicación de la sílaba según el dado:</p>
                        <div class="category-grid">
                            <div class="cat-item tic"><strong>TIC</strong><span>Al Inicio</span></div>
                            <div class="cat-item tac"><strong>TAC</strong><span>Al Centro</span></div>
                            <div class="cat-item bum"><strong>BUM</strong><span>Al Final</span></div>
                        </div>
                        <div class="rule-detail-box">
                            <p>• <strong>TIC:</strong> La palabra debe empezar con la sílaba.</p>
                            <p>• <strong>TAC:</strong> La sílaba debe estar dentro (no extremos).</p>
                            <p>• <strong>BUM:</strong> La palabra debe terminar con la sílaba.</p>
                        </div>
                    </div>

                    <div v-if="gameStore.tabReglasActual === 2" class="tab-pane animate-fade">
                        <section class="modo-section">
                            <h3 class="duelo-title">Modo Duelo</h3>
                            <div class="niveles-grid">
                                <div class="nivel-card n-facil"><strong>Fácil:</strong> Sílabas comunes.</div>
                                <div class="nivel-card n-medio"><strong>Medio:</strong> Sílabas de 3 letras.</div>
                                <div class="nivel-card n-dificil"><strong>Difícil:</strong> Combinaciones complejas.
                                </div>
                            </div>
                        </section>
                    </div>

                    <div v-if="gameStore.tabReglasActual === 3" class="tab-pane animate-fade">
                        <div class="penalty-grid">
                            <div class="penalty-card">
                                <h4>✅ Validación</h4>
                                <p>Mantén presionado el botón de micrófono para decir tu palabra.</p>
                            </div>
                            <div class="penalty-card warning">
                                <h4>⚠️ Penalización</h4>
                                <p>Si la bomba explota antes de responder, pierdes una vida.</p>
                            </div>
                        </div>
                    </div>

                </div>
            </div>
        </div>

        <button class="btn-primary" @click="irAlRegistroManualmente">
            <span>Ir al Registro (Botón SELECT) →</span>
        </button>

        <p class="hw-hint">
            Usa la 🕹️ <strong>PALANCA</strong> (Izquierda/Derecha) para navegar y 🔘 <strong>SELECT</strong> para
            continuar.
        </p>
    </div>
</template>

<style scoped>
.hw-hint {
    margin-top: 10px;
    font-size: 0.95rem;
    color: #bdc3c7;
    opacity: 0.9;
    font-style: italic;
    text-align: center;
}

.btn-primary:active {
    transform: scale(0.96);
}

/* =========================
   CONTENEDOR PRINCIPAL
========================= */
.reglas-page-wrapper {
    width: 95%;
    max-width: 1100px;
    height: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    margin: 0 auto;
    color: #ecf0f1;
}

.reglamento-title {
    flex-shrink: 0;
    margin-top: 8px;
    margin-bottom: 22px;
    color: #f5f6fa;
    font-size: 2.7rem;
    font-weight: 900;
    letter-spacing: 1px;
    text-shadow: 0 4px 16px rgba(0, 0, 0, 0.35);
}

/* =========================
   CARD PRINCIPAL
========================= */
.main-card {
    flex: 1;
    width: 100%;
    background: rgba(255, 255, 255, 0.05);
    border-radius: 28px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    box-shadow:
        0 20px 40px rgba(0, 0, 0, 0.28),
        0 0 20px rgba(255, 255, 255, 0.04);
    border: 1px solid rgba(255, 255, 255, 0.08);
    backdrop-filter: blur(10px);
}

.tabs-header {
    flex-shrink: 0;
    display: flex;
    background: rgba(255, 255, 255, 0.04);
    border-bottom: 1px solid rgba(255, 255, 255, 0.06);
}

.tabs-body-scroll {
    flex: 1;
    overflow-y: auto;
    padding: 0;
}

.padding-container {
    padding: 30px;
}

/* =========================
   TABS
========================= */
.tabs-header button {
    flex: 1;
    padding: 18px 12px;
    border: none;
    background: transparent;
    cursor: pointer;
    font-weight: 800;
    color: #bdc3c7;
    transition: all 0.25s ease;
    letter-spacing: 0.3px;
}

.tabs-header button:hover {
    background: rgba(255, 255, 255, 0.05);
    color: #ffffff;
}

.tabs-header button.active {
    background: rgba(241, 196, 15, 0.12);
    color: #f1c40f;
    box-shadow: inset 0 -3px 0 #f1c40f;
}

/* =========================
   BLOQUES INTERNOS
========================= */
.info-grid,
.category-grid,
.penalty-grid,
.niveles-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
    gap: 20px;
    margin-bottom: 20px;
}

.info-card,
.penalty-card,
.nivel-card,
.rule-detail-box {
    background: rgba(255, 255, 255, 0.06);
    padding: 22px;
    border-radius: 20px;
    text-align: center;
    border: 1px solid rgba(255, 255, 255, 0.07);
    box-shadow: 0 10px 24px rgba(0, 0, 0, 0.18);
    transition: transform 0.25s ease, box-shadow 0.25s ease, border-color 0.25s ease;
}

.info-card:hover,
.penalty-card:hover,
.nivel-card:hover,
.rule-detail-box:hover {
    transform: translateY(-4px);
    box-shadow: 0 16px 28px rgba(0, 0, 0, 0.24);
    border-color: rgba(241, 196, 15, 0.22);
}

.info-card h3,
.penalty-card h4,
.modo-section h3 {
    margin-top: 6px;
    margin-bottom: 10px;
    color: #ffffff;
    font-weight: 900;
}

.info-card p,
.penalty-card p,
.nivel-card,
.rule-detail-box p,
.intro-text,
.cat-explanation {
    color: #d5d8dc;
    line-height: 1.5;
}

.icon-circle {
    width: 68px;
    height: 68px;
    margin: 0 auto 12px;
    border-radius: 50%;
    display: flex;
    justify-content: center;
    align-items: center;
    font-size: 2rem;
    background: rgba(241, 196, 15, 0.14);
    box-shadow: 0 0 18px rgba(241, 196, 15, 0.14);
}

.category-grid {
    margin-top: 20px;
}

.cat-item {
    padding: 18px;
    border-radius: 18px;
    text-align: center;
    color: white;
    display: flex;
    flex-direction: column;
    gap: 6px;
    font-weight: 800;
    box-shadow: 0 10px 24px rgba(0, 0, 0, 0.2);
    transition: transform 0.25s ease;
}

.cat-item:hover {
    transform: translateY(-4px) scale(1.02);
}

.cat-item strong {
    font-size: 1.4rem;
    letter-spacing: 1px;
}

.tic {
    background: linear-gradient(135deg, #2ecc71, #27ae60);
}

.tac {
    background: linear-gradient(135deg, #f1c40f, #f39c12);
    color: #111;
}

.bum {
    background: linear-gradient(135deg, #e74c3c, #c0392b);
}

.rule-detail-box {
    margin-top: 18px;
    text-align: left;
}

.intro-text {
    text-align: center;
    font-size: 1.1rem;
    font-weight: 700;
    margin-bottom: 10px;
}

.modo-section {
    display: flex;
    flex-direction: column;
    gap: 18px;
}

.duelo-title {
    text-align: center;
    color: #f1c40f;
    font-size: 1.4rem;
    font-weight: 900;
    letter-spacing: 1px;
}

.nivel-card {
    text-align: left;
    border-left: 6px solid transparent;
    font-weight: 700;
}

.n-facil {
    border-left-color: #2ecc71;
}

.n-medio {
    border-left-color: #f1c40f;
}

.n-dificil {
    border-left-color: #e74c3c;
}

.penalty-card.warning {
    border-color: rgba(231, 76, 60, 0.35);
    box-shadow: 0 0 18px rgba(231, 76, 60, 0.12);
}

/* =========================
   BOTÓN PRINCIPAL
========================= */
.btn-primary {
    flex-shrink: 0;
    margin: 24px 0 14px;
    padding: 16px 58px;
    background: linear-gradient(135deg, #f1c40f, #f39c12);
    color: #111;
    border: none;
    border-radius: 999px;
    font-size: 1.1rem;
    font-weight: 900;
    cursor: pointer;
    letter-spacing: 0.6px;
    box-shadow:
        0 10px 24px rgba(241, 196, 15, 0.25),
        0 0 16px rgba(241, 196, 15, 0.18);
    transition: transform 0.25s ease, box-shadow 0.25s ease;
}

.btn-primary:hover {
    transform: translateY(-2px) scale(1.02);
    box-shadow:
        0 14px 28px rgba(241, 196, 15, 0.32),
        0 0 22px rgba(241, 196, 15, 0.22);
}

/* =========================
   SCROLLBAR
========================= */
.tabs-body-scroll::-webkit-scrollbar {
    width: 8px;
}

.tabs-body-scroll::-webkit-scrollbar-track {
    background: transparent;
}

.tabs-body-scroll::-webkit-scrollbar-thumb {
    background: rgba(241, 196, 15, 0.45);
    border-radius: 10px;
}

.tabs-body-scroll::-webkit-scrollbar-thumb:hover {
    background: rgba(241, 196, 15, 0.75);
}

/* =========================
   ANIMACIONES
========================= */
.animate-fade {
    animation: fadeIn 0.35s ease-out;
}

@keyframes fadeIn {
    from {
        opacity: 0;
        transform: translateY(10px);
    }

    to {
        opacity: 1;
        transform: translateY(0);
    }
}

/* =========================
   RESPONSIVE
========================= */
@media (max-width: 768px) {
    .reglamento-title {
        font-size: 2rem;
        text-align: center;
    }

    .padding-container {
        padding: 20px;
    }

    .tabs-header button {
        font-size: 0.82rem;
        padding: 14px 8px;
    }

    .btn-primary {
        width: 100%;
        padding: 15px 20px;
    }
}
</style>