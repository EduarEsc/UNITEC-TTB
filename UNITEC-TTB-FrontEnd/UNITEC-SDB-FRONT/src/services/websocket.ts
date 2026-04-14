import { useGameStore } from '@/stores/game';

let socket: WebSocket | null = null;

export const connectWebSocket = () => {
    const gameStore = useGameStore();
    const WS_URL = 'ws://localhost:8000/ws/VUE';

    socket = new WebSocket(WS_URL);

    socket.onopen = () => {
        console.log('✅ WebSocket: Conectado con el Servidor');
        gameStore.isWsConnected = true;
    };

    socket.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);

            // Detectamos si es un evento de hardware o un resultado de lógica
            const tipo = data.type;

            switch (tipo) {
                // --- EVENTOS DE HARDWARE ---
                case 'HW_SELECT_CLICK':
                    gameStore.confirmarSeleccionMazo();
                    break;

                case 'HW_ROLL_DICE_TURNS':
                    gameStore.lanzarDadosTurno();
                    break;

                case 'HW_ROLL_DICE_CAT':
                    gameStore.lanzarDadoCategoria();
                    break;

                case 'HW_MAZO_MOVE':
                    if (gameStore.vistaActual === 'REGLAS') {
                        gameStore.cambiarTabReglas(data.dir === 'R' ? 'NEXT' : 'PREV');
                    } else {
                        gameStore.cambiarFocoMazo(data.dir === 'R' ? 'NEXT' : 'PREV');
                    }
                    break;

                // --- RESULTADOS DE LÓGICA (PROCESADOS POR PYTHON) ---
                case 'VALIDATION_RESULT':
                    gameStore.actualizarResultadoPalabra(data);
                    break;

                case 'GAME_OVER_TIMEOUT':
                    gameStore.detonarBomba();
                    break;
            }
        } catch (error) {
            console.error("❌ Error en recepción WebSocket:", error);
        }
    };

    socket.onclose = () => {
        gameStore.isWsConnected = false;
        setTimeout(() => connectWebSocket(), 3000);
    };
};

export const sendEvent = (type: string, payload: any = {}) => {
    if (socket?.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify({ type, ...payload }));
    }
};