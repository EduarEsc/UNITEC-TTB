import { defineStore } from 'pinia';
import { sendEvent } from '@/services/websocket';

export const useGameStore = defineStore('game', {
    state: () => ({
        vistaActual: 'REGLAS',
        isWsConnected: false,

        // Jugadores
        jugador1: { nickname: '', puntos: 0, vidas: 3 },
        jugador2: { nickname: '', puntos: 0, vidas: 3 },

        // Partida
        turnoActual: 1,
        categoriaActual: '',
        silabaActual: '',
        dificultadActual: '',
        bombaExplotada: false,

        // Mazo
        mazo: [] as any[],
        indexEnFoco: 0,
        cartaConfirmada: false,

        // Estado de voz e interacción
        micOn: false,
        ultimaPalabraEscuchada: '',
        esPalabraValida: false,

        ultimoMovimientoPalanca: 0,
        tabReglasActual: 0,
        TOTAL_TABS_REGLAS: 4,
        indexBotonModal: 0,
    }),

    actions: {
        // --- 1. MÉTODOS DE HARDWARE Y REGLAS (RECUPERADOS) ---

        cambiarTabReglas(dir: 'NEXT' | 'PREV') {
            if (dir === 'NEXT') {
                this.tabReglasActual = (this.tabReglasActual + 1) % this.TOTAL_TABS_REGLAS;
            } else {
                this.tabReglasActual = (this.tabReglasActual - 1 + this.TOTAL_TABS_REGLAS) % this.TOTAL_TABS_REGLAS;
            }
        },

        lanzarDadosTurno() {
            // Esto lo escucha el componente DadoTurno.vue vía $onAction
            console.log("🎲 Iniciando duelo de dados desde Hardware");
        },

        lanzarDadoCategoria() {
            // Esto lo escucha el componente DadoCategoria.vue vía $onAction
            console.log("🎲 Girando dado de categoría desde Hardware");
        },

        // --- 2. MANEJO DE MAZO Y SELECCIÓN ---

        async prepararMazoTrasDado(categoria: string) {
            this.categoriaActual = categoria;
            await this.cargarMazoDesdeBack();
            // Cambiamos a la sub-vista de cartas dentro de Configuración
            this.vistaActual = 'CONFIGURACION_CARTAS';
        },

        async cargarMazoDesdeBack() {
            try {
                const res = await fetch(`http://localhost:8000/mazo/${this.categoriaActual}`);
                this.mazo = await res.json();
            } catch (e) {
                console.error("Error mazo:", e);
            }
        },

        cambiarFocoMazo(direccion: 'NEXT' | 'PREV') {
            const ahora = Date.now();
            if (ahora - this.ultimoMovimientoPalanca < 250) return;
            this.ultimoMovimientoPalanca = ahora;

            if (direccion === 'NEXT') {
                this.indexEnFoco = (this.indexEnFoco + 1) % this.mazo.length;
            } else {
                this.indexEnFoco = (this.indexEnFoco - 1 + this.mazo.length) % this.mazo.length;
            }
        },

        cambiarFocoModal(direccion: 'NEXT' | 'PREV', maxOpciones: number) {
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
            // Lógica de navegación unificada para el botón SELECT
            if (this.vistaActual === 'REGLAS') {
                this.vistaActual = 'REGISTRO';
                sendEvent('UI_CAMBIO_FASE', { fase: 'REGISTRO' });
            }
            else if (this.vistaActual === 'REGISTRO') {
                if (this.jugador1.nickname && this.jugador2.nickname) {
                    this.vistaActual = 'CONFIGURACION';
                }
            }
            else if (this.vistaActual === 'CONFIGURACION_CARTAS') {
                this.confirmarCartaFinal();
            }
        },

        confirmarCartaFinal() {
            const carta = this.mazo[this.indexEnFoco];
            if (carta) {
                this.silabaActual = carta.silaba;
                this.dificultadActual = carta.dificultad;
                this.cartaConfirmada = true;

                // Avisar al backend y al ESP32 que todo está listo
                sendEvent('UI_CONFIG_COMPLETA', {
                    categoria: this.categoriaActual,
                    dificultad: this.dificultadActual,
                    silaba: this.silabaActual,
                    turno: this.turnoActual
                });

                // Pequeña espera para la animación de RevelacionCarta.vue
                setTimeout(() => {
                    this.vistaActual = 'JUEGO';
                    sendEvent('START_ROUND'); // Activa el Tic-Tac físico
                }, 1500);
            }
        },

        // --- 3. LÓGICA DE JUEGO ACTIVO ---

        cambiarTurno() {
            this.turnoActual = this.turnoActual === 1 ? 2 : 1;
            this.esPalabraValida = false;
            this.ultimaPalabraEscuchada = '';
            // Feedback al hardware
            sendEvent('CAMBIAR_TURNO_HARDWARE', { nuevoTurno: this.turnoActual });
        },

        actualizarResultadoPalabra(data: any) {
            // Veredicto del motor de voz del backend
            this.esPalabraValida = data.correct;
            this.ultimaPalabraEscuchada = data.palabra;

            if (data.correct) {
                // Sumar puntos al jugador en turno
                if (this.turnoActual === 1) this.jugador1.puntos += 10;
                else this.jugador2.puntos += 10;

                // Efecto de sonido de acierto
                sendEvent('SOLICITAR_AUDIO_ESP32', { id: 'SUCCESS' });

                this.cambiarTurno();
            }
        },

        detonarBomba() {
            this.bombaExplotada = true;
            console.log("💥 ¡BOOM! en Store");
        },

        descontarVida() {
            if (this.turnoActual === 1) {
                this.jugador1.vidas = Math.max(0, this.jugador1.vidas - 1);
            } else {
                this.jugador2.vidas = Math.max(0, this.jugador2.vidas - 1);
            }
        }
    }
});