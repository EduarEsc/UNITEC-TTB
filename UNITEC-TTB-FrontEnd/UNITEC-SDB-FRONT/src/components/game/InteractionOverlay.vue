<script setup lang="ts">
import { computed } from 'vue';

interface Props {
    micOn: boolean;
    micProcessing?: boolean;
    nombreJugadorActivo: string;
    ultimaPalabraEscuchada: string;
    ultimaPalabraValidada?: string;
    esPalabraValida: boolean | null;
    ultimoMotivoError?: string;
}

const props = withDefaults(defineProps<Props>(), {
    micProcessing: false,
    ultimaPalabraValidada: '',
    ultimoMotivoError: ''
});

const estadoOverlay = computed(() => {
    if (props.micOn) return 'listening';
    if (props.micProcessing) return 'processing';
    if (props.esPalabraValida === true) return 'correct';
    if (props.esPalabraValida === false) return 'incorrect';
    return 'waiting';
});

const mensajePrincipal = computed(() => {
    switch (estadoOverlay.value) {
        case 'listening':
            return `CAPTURANDO RESPUESTA DE ${props.nombreJugadorActivo.toUpperCase()}...`;
        case 'processing':
            return `VALIDANDO RESPUESTA DE ${props.nombreJugadorActivo.toUpperCase()}...`;
        case 'correct':
            return `¡RESPUESTA CORRECTA DE ${props.nombreJugadorActivo.toUpperCase()}!`;
        case 'incorrect':
            return `RESPUESTA INVÁLIDA DE ${props.nombreJugadorActivo.toUpperCase()}`;
        default:
            return `ESPERANDO TURNO DE ${props.nombreJugadorActivo.toUpperCase()}...`;
    }
});

const palabraMostrada = computed(() => {
    if (props.esPalabraValida === true) {
        return props.ultimaPalabraValidada || props.ultimaPalabraEscuchada;
    }
    return props.ultimaPalabraEscuchada;
});
</script>

<template>
    <div class="interaction-zone">
        <Transition name="fade-down" mode="out-in">
            <div v-if="estadoOverlay === 'listening'" key="listening" class="active-feedback">
                <div class="mic-status active listening">
                    <div class="pulse-ring"></div>
                    <div class="mic-icon">🎙️</div>
                    <p class="listening-text">
                        <strong>{{ props.nombreJugadorActivo }}</strong> está respondiendo
                    </p>
                    <p class="status-msg">{{ mensajePrincipal }}</p>
                    <p class="hint-msg">Presiona de nuevo el botón físico para enviar la respuesta.</p>
                </div>
            </div>

            <div v-else-if="estadoOverlay === 'processing'" key="processing" class="active-feedback">
                <div class="mic-status processing">
                    <div class="mic-icon">📡</div>
                    <p class="listening-text">Respuesta capturada</p>
                    <p class="status-msg">{{ mensajePrincipal }}</p>
                    <p class="hint-msg">Analizando la palabra pronunciada...</p>
                </div>
            </div>

            <div v-else-if="estadoOverlay === 'correct'" key="correct" class="active-feedback">
                <div class="mic-status result success">
                    <div class="mic-icon">✅</div>
                    <p class="status-msg">{{ mensajePrincipal }}</p>

                    <div v-if="palabraMostrada" class="word-preview">
                        <span class="valid-text">"{{ palabraMostrada }}"</span>
                    </div>

                    <p class="hint-msg">Preparando siguiente ronda...</p>
                </div>
            </div>

            <div v-else-if="estadoOverlay === 'incorrect'" key="incorrect" class="active-feedback">
                <div class="mic-status result error">
                    <div class="mic-icon">❌</div>
                    <p class="status-msg">{{ mensajePrincipal }}</p>

                    <div v-if="palabraMostrada" class="word-preview">
                        <span class="invalid-text">"{{ palabraMostrada }}"</span>
                    </div>

                    <p v-if="props.ultimoMotivoError" class="hint-msg">
                        {{ props.ultimoMotivoError }}
                    </p>
                </div>
            </div>

            <div v-else key="waiting" class="mic-status inactive">
                <div class="mic-icon muted">🔇</div>
                <p class="status-msg">{{ mensajePrincipal }}</p>
            </div>
        </Transition>
    </div>
</template>

<style scoped>
.interaction-zone {
    width: 100%;
    display: flex;
    justify-content: center;
    align-items: center;
    margin-top: 30px;
    position: relative;
    height: 120px;
    /* Altura fija para evitar saltos en la UI */
    z-index: 10;
}

.active-feedback {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 15px;
}

.mic-status {
    display: flex;
    align-items: center;
    gap: 12px;
    background: rgba(0, 0, 0, 0.5);
    padding: 10px 20px;
    border-radius: 25px;
    border: 2px solid transparent;
    transition: all 0.3s ease;
}

.mic-status.active {
    background: rgba(46, 204, 113, 0.1);
    border-color: #2ecc71;
    position: relative;
}

.mic-status.inactive {
    background: rgba(255, 255, 255, 0.05);
    opacity: 0.6;
}

.mic-icon {
    font-size: 1.5rem;
}

.mic-icon.muted {
    filter: grayscale(1);
}

.pulse-ring {
    position: absolute;
    top: 50%;
    left: 30px;
    /* Alineado con el icono */
    transform: translate(-50%, -50%);
    width: 40px;
    height: 40px;
    background: rgba(46, 204, 113, 0.4);
    border-radius: 50%;
    animation: pulse-ring-anim 1.5s cubic-bezier(0.4, 0, 0.6, 1) infinite;
    z-index: -1;
}

.listening-text,
.status-msg {
    color: #bdc3c7;
    margin: 0;
    font-weight: bold;
    font-size: 1rem;
}

.listening-text strong {
    color: white;
    text-transform: capitalize;
    border-bottom: 2px solid #2ecc71;
}

.status-msg {
    color: #7f8c8d;
    letter-spacing: 1px;
}

/* --- FEEDBACK DE PALABRA --- */
.word-preview {
    text-align: center;
    transition: all 0.3s ease;
}

.valid-text {
    color: #2ecc71;
    font-size: 2.2rem;
    font-weight: 900;
    text-transform: uppercase;
    text-shadow: 0 0 15px rgba(46, 204, 113, 0.6);
    border-bottom: 3px solid #2ecc71;
}

.invalid-text {
    color: #e74c3c;
    font-size: 1.8rem;
    font-weight: bold;
    opacity: 0.8;
    text-decoration: line-through;
}

.hint-msg {
    margin-top: 5px;
    color: white;
    font-weight: 900;
    font-size: 0.85rem;
    letter-spacing: 2px;
    animation: flash-hint 0.8s infinite;
}

/* --- ANIMACIONES VUE --- */
@keyframes pulse-ring-anim {
    0% {
        transform: translate(-50%, -50%) scale(0.7);
        opacity: 1;
    }

    100% {
        transform: translate(-50%, -50%) scale(1.5);
        opacity: 0;
    }
}

@keyframes flash-hint {
    50% {
        opacity: 0.3;
    }
}

.fade-down-enter-active,
.fade-down-leave-active {
    transition: all 0.3s ease;
}

.fade-down-enter-from {
    opacity: 0;
    transform: translateY(-20px);
}

.fade-down-leave-to {
    opacity: 0;
    transform: translateY(20px);
}

.pop-in-enter-active {
    animation: pop-word 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275);
}

@keyframes pop-word {
    0% {
        transform: scale(0.5);
        opacity: 0;
    }

    100% {
        transform: scale(1);
        opacity: 1;
    }
}
</style>