<script setup lang="ts">
import { computed } from 'vue';
import { useGameStore } from '@/stores/game';
import { sendEvent } from '@/services/websocket';

const gameStore = useGameStore();

const allNavItems = [
    { name: 'REGLAS', icon: '📋', label: 'Reglas' },
    { name: 'REGISTRO', icon: '👤', label: 'Registro' },
    { name: 'CONFIGURACION', icon: '⚙️', label: 'Modos' },
    { name: 'RANKING', icon: '🏆', label: 'Ranking' },
];

const filteredNavItems = computed(() => {
    return allNavItems.filter(item => {
        if (item.name === 'CONFIGURACION') {
            return gameStore.jugador1.nickname !== '';
        }
        return true;
    });
});

const cambiarVista = (nombreVista: string) => {
    gameStore.vistaActual = nombreVista as any;
};

const reiniciarJuego = () => {
    // 1) avisamos al backend/ESP que detenga todo
    sendEvent('UI_RESET_GAME');

    // 2) reseteamos solo el juego/front
    gameStore.reiniciarPartidaCompleta();

    // 3) regresamos a reglas
    gameStore.vistaActual = 'REGLAS';

    // IMPORTANTE:
    // no tocar voiceReady
    // no liberar stream del micrófono
    console.log('🔄 Juego reiniciado a REGLAS sin perder permiso de micrófono');
};
</script>

<template>
    <aside class="side-nav">
        <div class="nav-top">
            <div class="side-brand">
                <span class="brand-icon">🎮</span>
            </div>

            <div class="nav-menu">
                <button v-for="item in filteredNavItems" :key="item.name" class="nav-icon-btn"
                    :class="{ active: gameStore.vistaActual === item.name }" @click="cambiarVista(item.name)">
                    <div class="icon-square" :title="item.label">
                        <span class="icon-text">{{ item.icon }}</span>
                    </div>
                    <span class="icon-label-text">{{ item.label }}</span>
                </button>
            </div>
        </div>

        <div class="nav-bottom">
            <button class="reset-btn" @click="reiniciarJuego" title="Reiniciar juego">
                <div class="icon-square reset">
                    <span class="icon-text">↺</span>
                </div>
                <span class="icon-label-text danger">Reiniciar</span>
            </button>

            <div class="ws-container">
                <div class="ws-status" :class="{ online: gameStore.isWsConnected }">
                    {{ gameStore.isWsConnected ? '●' : '○' }}
                </div>
                <span class="ws-label">{{ gameStore.isWsConnected ? 'Online' : 'Offline' }}</span>
            </div>
        </div>
    </aside>
</template>

<style scoped>
.side-nav {
    width: 108px;
    min-width: 108px;
    background:
        linear-gradient(180deg, rgba(255, 255, 255, 0.06) 0%, rgba(255, 255, 255, 0.03) 100%);
    border-right: 1px solid rgba(255, 255, 255, 0.08);
    display: flex;
    flex-direction: column;
    justify-content: space-between;
    align-items: center;
    padding: 18px 0 20px;
    backdrop-filter: blur(10px);
    box-shadow: 8px 0 24px rgba(0, 0, 0, 0.16);
}

.nav-top {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 24px;
}

.side-brand {
    width: 62px;
    height: 62px;
    border-radius: 18px;
    display: flex;
    justify-content: center;
    align-items: center;
    background: rgba(241, 196, 15, 0.12);
    border: 2px solid rgba(241, 196, 15, 0.4);
    box-shadow: 0 0 18px rgba(241, 196, 15, 0.14);
}

.brand-icon {
    font-size: 1.8rem;
}

.nav-menu {
    display: flex;
    flex-direction: column;
    gap: 18px;
    width: 100%;
    align-items: center;
}

.nav-icon-btn,
.reset-btn {
    background: none;
    border: none;
    cursor: pointer;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 7px;
    padding: 0;
    color: #bdc3c7;
    transition: all 0.25s ease;
}

.icon-square {
    width: 56px;
    height: 56px;
    background: rgba(255, 255, 255, 0.06);
    border-radius: 16px;
    display: flex;
    justify-content: center;
    align-items: center;
    color: #f5f6fa;
    border: 2px solid rgba(255, 255, 255, 0.08);
    box-shadow:
        0 10px 20px rgba(0, 0, 0, 0.22),
        0 0 10px rgba(255, 255, 255, 0.03);
    transition: all 0.25s ease;
}

.icon-square.reset {
    background: rgba(231, 76, 60, 0.12);
    border-color: rgba(231, 76, 60, 0.32);
}

.icon-label-text {
    font-size: 0.68rem;
    font-weight: 900;
    text-transform: uppercase;
    letter-spacing: 0.8px;
    text-align: center;
    color: #bdc3c7;
}

.icon-label-text.danger {
    color: #ffb3ad;
}

.nav-icon-btn:hover .icon-square,
.reset-btn:hover .icon-square {
    transform: translateY(-2px) scale(1.04);
}

.nav-icon-btn.active .icon-square {
    background: rgba(241, 196, 15, 0.14);
    color: #111;
    border-color: rgba(241, 196, 15, 0.58);
    box-shadow:
        0 0 18px rgba(241, 196, 15, 0.18),
        0 10px 22px rgba(0, 0, 0, 0.22);
}

.nav-icon-btn.active .icon-label-text {
    color: #fef9c3;
}

.reset-btn:hover .icon-square.reset {
    background: rgba(231, 76, 60, 0.2);
    box-shadow:
        0 0 18px rgba(231, 76, 60, 0.18),
        0 10px 20px rgba(0, 0, 0, 0.22);
}

.icon-text {
    font-size: 1.4rem;
}

.nav-bottom {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 18px;
}

.ws-container {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 2px;
}

.ws-status {
    font-size: 1.4rem;
    color: #e74c3c;
    text-shadow: 0 0 10px rgba(231, 76, 60, 0.2);
}

.ws-status.online {
    color: #2ecc71;
    text-shadow: 0 0 10px rgba(46, 204, 113, 0.26);
}

.ws-label {
    font-size: 0.62rem;
    font-weight: 900;
    letter-spacing: 0.8px;
    color: #95a5a6;
    text-transform: uppercase;
}

@media (max-width: 900px) {
    .side-nav {
        width: 88px;
        min-width: 88px;
    }

    .icon-square {
        width: 50px;
        height: 50px;
    }

    .icon-label-text {
        font-size: 0.62rem;
    }
}
</style>