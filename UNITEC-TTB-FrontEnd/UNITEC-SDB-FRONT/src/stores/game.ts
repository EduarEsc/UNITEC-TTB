import { defineStore } from 'pinia';
import { sendEvent } from '@/services/websocket';

type VistaJuego =
    | 'REGLAS'
    | 'REGISTRO'
    | 'CONFIGURACION'
    | 'CONFIGURACION_CARTAS'
    | 'JUEGO';

type ModalActivo = 'NINGUNO' | 'BOOM' | 'WINNER';
type ConfigStep = 'TURNO' | 'CATEGORIA' | 'CARTA' | 'REVELACION';

interface JugadorState {
    nickname: string;
    puntos: number;
    vidas: number;
}

interface ResultadoVozPayload {
    type: 'WORD_CORRECT' | 'WORD_INCORRECT';
    player?: number;
    word?: string;
    heard?: string;
    reason?: string;
}

interface TimerSyncPayload {
    type: 'TIMER_SYNC';
    state: 'started' | 'paused' | 'resumed' | 'stopped';
    reason?: string;
    totalMs: number;
    remainingMs: number;
}

export const useGameStore = defineStore('game', {
    state: () => ({
        vistaActual: 'REGLAS' as VistaJuego,
        configStep: 'TURNO' as ConfigStep,
        isWsConnected: false,

        jugador1: { nickname: '', puntos: 0, vidas: 3 } as JugadorState,
        jugador2: { nickname: '', puntos: 0, vidas: 3 } as JugadorState,

        turnoActual: 1,
        categoriaActual: '',
        silabaActual: '',
        dificultadActual: '',
        bombaExplotada: false,

        mazo: [] as any[],
        indexEnFoco: 0,
        cartaConfirmada: false,
        cartaListaParaIniciar: false,

        micOn: false,
        micPlayer: 0,
        micProcessing: false,
        ultimoAudioBytes: 0,
        ultimaPalabraEscuchada: '',
        ultimaPalabraValidada: '',
        ultimoMotivoError: '',
        esPalabraValida: null as boolean | null,

        rondaFinalizadaConAcierto: false,

        ultimoMovimientoPalanca: 0,
        tabReglasActual: 0,
        TOTAL_TABS_REGLAS: 4,
        indexBotonModal: 0,
        modalActivo: 'NINGUNO' as ModalActivo,

        timerTotalMs: 10000,
        timerRemainingMs: 10000,
        timerState: 'stopped' as 'started' | 'paused' | 'resumed' | 'stopped',
        timerReason: '',
        timerLastSyncAt: 0,

        voiceReady: false,
        selectBloqueadoHasta: 0,
        vidaYaDescontadaEnTurno: false,
    }),

    getters: {
        nombreJugadorEnTurno(state): string {
            return state.turnoActual === 1
                ? state.jugador1.nickname || 'Jugador 1'
                : state.jugador2.nickname || 'Jugador 2';
        },

        nombreJugadorMicActivo(state): string {
            if (state.micPlayer === 1) return state.jugador1.nickname || 'Jugador 1';
            if (state.micPlayer === 2) return state.jugador2.nickname || 'Jugador 2';

            return state.turnoActual === 1
                ? state.jugador1.nickname || 'Jugador 1'
                : state.jugador2.nickname || 'Jugador 2';
        },

        maxOpcionesModal(state): number {
            if (state.modalActivo === 'BOOM') {
                const puedeContinuar = state.jugador1.vidas > 0 && state.jugador2.vidas > 0;
                return puedeContinuar ? 3 : 2;
            }
            if (state.modalActivo === 'WINNER') return 2;
            return 0;
        },

        tiempoRestanteSegundos(state): number {
            return Math.max(0, Math.ceil(state.timerRemainingMs / 1000));
        },

        timerPausado(state): boolean {
            return state.timerState === 'paused';
        }
    },

    actions: {
        setVoiceReady(ready: boolean) {
            this.voiceReady = ready;
        },

        setConfigStep(step: ConfigStep) {
            this.configStep = step;
        },

        bloquearSelectPor(ms: number) {
            this.selectBloqueadoHasta = Date.now() + ms;
        },

        prepararVistaCartas() {
            this.indexEnFoco = 0;
            this.cartaConfirmada = false;
            this.cartaListaParaIniciar = false;
            this.silabaActual = '';
            this.dificultadActual = '';
        },

        actualizarTimerDesdeBack(data: TimerSyncPayload) {
            this.timerTotalMs = data.totalMs ?? this.timerTotalMs;
            this.timerRemainingMs = Math.max(0, data.remainingMs ?? this.timerRemainingMs);
            this.timerState = data.state ?? this.timerState;
            this.timerReason = data.reason ?? '';
            this.timerLastSyncAt = Date.now();

            if (data.state === 'started') {
                this.bombaExplotada = false;
                this.modalActivo = 'NINGUNO';
                this.micOn = false;
                this.micProcessing = false;
                this.esPalabraValida = null;
                this.ultimaPalabraEscuchada = '';
                this.ultimaPalabraValidada = '';
                this.ultimoMotivoError = '';
            }

            console.log('⏱️ TIMER_SYNC:', {
                state: this.timerState,
                reason: this.timerReason,
                totalMs: this.timerTotalMs,
                remainingMs: this.timerRemainingMs
            });
        },

        cambiarTabReglas(dir: 'NEXT' | 'PREV') {
            if (dir === 'NEXT') {
                this.tabReglasActual = (this.tabReglasActual + 1) % this.TOTAL_TABS_REGLAS;
            } else {
                this.tabReglasActual =
                    (this.tabReglasActual - 1 + this.TOTAL_TABS_REGLAS) % this.TOTAL_TABS_REGLAS;
            }
        },

        lanzarDadosTurno() {
            console.log('🎲 Iniciando duelo de dados desde Hardware');
        },

        lanzarDadoCategoria() {
            console.log('🎲 Girando dado de categoría desde Hardware');
        },

        async prepararMazoTrasDado(categoria: string) {
            this.categoriaActual = categoria;
            this.prepararVistaCartas();

            await this.cargarMazoDesdeBack();

            this.vistaActual = 'CONFIGURACION_CARTAS';
        },

        async cargarMazoDesdeBack() {
            try {
                const res = await fetch(`http://localhost:8000/mazo/${this.categoriaActual}`);
                this.mazo = await res.json();
                this.indexEnFoco = 0;
            } catch (e) {
                console.error('❌ Error mazo:', e);
                this.mazo = [];
            }
        },

        cambiarFocoMazo(direccion: 'NEXT' | 'PREV') {
            if (!this.mazo.length) return;
            if (this.configStep !== 'CARTA') return;
            if (this.cartaConfirmada) return;

            const ahora = Date.now();
            if (ahora - this.ultimoMovimientoPalanca < 180) return;
            this.ultimoMovimientoPalanca = ahora;

            if (direccion === 'NEXT') {
                this.indexEnFoco = (this.indexEnFoco + 1) % this.mazo.length;
            } else {
                this.indexEnFoco = (this.indexEnFoco - 1 + this.mazo.length) % this.mazo.length;
            }
        },

        cambiarFocoModal(direccion: 'NEXT' | 'PREV', maxOpciones: number) {
            if (maxOpciones <= 0) return;

            if (direccion === 'NEXT') {
                this.indexBotonModal = (this.indexBotonModal + 1) % maxOpciones;
            } else {
                this.indexBotonModal = (this.indexBotonModal - 1 + maxOpciones) % maxOpciones;
            }
        },

        resetFocoModal() {
            this.indexBotonModal = 0;
        },

        confirmarSeleccionMazo() {
            if (Date.now() < this.selectBloqueadoHasta) {
                console.log('⛔ SELECT ignorado por bloqueo temporal');
                return;
            }
            if (this.modalActivo !== 'NINGUNO') {
                return;
            }

            if (this.vistaActual === 'REGLAS') {
                this.vistaActual = 'REGISTRO';
                sendEvent('UI_CAMBIO_FASE', { fase: 'REGISTRO' });
                return;
            }

            if (this.vistaActual === 'REGISTRO') {
                if (this.jugador1.nickname && this.jugador2.nickname) {
                    this.vistaActual = 'CONFIGURACION';
                    this.configStep = 'TURNO';
                }
                return;
            }

            // AQUI ESTÁ LA CORRECCIÓN IMPORTANTE:
            // solo puede confirmar carta si realmente estamos en subpaso CARTA
            if (this.vistaActual === 'CONFIGURACION_CARTAS' && this.configStep === 'CARTA') {
                this.confirmarCartaFinal();
                return;
            }

            if (this.vistaActual === 'CONFIGURACION_CARTAS' && this.configStep === 'REVELACION') {
                this.iniciarJuegoTrasRevelacion();
                return;
            }
        },

        confirmarCartaFinal() {
            if (this.configStep !== 'CARTA') return;

            const carta = this.mazo[this.indexEnFoco];
            if (!carta) return;

            this.silabaActual = carta.silaba;
            this.dificultadActual = carta.dificultad;
            this.cartaConfirmada = true;
            this.cartaListaParaIniciar = false;
            this.configStep = 'REVELACION';

            this.resetEstadoRonda();

            sendEvent('UI_CONFIG_COMPLETA', {
                categoria: this.categoriaActual,
                dificultad: this.dificultadActual,
                silaba: this.silabaActual,
                turno: this.turnoActual
            });
        },

        marcarCartaListaParaIniciar() {
            this.cartaListaParaIniciar = true;
        },

        iniciarJuegoTrasRevelacion() {
            if (!this.cartaConfirmada || !this.cartaListaParaIniciar) return;

            this.asignarVidasPorDificultad();
            this.vistaActual = 'JUEGO';
            this.cartaListaParaIniciar = false;
            sendEvent('START_ROUND');
        },

        resetEstadoVoz() {
            this.micOn = false;
            this.micPlayer = 0;
            this.micProcessing = false;
            this.ultimoAudioBytes = 0;
            this.ultimaPalabraEscuchada = '';
            this.ultimaPalabraValidada = '';
            this.ultimoMotivoError = '';
            this.esPalabraValida = null;
        },

        resetEstadoRonda() {
            this.resetEstadoVoz();
            this.bombaExplotada = false;
            this.rondaFinalizadaConAcierto = false;
            this.modalActivo = 'NINGUNO';
            this.vidaYaDescontadaEnTurno = false;
            this.resetFocoModal();
        },

        marcarVidaDescontada() {
            this.vidaYaDescontadaEnTurno = true;
        },

        prepararSiguienteTurnoTrasAcierto() {
            this.resetEstadoVoz();
            this.bombaExplotada = false;
            this.rondaFinalizadaConAcierto = false;
            this.esPalabraValida = null;
            this.ultimaPalabraEscuchada = '';
            this.ultimaPalabraValidada = '';
            this.ultimoMotivoError = '';

            this.cambiarTurno();

            sendEvent('NEXT_TURN', {
                player: this.turnoActual,
                totalMs: 10000
            });
        },

        cambiarTurno() {
            this.turnoActual = this.turnoActual === 1 ? 2 : 1;
        },

        asignarVidasPorDificultad() {
            let vidasBase = 3;

            if (this.dificultadActual?.toLowerCase() === 'facil') {
                vidasBase = 1;
            } else if (this.dificultadActual?.toLowerCase() === 'medio') {
                vidasBase = 2;
            } else if (this.dificultadActual?.toLowerCase() === 'dificil') {
                vidasBase = 3;
            }

            this.jugador1.vidas = vidasBase;
            this.jugador2.vidas = vidasBase;
        },

        prepararSiguienteRonda() {
            this.rondaFinalizadaConAcierto = false;
            this.resetEstadoVoz();
            this.bombaExplotada = false;
            this.modalActivo = 'NINGUNO';
        },

        iniciarEscuchaMicrofono(player: number) {
            this.micOn = true;
            this.micPlayer = player;
            this.micProcessing = false;
            this.ultimoAudioBytes = 0;
            this.ultimaPalabraEscuchada = '';
            this.ultimaPalabraValidada = '';
            this.ultimoMotivoError = '';
            this.esPalabraValida = null;

            console.log(`🎙️ Captura de voz activa para jugador ${player}`);
        },

        detenerEscuchaMicrofono(player: number, bytes: number = 0) {
            this.micOn = false;
            this.micPlayer = player;
            this.ultimoAudioBytes = bytes;

            console.log(`🛑 Captura de voz detenida para jugador ${player}. Bytes=${bytes}`);
        },

        iniciarProcesamientoVoz(player: number) {
            this.micOn = false;
            this.micPlayer = player;
            this.micProcessing = true;

            console.log(`🧠 Procesando voz del jugador ${player}`);
        },

        procesarPalabraCorrecta(data: ResultadoVozPayload) {
            const player = data.player ?? this.turnoActual;
            const palabraFinal = data.word ?? '';
            const palabraEscuchada = data.heard ?? palabraFinal;

            this.micOn = false;
            this.micProcessing = false;
            this.micPlayer = player;
            this.esPalabraValida = true;
            this.ultimaPalabraEscuchada = palabraEscuchada;
            this.ultimaPalabraValidada = palabraFinal;
            this.ultimoMotivoError = '';

            if (player === 1) this.jugador1.puntos += 10;
            else this.jugador2.puntos += 10;

            console.log(`✅ Palabra correcta. Jugador ${player}: ${palabraFinal}`);

            window.setTimeout(() => {
                this.prepararSiguienteTurnoTrasAcierto();
            }, 2800);
        },

        procesarPalabraIncorrecta(data: ResultadoVozPayload) {
            const player = data.player ?? this.turnoActual;
            const palabraEscuchada = data.heard ?? '';
            const reason = data.reason ?? 'Palabra inválida';

            this.micOn = false;
            this.micProcessing = false;
            this.micPlayer = player;
            this.esPalabraValida = false;
            this.ultimaPalabraEscuchada = palabraEscuchada;
            this.ultimaPalabraValidada = '';
            this.ultimoMotivoError = reason;

            if (player === 1) {
                this.jugador1.vidas = Math.max(0, this.jugador1.vidas - 1);
            } else {
                this.jugador2.vidas = Math.max(0, this.jugador2.vidas - 1);
            }

            console.log(`❌ Palabra incorrecta. Jugador ${player}: ${reason}`);
        },

        detonarBomba() {
            this.bombaExplotada = true;
            this.micOn = false;
            this.micProcessing = false;
            this.modalActivo = 'BOOM';
            console.log('💥 ¡BOOM! en Store');
        },

        descontarVida() {
            if (this.turnoActual === 1) {
                this.jugador1.vidas = Math.max(0, this.jugador1.vidas - 1);
            } else {
                this.jugador2.vidas = Math.max(0, this.jugador2.vidas - 1);
            }
        },

        abrirModalWinner() {
            this.modalActivo = 'WINNER';
            this.resetFocoModal();
        },

        cerrarModales() {
            this.modalActivo = 'NINGUNO';
            this.resetFocoModal();
        },

        reiniciarPartidaCompleta() {
            this.turnoActual = 1;
            this.categoriaActual = '';
            this.silabaActual = '';
            this.dificultadActual = '';
            this.bombaExplotada = false;

            this.mazo = [];
            this.indexEnFoco = 0;
            this.cartaConfirmada = false;
            this.cartaListaParaIniciar = false;
            this.configStep = 'TURNO';

            this.jugador1.puntos = 0;
            this.jugador1.vidas = 3;

            this.jugador2.puntos = 0;
            this.jugador2.vidas = 3;

            this.timerTotalMs = 10000;
            this.timerRemainingMs = 10000;
            this.timerState = 'stopped';
            this.timerReason = '';
            this.timerLastSyncAt = 0;

            this.resetEstadoRonda();
        }
    }
});