<script setup lang="ts">
import { computed } from 'vue';
import { useGameStore } from '@/stores/game';

const gameStore = useGameStore();

// Definimos la estructura completa de navegación
const allNavItems = [
    { name: 'REGLAS', icon: '📋', label: 'Reglas' },
    { name: 'REGISTRO', icon: '👤', label: 'Registro' },
    { name: 'RANKING', icon: '🏆', label: 'Ranking' },
    { name: 'CONFIGURACION', icon: '⚙️', label: 'Modos' },
];

// Filtramos los items: CONFIGURACION solo aparece si hay un jugador registrado
const filteredNavItems = computed(() => {
    return allNavItems.filter(item => {
        if (item.name === 'CONFIGURACION') {
            // Solo mostramos "Modos" si el jugador 1 ya ingresó su nombre
            return gameStore.jugador1.nickname !== '';
        }
        return true; // Las demás siempre se muestran
    });
});

const cambiarVista = (nombreVista: string) => {
    gameStore.vistaActual = nombreVista;
};
</script>

<template>
    <aside class="side-nav">
        <div class="nav-menu">
            <button v-for="item in filteredNavItems" :key="item.name" class="nav-icon-btn"
                :class="{ active: gameStore.vistaActual === item.name }" @click="cambiarVista(item.name)">
                <div class="icon-square" :title="item.label">
                    <span class="icon-text">{{ item.icon }}</span>
                </div>
                <span class="icon-label-text">{{ item.label }}</span>
            </button>
        </div>

        <div class="ws-container">
            <div class="ws-status" :class="{ online: gameStore.isWsConnected }">
                {{ gameStore.isWsConnected ? '●' : '○' }}
            </div>
            <span class="ws-label">{{ gameStore.isWsConnected ? 'Online' : 'Offline' }}</span>
        </div>
    </aside>
</template>

<style scoped>
.side-nav {
    width: 90px;
    background-color: #d99a9a;
    display: flex;
    flex-direction: column;
    align-items: center;
    padding: 30px 0;
}

.nav-menu {
    margin-top: 100px;
    display: flex;
    flex-direction: column;
    gap: 25px;
}

.nav-icon-btn {
    background: none;
    border: none;
    cursor: pointer;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 5px;
    padding: 0;
    color: #733131;
    transition: all 0.3s ease;
}

.icon-square {
    width: 50px;
    height: 50px;
    background-color: #733131;
    border-radius: 12px;
    display: flex;
    justify-content: center;
    align-items: center;
    color: white;
    box-shadow: 2px 4px 8px rgba(0, 0, 0, 0.2);
    transition: all 0.2s ease;
}

.icon-label-text {
    font-size: 0.7rem;
    font-weight: bold;
    text-transform: uppercase;
}

/* Efectos de Hover y Activo */
.nav-icon-btn:hover .icon-square {
    transform: translateY(-3px);
    background-color: #8c3d3d;
}

.nav-icon-btn.active .icon-square {
    background-color: #f2d1d1;
    color: #733131;
    border: 2px solid #733131;
    box-shadow: inset 2px 2px 4px rgba(0, 0, 0, 0.1);
}

.icon-text {
    font-size: 1.4rem;
}

.ws-container {
    margin-top: auto;
    display: flex;
    flex-direction: column;
    align-items: center;
}

.ws-status {
    font-size: 1.5rem;
    color: #733131;
}

.ws-status.online {
    color: #42b883;
}

.ws-label {
    font-size: 0.6rem;
    font-weight: bold;
    color: #733131;
}
</style>