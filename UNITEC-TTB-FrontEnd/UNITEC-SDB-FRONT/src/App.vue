<script setup lang="ts">
import { computed, onMounted, watch } from 'vue';
import { useGameStore } from './stores/game';
import { connectWebSocket, sendEvent } from '@/services/websocket';
import SideBar from '@/components/SideBar.vue';

import ReglasView from '@/views/ReglasView.vue';
import RegistroView from '@/views/RegistroView.vue';
import ConfiguracionView from '@/views/ConfiguracionView.vue';
import JuegoView from '@/views/JuegoView.vue';
import RankingView from '@/views/RankingView.vue';

const gameStore = useGameStore();

onMounted(() => {
  connectWebSocket();
});

// Mapeo dinámico de vistas
const views: Record<string, any> = {
  REGLAS: ReglasView,
  REGISTRO: RegistroView,
  CONFIGURACION: ConfiguracionView,
  CONFIGURACION_CARTAS: ConfiguracionView,
  JUEGO: JuegoView,
  RANKING: RankingView
};

const vistaActiva = computed(() => views[gameStore.vistaActual] || ReglasView);

// Vigilamos cambios de vista para avisar al backend (y que el ESP32 toque audios)
watch(() => gameStore.vistaActual, (nuevaVista) => {
  sendEvent('UI_CAMBIO_FASE', { fase: nuevaVista });
});
</script>

<template>
  <div class="layout">
    <header class="main-header">
      <div class="header-left">
        <img src="@/assets/logo.png" alt="Logo" class="header-logo">
        <h1 class="main-title">Protocolo Tic-Tac</h1>
      </div>
      <div class="user-profile">
        <span>{{ gameStore.jugador1.nickname || 'Jugador 1' }} vs {{ gameStore.jugador2.nickname || 'Jugador 2'
        }}</span>
      </div>
    </header>

    <div class="middle-container">
      <SideBar />
      <main class="main-content">
        <transition name="page-fade" mode="out-in">
          <component :is="vistaActiva" />
        </transition>
      </main>
    </div>

    <footer class="main-footer">
      <p>Mesa Creativa 2026 - Hardware Habilitado: {{ gameStore.isWsConnected ? '🟢 OK' : '🔴 Desconectado' }}</p>
    </footer>
  </div>
</template>

<style scoped>
/* Ajuste en el header para que el título se vea bien centrado */
.header-left {
  display: flex;
  align-items: center;
  flex: 1;
  /* Ocupa el espacio para empujar el perfil a la derecha */
}

.main-title {
  flex-grow: 1;
  /* Toma el espacio disponible */
  text-align: center;
  margin-left: 50px !important;
  color: rgb(79, 37, 37);
  font-family: 'Georgia', serif;
  font-style: italic;
  font-size: 4rem;
}

/* ESTRUCTURA GENERAL */
.layout {
  display: flex;
  flex-direction: column;
  height: 100vh;
  background-color: #f2d1d1;
  /* Fondo rosado Figma */
  color: #333;
  overflow: hidden;
}

/* HEADER */
.main-header {
  background-color: #a64444;
  /* Rojo Borgoña */
  height: 80px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0 30px;
  box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
  z-index: 10;
}

.header-left {
  display: flex;
  align-items: center;
  gap: 20px;
}

.logo-wrapper {
  background: white;
  padding: 5px;
  border-radius: 8px;
  display: flex;
  align-items: center;
}

.header-logo {
  width: 45px;
  height: 45px;
}

.user-profile {
  display: flex;
  align-items: center;
  gap: 15px;
  background: rgba(255, 255, 255, 0.2);
  padding: 5px 20px;
  border-radius: 30px;
  color: white;
}

.avatar-circle {
  width: 35px;
  height: 35px;
  background: #2a1a1a;
  border-radius: 50%;
  display: flex;
  justify-content: center;
  align-items: center;
}

/* MIDDLE CONTAINER */
.middle-container {
  display: flex;
  flex: 1;
  overflow: hidden;
}

/* SIDEBAR */
.side-nav {
  width: 80px;
  background-color: #d99a9a;
  /* Rosa fuerte Figma */
  display: flex;
  flex-direction: column;
  align-items: center;
  padding-top: 40px;
  gap: 25px;
}

.nav-icon-btn {
  background: none;
  border: none;
  cursor: pointer;
}

.icon-square {
  width: 50px;
  height: 50px;
  background-color: #733131;
  /* Rojo oscuro Figma */
  border-radius: 12px;
  display: flex;
  justify-content: center;
  align-items: center;
  color: white;
  box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.2);
  transition: transform 0.2s;
}

.icon-square:hover {
  transform: scale(1.1);
}

.ws-status {
  margin-top: auto;
  margin-bottom: 20px;
  font-size: 1.5rem;
  color: #733131;
}

.ws-status.online {
  color: #42b883;
}

/* MAIN CONTENT */
.main-content {
  flex: 1;
  padding: 40px;
  overflow-y: auto;
  display: flex;
  justify-content: center;
}

.main-footer {
  background-color: #733131;
  /* Rojo oscuro/borgoña del diseño */
  color: #d99a9a;
  /* Texto en tono rosado suave */
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px 50px;
  font-size: 0.8rem;
  line-height: 1.4;
  border-top: 2px solid rgba(0, 0, 0, 0.1);
}

.footer-column {
  flex: 1;
  display: flex;
  flex-direction: column;
  justify-content: center;
}

.footer-column.left {
  text-align: left;
}

.footer-column.right {
  text-align: right;
}

/* Contenedor de iconos centrales */
.footer-column.center {
  align-items: center;
  flex: 0.5;
  /* Ocupa menos espacio que los textos laterales */
}

.social-icons {
  display: flex;
  gap: 20px;
}

.social-circle {
  width: 45px;
  height: 45px;
  border-radius: 50%;
  display: flex;
  justify-content: center;
  align-items: center;
  color: white;
  font-weight: bold;
  font-size: 1.2rem;
  text-decoration: none;
  transition: transform 0.2s;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
}

.social-circle:hover {
  transform: scale(1.1);
}

/* Colores de marca para los iconos */
.fb {
  background-color: #1877F2;
}

.ig {
  background: linear-gradient(45deg, #f09433, #e6683c, #dc2743, #cc2366, #bc1888);
}

.tk {
  background-color: #000000;
}

@media (max-width: 768px) {
  .main-footer {
    flex-direction: column;
    gap: 20px;
    text-align: center;
  }

  .footer-column.left,
  .footer-column.right {
    text-align: center;
  }
}

/* ANIMACIONES */
.page-fade-enter-active,
.page-fade-leave-active {
  transition: all 0.3s ease;
}

.page-fade-enter-from {
  opacity: 0;
  transform: translateY(10px);
}

.page-fade-leave-to {
  opacity: 0;
  transform: translateY(-10px);
}

/* Asegura que el cuerpo ocupe el 100% */
.middle-container {
  display: flex;
  flex: 1;
  width: 100vw;
  /* Ocupa todo el ancho de la ventana */
  overflow: hidden;
}

.main-content {
  flex: 1;
  /* Esto obliga a que el contenido crezca y use todo el espacio sobrante */
  padding: 40px;
  overflow-y: auto;
  display: flex;
  flex-direction: column;
  align-items: center;
  /* Centra el contenido internamente */
  background-color: #f2d1d1;
  /* Asegura que el fondo cubra todo */
}

/* Ajusta el contenedor de las vistas para que no sea tan pequeño */
.reglas-container,
.registro-container,
.game-container {
  width: 100%;
  max-width: 1200px;
  /* Aumentamos el límite para pantallas grandes */
  margin: 0 auto;
}

.layout {
  display: flex;
  flex-direction: column;
  height: 100vh;
  width: 100vw;
  /* Forzamos ancho de ventana completa */
  background-color: #f2d1d1;
  margin: 0;
}
</style>