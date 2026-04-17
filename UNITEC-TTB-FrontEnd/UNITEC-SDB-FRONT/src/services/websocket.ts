import { useGameStore } from '@/stores/game';
import {
    initVoiceCapture,
    startVoiceCapture,
    stopVoiceCaptureAndUpload,
    isVoiceCaptureRecording
} from '@/services/voiceCapture';

let socket: WebSocket | null = null;
let reconnectTimeout: number | null = null;

export const connectWebSocket = async () => {
    const gameStore = useGameStore();
    const WS_URL = 'ws://192.168.1.70:8000/ws/VUE';

    if (socket && (socket.readyState === WebSocket.OPEN || socket.readyState === WebSocket.CONNECTING)) {
        return;
    }

    // Pedimos permiso una sola vez al cargar la app para que sea discreto en la demo
    if (!gameStore.voiceReady) {
        const ok = await initVoiceCapture();
        gameStore.setVoiceReady(ok);
    }

    socket = new WebSocket(WS_URL);

    socket.onopen = () => {
        console.log('✅ WebSocket: Conectado con el Servidor');
        gameStore.isWsConnected = true;

        if (reconnectTimeout) {
            clearTimeout(reconnectTimeout);
            reconnectTimeout = null;
        }
    };

    socket.onmessage = async (event) => {
        try {
            const data = JSON.parse(event.data);
            const tipo = data.type;

            switch (tipo) {
                // ============================
                // EVENTOS DE HARDWARE
                // ============================
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
                    } else if (
                        gameStore.vistaActual === 'CONFIGURACION_CARTAS' &&
                        !gameStore.cartaConfirmada
                    ) {
                        gameStore.cambiarFocoMazo(data.dir === 'R' ? 'NEXT' : 'PREV');
                    } else if (gameStore.modalActivo !== 'NINGUNO') {
                        gameStore.cambiarFocoModal(
                            data.dir === 'R' ? 'NEXT' : 'PREV',
                            gameStore.maxOpcionesModal
                        );
                    }
                    break;

                // ============================
                // EVENTOS DE TIMER DESDE BACK
                // ============================
                case 'TIMER_SYNC':
                    gameStore.actualizarTimerDesdeBack(data);
                    break;

                // ============================
                // EVENTOS DE MICRÓFONO / VOZ
                // ============================
                case 'MIC_ACTIVE':
                    if (gameStore.timerState === 'stopped') {
                        console.warn('⚠️ MIC_ACTIVE ignorado porque la ronda ya está detenida.');
                        break;
                    }

                    if (gameStore.micOn || gameStore.micProcessing) {
                        console.warn('⚠️ MIC_ACTIVE ignorado porque ya hay captura/procesamiento en curso.');
                        break;
                    }

                    gameStore.iniciarEscuchaMicrofono(data.player);

                    if (gameStore.voiceReady) {
                        await startVoiceCapture(data.player);
                    } else {
                        console.warn('⚠️ La captura de voz no está lista.');
                    }
                    break;

                case 'MIC_INACTIVE':
                    gameStore.detenerEscuchaMicrofono(data.player, data.bytes ?? 0);

                    if (isVoiceCaptureRecording()) {
                        try {
                            await stopVoiceCaptureAndUpload(data.player);
                        } catch (error) {
                            console.error('❌ Error subiendo audio al backend:', error);
                            gameStore.procesarPalabraIncorrecta({
                                type: 'WORD_INCORRECT',
                                player: data.player,
                                reason: 'No se pudo enviar el audio al backend'
                            });
                        }
                    } else {
                        console.warn('⚠️ MIC_INACTIVE recibido pero ya no había grabación activa.');
                    }
                    break;

                case 'MIC_PROCESSING':
                    gameStore.iniciarProcesamientoVoz(data.player);
                    break;

                case 'WORD_CORRECT':
                    gameStore.procesarPalabraCorrecta(data);
                    break;

                case 'WORD_INCORRECT':
                    gameStore.procesarPalabraIncorrecta(data);
                    break;

                // ============================
                // EVENTOS DE JUEGO
                // ============================
                case 'GAME_OVER_TIMEOUT':
                    gameStore.detonarBomba();
                    break;

                default:
                    console.log('ℹ️ Evento WS no manejado:', data);
                    break;
            }
        } catch (error) {
            console.error('❌ Error en recepción WebSocket:', error, event.data);
        }
    };

    socket.onclose = () => {
        console.warn('⚠️ WebSocket cerrado. Reintentando...');
        gameStore.isWsConnected = false;

        if (!reconnectTimeout) {
            reconnectTimeout = window.setTimeout(() => {
                reconnectTimeout = null;
                connectWebSocket();
            }, 3000);
        }
    };

    socket.onerror = (error) => {
        console.error('❌ Error WebSocket:', error);
    };
};

export const sendEvent = (type: string, payload: any = {}) => {
    if (socket?.readyState === WebSocket.OPEN) {
        socket.send(JSON.stringify({ type, ...payload }));
    } else {
        console.warn('⚠️ WebSocket no está listo. Evento no enviado:', type, payload);
    }
};