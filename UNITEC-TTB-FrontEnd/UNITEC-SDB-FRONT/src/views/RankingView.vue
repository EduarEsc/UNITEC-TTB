<script setup lang="ts">
import { ref, onMounted } from 'vue';
import { useGameStore } from '@/stores/game';

const gameStore = useGameStore();
const ranking = ref<{ nickname: string, score: number }[]>([]);
const cargando = ref(true);

// Función para obtener los datos del backend
const obtenerRanking = async () => {
    try {
        cargando.value = true;
        // Cambia la URL por la de tu servidor si es distinta
        const response = await fetch('http://localhost:8000/ranking');
        const data = await response.json();
        ranking.value = data;
    } catch (error) {
        console.error("Error al obtener el ranking:", error);
    } finally {
        cargando.value = false;
    }
};

onMounted(() => {
    obtenerRanking();
});

const volverAlInicio = () => {
    gameStore.vistaActual = 'REGLAS';
};
</script>

<template>
    <div class="ranking-container">
        <h1 class="ranking-title">🏆 Salón de la Fama 🏆</h1>

        <div class="ranking-card">
            <div v-if="cargando" class="loader">Cargando puntuaciones...</div>

            <table class="ranking-table">
                <thead>
                    <tr>
                        <th>Puesto</th>
                        <th>Jugador</th>
                        <th>Puntuación</th>
                    </tr>
                </thead>
                <tbody>
                    <tr v-for="(user, index) in ranking" :key="index">
                        <td class="pos-cell">
                            <span v-if="index === 0">🥇</span>
                            <span v-else-if="index === 1">🥈</span>
                            <span v-else-if="index === 2">🥉</span>
                            <span v-else>{{ index + 1 }}</span>
                        </td>
                        <td class="name-cell">{{ user.nickname }}</td>
                        <td class="score-cell">
                            {{ user.score }} <span class="pts-label">pts</span>
                        </td>
                    </tr>
                </tbody>
            </table>

            <div v-if="!cargando && ranking.length === 0" class="no-data">
                Aún no hay puntuaciones registradas. ¡Sé el primero!
            </div>
        </div>

        <button class="btn-volver" @click="volverAlInicio">
            ← Volver al Inicio
        </button>
    </div>
</template>

<style scoped>
.ranking-container {
    width: 100%;
    max-width: 980px;
    min-height: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: flex-start;
    padding: 8px 0 18px;
    color: #ecf0f1;
}

.ranking-title {
    color: #f5f6fa;
    font-size: 2.8rem;
    font-weight: 900;
    margin: 0 0 24px;
    letter-spacing: 1px;
    text-align: center;
    text-shadow: 0 4px 16px rgba(0, 0, 0, 0.35);
}

.ranking-card {
    width: 100%;
    max-width: 920px;
    padding: 26px;
    border-radius: 28px;
    background: rgba(255, 255, 255, 0.05);
    border: 1px solid rgba(255, 255, 255, 0.08);
    box-shadow:
        0 22px 40px rgba(0, 0, 0, 0.28),
        0 0 18px rgba(255, 255, 255, 0.03);
    backdrop-filter: blur(10px);
    margin-bottom: 28px;
    overflow: hidden;
}

.ranking-table {
    width: 100%;
    border-collapse: separate;
    border-spacing: 0 12px;
    font-size: 1.05rem;
}

.ranking-table thead th {
    background: linear-gradient(135deg, #f1c40f, #f39c12);
    color: #111;
    padding: 14px 16px;
    text-align: center;
    font-size: 0.92rem;
    font-weight: 900;
    letter-spacing: 1px;
    text-transform: uppercase;
    border: none;
}

.ranking-table thead th:first-child {
    border-top-left-radius: 14px;
    border-bottom-left-radius: 14px;
}

.ranking-table thead th:last-child {
    border-top-right-radius: 14px;
    border-bottom-right-radius: 14px;
}

.ranking-table tbody tr {
    background: rgba(255, 255, 255, 0.05);
    transition: transform 0.22s ease, box-shadow 0.22s ease, background 0.22s ease;
    box-shadow: 0 10px 18px rgba(0, 0, 0, 0.12);
}

.ranking-table tbody tr:hover {
    transform: translateY(-2px);
    background: rgba(255, 255, 255, 0.075);
    box-shadow: 0 14px 22px rgba(0, 0, 0, 0.18);
}

.ranking-table td {
    padding: 16px 18px;
    border: none;
    color: #ecf0f1;
}

.ranking-table td:first-child {
    border-top-left-radius: 16px;
    border-bottom-left-radius: 16px;
}

.ranking-table td:last-child {
    border-top-right-radius: 16px;
    border-bottom-right-radius: 16px;
}

/* =========================
   TOP 3
========================= */
.ranking-table tbody tr:nth-child(1) {
    background: rgba(241, 196, 15, 0.14);
    box-shadow:
        0 0 18px rgba(241, 196, 15, 0.14),
        0 12px 20px rgba(0, 0, 0, 0.16);
}

.ranking-table tbody tr:nth-child(2) {
    background: rgba(189, 195, 199, 0.12);
}

.ranking-table tbody tr:nth-child(3) {
    background: rgba(211, 84, 0, 0.12);
}

.pos-cell {
    width: 90px;
    text-align: center;
    font-size: 1.4rem;
    font-weight: 900;
    color: #ffffff;
}

.name-cell {
    color: #ffffff;
    font-weight: 800;
    text-transform: capitalize;
    letter-spacing: 0.3px;
}

.score-cell {
    text-align: right;
    color: #f1c40f;
    font-weight: 900;
    font-size: 1.15rem;
}

.pts-label {
    font-size: 0.78rem;
    font-weight: 800;
    letter-spacing: 0.8px;
    text-transform: uppercase;
    color: #bdc3c7;
    margin-left: 4px;
}

/* =========================
   ESTADOS
========================= */
.loader,
.no-data {
    text-align: center;
    padding: 42px 18px;
    color: #bdc3c7;
    font-style: italic;
    font-weight: 700;
    letter-spacing: 0.2px;
}

.loader {
    animation: pulse-loading 1s infinite;
}

@keyframes pulse-loading {
    0% {
        opacity: 1;
    }

    50% {
        opacity: 0.45;
    }

    100% {
        opacity: 1;
    }
}

/* =========================
   BOTÓN
========================= */
.btn-volver {
    background: linear-gradient(135deg, #3498db, #2980b9);
    color: white;
    border: none;
    padding: 14px 34px;
    border-radius: 999px;
    cursor: pointer;
    transition: all 0.25s ease;
    font-weight: 900;
    letter-spacing: 0.5px;
    font-size: 1rem;
    box-shadow:
        0 10px 20px rgba(52, 152, 219, 0.24),
        0 0 14px rgba(52, 152, 219, 0.14);
}

.btn-volver:hover {
    transform: translateY(-2px) scale(1.02);
    box-shadow:
        0 14px 24px rgba(52, 152, 219, 0.28),
        0 0 18px rgba(52, 152, 219, 0.18);
}

/* =========================
   RESPONSIVE
========================= */
@media (max-width: 768px) {
    .ranking-title {
        font-size: 2rem;
    }

    .ranking-card {
        padding: 18px;
        border-radius: 20px;
    }

    .ranking-table {
        font-size: 0.92rem;
        border-spacing: 0 8px;
    }

    .ranking-table thead th {
        font-size: 0.72rem;
        padding: 12px 10px;
    }

    .ranking-table td {
        padding: 12px 10px;
    }

    .pos-cell {
        font-size: 1.1rem;
        width: 66px;
    }

    .score-cell {
        font-size: 1rem;
    }

    .btn-volver {
        width: 100%;
        max-width: 320px;
        text-align: center;
    }
}
</style>