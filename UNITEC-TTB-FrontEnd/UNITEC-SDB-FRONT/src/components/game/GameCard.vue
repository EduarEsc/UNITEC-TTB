<script setup lang="ts">
import { computed } from 'vue';

interface Props {
    categoria: string;
    silaba: string;
    dificultad: string;
}

const props = defineProps<Props>();

const dificultadClase = computed(() => {
    const d = props.dificultad.toLowerCase();
    if (d.includes('fácil') || d.includes('facil')) return 'facil';
    if (d.includes('difícil') || d.includes('dificil')) return 'dificil';
    return 'medio';
});

// Mostramos la penalización/recompensa visual de la carta
const resistenciaTexto = computed(() => {
    if (dificultadClase.value === 'facil') return 'Baja';
    if (dificultadClase.value === 'medio') return 'Media';
    return '¡MÁXIMA!';
});
</script>

<template>
    <div class="card-visual" :class="dificultadClase">
        <div class="card-inner">
            <div class="card-header">
                <span class="category-badge">{{ props.categoria || 'Cargando...' }}</span>
            </div>

            <div class="syllable-container">
                <Transition name="pop" mode="out-in">
                    <h1 :key="props.silaba" class="syllable">
                        {{ props.silaba || '??' }}
                    </h1>
                </Transition>
            </div>

            <div class="card-footer">
                <div class="difficulty-info">
                    <span class="difficulty-label">{{ props.dificultad }}</span>
                    <div class="hearts-display">
                        <span v-for="n in (dificultadClase === 'facil' ? 1 : dificultadClase === 'medio' ? 2 : 3)"
                            :key="n" class="heart">❤️</span>
                    </div>
                </div>
            </div>
        </div>
        <div class="card-glow"></div>
    </div>
</template>

<style scoped>
.card-visual {
    background: white;
    width: 280px;
    height: 380px;
    border-radius: 25px;
    color: #2c3e50;
    position: relative;
    overflow: hidden;
    display: flex;
    flex-direction: column;
    box-shadow: 15px 15px 0px rgba(0, 0, 0, 0.2), 0 10px 30px rgba(0, 0, 0, 0.5);
    transition: all 0.5s cubic-bezier(0.175, 0.885, 0.32, 1.275);
    z-index: 2;
}

/* --- VARIANTES POR DIFICULTAD --- */
.facil {
    border-bottom: 15px solid #2ecc71;
    box-shadow: 0 0 20px rgba(46, 204, 113, 0.2);
}

.medio {
    border-bottom: 15px solid #f1c40f;
    box-shadow: 0 0 20px rgba(241, 196, 15, 0.2);
}

.dificil {
    border-bottom: 15px solid #e74c3c;
    box-shadow: 0 0 20px rgba(231, 76, 60, 0.2);
}

.card-inner {
    position: relative;
    z-index: 3;
    display: flex;
    flex-direction: column;
    height: 100%;
    padding: 25px;
    justify-content: space-between;
}

.category-badge {
    background: #f1f2f6;
    padding: 6px 15px;
    border-radius: 20px;
    font-size: 0.8rem;
    font-weight: 800;
    color: #7f8c8d;
    text-transform: uppercase;
    letter-spacing: 1px;
}

.syllable-container {
    flex-grow: 1;
    display: flex;
    align-items: center;
    justify-content: center;
}

.syllable {
    font-size: 6.5rem;
    margin: 0;
    font-weight: 900;
    text-transform: uppercase;
    color: #2c3e50;
    text-shadow: 4px 4px 0px rgba(0, 0, 0, 0.05);
}

.card-footer {
    border-top: 1px solid #eee;
    padding-top: 15px;
}

.difficulty-tag {
    font-weight: 900;
    font-size: 0.9rem;
    display: flex;
    align-items: center;
    gap: 8px;
    margin-bottom: 5px;
}

.dot {
    width: 10px;
    height: 10px;
    border-radius: 50%;
}

/* Colores de los puntos según dificultad */
.facil .dot {
    background: #2ecc71;
}

.medio .dot {
    background: #f1c40f;
}

.dificil .dot {
    background: #e74c3c;
}

.resistencia-info {
    font-size: 0.75rem;
    color: #95a5a6;
    font-weight: bold;
}

.card-pattern {
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background-image: radial-gradient(#f1f2f6 1px, transparent 1px);
    background-size: 20px 20px;
    opacity: 0.4;
    z-index: 1;
}

/* --- ANIMACIÓN DE CAMBIO DE SÍLABA --- */
.slide-up-enter-active,
.slide-up-leave-active {
    transition: all 0.4s ease;
}

.slide-up-enter-from {
    opacity: 0;
    transform: translateY(30px) scale(0.8);
}

.slide-up-leave-to {
    opacity: 0;
    transform: translateY(-30px) scale(1.2);
}
</style>