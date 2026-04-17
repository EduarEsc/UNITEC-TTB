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

onMounted(async () => {
  await connectWebSocket();
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

// Vigilamos cambios de vista para avisar al backend
watch(() => gameStore.vistaActual, (nuevaVista) => {
  // Estas fases se manejan dentro de ConfiguracionView por subpasos
  if (nuevaVista === 'CONFIGURACION' || nuevaVista === 'CONFIGURACION_CARTAS') {
    return;
  }

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
/* =========================
   ESTRUCTURA GENERAL
========================= */
.layout {
  display: flex;
  flex-direction: column;
  height: 100vh;
  width: 100vw;
  margin: 0;
  overflow: hidden;
  background:
    radial-gradient(circle at top, #1e272e 0%, #0f1418 45%, #050505 100%);
  color: #ecf0f1;
}

/* =========================
   HEADER
========================= */
.main-header {
  height: 88px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0 28px;
  background: rgba(255, 255, 255, 0.05);
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
  backdrop-filter: blur(8px);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.25);
  z-index: 20;
}

.header-left {
  display: flex;
  align-items: center;
  flex: 1;
  gap: 18px;
}

.header-logo {
  width: 50px;
  height: 50px;
  object-fit: contain;
  background: rgba(255, 255, 255, 0.08);
  border-radius: 14px;
  padding: 6px;
  box-shadow: 0 6px 18px rgba(0, 0, 0, 0.25);
}

.main-title {
  flex-grow: 1;
  text-align: center;
  margin-left: 40px !important;
  margin-bottom: 0;
  color: #f5f6fa;
  font-size: 2.7rem;
  font-weight: 900;
  letter-spacing: 1px;
  text-shadow: 0 3px 14px rgba(0, 0, 0, 0.35);
}

.user-profile {
  display: flex;
  align-items: center;
  gap: 12px;
  background: rgba(241, 196, 15, 0.12);
  border: 2px solid rgba(241, 196, 15, 0.55);
  color: #fef9c3;
  padding: 10px 18px;
  border-radius: 999px;
  box-shadow: 0 0 18px rgba(241, 196, 15, 0.15);
  font-weight: 800;
  font-size: 0.95rem;
}

/* =========================
   MIDDLE
========================= */
.middle-container {
  display: flex;
  flex: 1;
  width: 100vw;
  overflow: hidden;
}

.main-content {
  flex: 1;
  padding: 28px;
  overflow-y: auto;
  display: flex;
  flex-direction: column;
  align-items: center;
  background:
    radial-gradient(circle at center, rgba(255, 255, 255, 0.02) 0%, rgba(0, 0, 0, 0) 60%);
}

/* =========================
   FOOTER
========================= */
.main-footer {
  min-height: 54px;
  background: rgba(255, 255, 255, 0.04);
  color: #bdc3c7;
  display: flex;
  justify-content: center;
  align-items: center;
  padding: 10px 24px;
  font-size: 0.82rem;
  font-weight: 700;
  letter-spacing: 0.5px;
  border-top: 1px solid rgba(255, 255, 255, 0.08);
  box-shadow: 0 -8px 24px rgba(0, 0, 0, 0.18);
}

/* =========================
   ANIMACIONES
========================= */
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

/* =========================
   RESPONSIVE
========================= */
@media (max-width: 980px) {
  .main-title {
    font-size: 2rem;
    margin-left: 16px !important;
    text-align: left;
  }

  .user-profile {
    font-size: 0.82rem;
    padding: 8px 14px;
  }

  .main-content {
    padding: 18px;
  }
}

@media (max-width: 768px) {
  .main-header {
    padding: 0 16px;
    gap: 12px;
  }

  .main-title {
    font-size: 1.6rem;
  }

  .header-logo {
    width: 42px;
    height: 42px;
  }

  .user-profile {
    max-width: 40%;
    text-align: center;
    line-height: 1.2;
  }
}
</style>