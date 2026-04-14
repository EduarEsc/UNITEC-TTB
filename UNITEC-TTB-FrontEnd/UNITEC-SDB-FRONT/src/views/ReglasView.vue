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
/* Mantén tus estilos y añade estos para feedback de hardware */
.hw-hint {
    margin-top: 10px;
    font-size: 0.9rem;
    color: #733131;
    opacity: 0.7;
    font-style: italic;
}

.btn-primary:active {
    transform: scale(0.95);
    background: #733131;
}

/* CONTENEDOR PRINCIPAL: Controla que nada se mueva */
.reglas-page-wrapper {
    width: 95%;
    max-width: 1000px;
    height: 90vh;
    /* Ocupa casi toda la pantalla */
    display: flex;
    flex-direction: column;
    align-items: center;
    margin: 0 auto;
}

.reglamento-title {
    flex-shrink: 0;
    /* No permite que el título se encoja o mueva */
    margin-top: 20px;
    color: #3e8ebf;
    font-family: 'Georgia', serif;
    font-size: 2.5rem;
    margin-bottom: 20px;
}

.main-card {
    flex: 1;
    /* Toma todo el espacio disponible */
    width: 100%;
    background: #fdf2f2;
    border-radius: 25px;
    display: flex;
    flex-direction: column;
    overflow: hidden;
    /* Corta el contenido que se salga */
    box-shadow: 0 15px 35px rgba(0, 0, 0, 0.1);
    border: 1px solid rgba(115, 49, 49, 0.1);
}

.tabs-header {
    flex-shrink: 0;
    display: flex;
    background: #d99a9a;
}

/* SCROLL INTERNO: Esta es la clave */
.tabs-body-scroll {
    flex: 1;
    overflow-y: auto;
    /* Solo hay scroll aquí dentro */
    padding: 0;
}

.padding-container {
    padding: 30px;
}

/* ESTILOS DE TABS REDISEÑADOS */
.info-grid,
.category-grid,
.penalty-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    gap: 20px;
    margin-bottom: 20px;
}

.info-card,
.penalty-card {
    background: white;
    padding: 20px;
    border-radius: 15px;
    text-align: center;
    box-shadow: 0 5px 15px rgba(0, 0, 0, 0.05);
}

.icon-circle {
    font-size: 2rem;
    margin-bottom: 10px;
}

.cat-item {
    padding: 15px;
    border-radius: 12px;
    text-align: center;
    color: white;
    display: flex;
    flex-direction: column;
}

.tic {
    background: #a64444;
}

.tac {
    background: #3e8ebf;
}

.bum {
    background: #733131;
}

.rule-detail-box {
    background: white;
    padding: 20px;
    border-radius: 15px;
    margin-top: 20px;
}

/* ESTILOS EXISTENTES MEJORADOS */
.tabs-header button {
    flex: 1;
    padding: 18px 10px;
    border: none;
    background: none;
    cursor: pointer;
    font-weight: bold;
    color: #733131;
    transition: all 0.3s;
}

.tabs-header button.active {
    background: #fdf2f2;
    color: #3e8ebf;
}

.nivel-card {
    background: white;
    padding: 15px;
    border-radius: 10px;
    border-left: 5px solid #a64444;
}

.record-table {
    width: 100%;
    border-collapse: collapse;
    background: white;
    border-radius: 10px;
    overflow: hidden;
}

.record-table th {
    background: #733131;
    color: white;
    padding: 12px;
}

.record-table td {
    padding: 12px;
    border-bottom: 1px solid #eee;
    text-align: center;
}

.btn-primary {
    flex-shrink: 0;
    /* El botón no se mueve */
    margin: 20px 0;
    padding: 15px 60px;
    background: #a64444;
    color: white;
    border: none;
    border-radius: 40px;
    font-size: 1.2rem;
    font-weight: bold;
    cursor: pointer;
    box-shadow: 0 8px 20px rgba(166, 68, 68, 0.3);
}

/* Personalización del Scrollbar */
.tabs-body-scroll::-webkit-scrollbar {
    width: 8px;
}

.tabs-body-scroll::-webkit-scrollbar-track {
    background: transparent;
}

.tabs-body-scroll::-webkit-scrollbar-thumb {
    background: #d99a9a;
    border-radius: 10px;
}

.animate-fade {
    animation: fadeIn 0.4s ease-out;
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
</style>