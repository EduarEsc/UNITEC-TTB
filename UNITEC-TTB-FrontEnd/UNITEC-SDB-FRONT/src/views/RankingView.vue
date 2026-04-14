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
    height: 100%;
    width: 100%;
    max-width: 800px;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: flex-start;
    padding-bottom: 10px !important;
}

.ranking-title {
    color: #733131;
    font-family: 'Georgia', serif;
    font-size: 3rem;
    margin-bottom: 30px;
    text-shadow: 2px 2px white;
}

.ranking-card {
    background: #fdf2f2;
    border-radius: 20px;
    width: 90%;
    max-width: 900px;
    /* Evita que se estire demasiado en pantallas grandes */
    padding: 30px;
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.15);
    border: 3px solid #d99a9a;
    margin-bottom: 40px;
}

.ranking-table {
    width: 100%;
    height: 50%;
    font-size: 1.2rem;
    border-collapse: separate;
    border-spacing: 0 10px;
}

.ranking-table th {
    border-radius: 8px;
    background-color: #a64444;
    /* Rojo borgoña para contraste */
    color: #ffffff;
    /* Letras blancas para que se lea perfecto */
    padding: 10px;
    text-align: center;
    font-family: 'Georgia', serif;
    font-size: 1.3rem;
    text-transform: uppercase;
    letter-spacing: 1px;
}

.ranking-table td {
    padding: 15px;
    border-bottom: 1px solid #f2d1d1;
}

/* Estilos para los 3 mejores */
.top-1 {
    background-color: rgba(255, 215, 0, 0.1);
    font-weight: bold;
    font-size: 1.4rem;
}

.top-2 {
    background-color: rgba(192, 192, 192, 0.1);
    font-weight: bold;
}

.top-3 {
    background-color: rgba(205, 127, 50, 0.1);
    font-weight: bold;
}

.pos-cell {
    width: 80px;
    text-align: center;
}

.name-cell {
    color: #333;
}

.score-cell {
    text-align: right;
    color: #a64444;
    font-weight: 900;
}

.btn-volver {
    margin-bottom: 10px;
    background-color: #733131;
    color: white;
    border: none;
    padding: 12px 30px;
    border-radius: 25px;
    cursor: pointer;
    transition: 0.3s;
}

.btn-volver:hover {
    background-color: #a64444;
    transform: translateX(-5px);
}

.loader,
.no-data {
    text-align: center;
    padding: 40px;
    color: #733131;
    font-style: italic;
}
</style>
