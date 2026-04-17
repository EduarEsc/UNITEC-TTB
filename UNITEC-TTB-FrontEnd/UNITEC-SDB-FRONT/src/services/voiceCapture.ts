let mediaStream: MediaStream | null = null;
let mediaRecorder: MediaRecorder | null = null;
let recordedChunks: BlobPart[] = [];
let currentPlayer: number | null = null;
let isInitialized = false;
let isRecording = false;
let recordingStartedAt = 0;

const API_BASE = 'http://192.168.1.70:8000';

async function ensureStream(): Promise<MediaStream> {
    if (mediaStream) return mediaStream;

    mediaStream = await navigator.mediaDevices.getUserMedia({
        audio: {
            echoCancellation: true,
            noiseSuppression: true,
            autoGainControl: true,
        }
    });

    return mediaStream;
}

function pickMimeType(): string {
    const candidates = [
        'audio/webm;codecs=opus',
        'audio/webm',
        'audio/ogg;codecs=opus',
        'audio/ogg'
    ];

    for (const type of candidates) {
        if (MediaRecorder.isTypeSupported(type)) {
            return type;
        }
    }

    return '';
}

async function blobToAudioBuffer(blob: Blob): Promise<AudioBuffer> {
    const arrayBuffer = await blob.arrayBuffer();
    const audioContext = new AudioContext();
    try {
        return await audioContext.decodeAudioData(arrayBuffer.slice(0));
    } finally {
        await audioContext.close();
    }
}

function floatTo16BitPCM(float32Array: Float32Array): Int16Array {
    const out = new Int16Array(float32Array.length);

    for (let i = 0; i < float32Array.length; i++) {
        const sample = float32Array[i] ?? 0;
        const s = Math.max(-1, Math.min(1, sample));
        out[i] = s < 0 ? s * 0x8000 : s * 0x7FFF;
    }

    return out;
}

function downsampleBuffer(
    buffer: Float32Array,
    inputSampleRate: number,
    outputSampleRate: number
): Float32Array {
    if (outputSampleRate >= inputSampleRate) {
        return buffer;
    }

    const ratio = inputSampleRate / outputSampleRate;
    const newLength = Math.round(buffer.length / ratio);
    const result = new Float32Array(newLength);

    let offsetResult = 0;
    let offsetBuffer = 0;

    while (offsetResult < result.length) {
        const nextOffsetBuffer = Math.round((offsetResult + 1) * ratio);
        let accum = 0;
        let count = 0;

        for (let i = offsetBuffer; i < nextOffsetBuffer && i < buffer.length; i++) {
            accum += buffer[i] ?? 0;
            count++;
        }

        result[offsetResult] = count > 0 ? accum / count : 0;
        offsetResult++;
        offsetBuffer = nextOffsetBuffer;
    }

    return result;
}

function audioBufferToWav(audioBuffer: AudioBuffer, targetSampleRate = 16000): Blob {
    const channelData = audioBuffer.getChannelData(0);
    const monoData = downsampleBuffer(channelData, audioBuffer.sampleRate, targetSampleRate);
    const pcm16 = floatTo16BitPCM(monoData);

    const dataSize = pcm16.length * 2;
    const buffer = new ArrayBuffer(44 + dataSize);
    const view = new DataView(buffer);

    const writeString = (offset: number, str: string) => {
        for (let i = 0; i < str.length; i++) {
            view.setUint8(offset + i, str.charCodeAt(i));
        }
    };

    writeString(0, 'RIFF');
    view.setUint32(4, 36 + dataSize, true);
    writeString(8, 'WAVE');
    writeString(12, 'fmt ');
    view.setUint32(16, 16, true); // Subchunk1Size
    view.setUint16(20, 1, true);  // PCM
    view.setUint16(22, 1, true);  // Mono
    view.setUint32(24, targetSampleRate, true);
    view.setUint32(28, targetSampleRate * 2, true); // ByteRate
    view.setUint16(32, 2, true);  // BlockAlign
    view.setUint16(34, 16, true); // BitsPerSample
    writeString(36, 'data');
    view.setUint32(40, dataSize, true);

    let offset = 44;
    for (let i = 0; i < pcm16.length; i++, offset += 2) {
        view.setInt16(offset, pcm16[i] ?? 0, true);
    }

    return new Blob([buffer], { type: 'audio/wav' });
}

export async function initVoiceCapture(): Promise<boolean> {
    try {
        await ensureStream();
        isInitialized = true;
        console.log('🎙️ Captura de voz inicializada');
        return true;
    } catch (error) {
        console.error('❌ No se pudo inicializar la captura de voz:', error);
        return false;
    }
}

export function isVoiceCaptureRecording(): boolean {
    return isRecording;
}

export async function startVoiceCapture(player: number): Promise<void> {
    if (isRecording) {
        console.warn('⚠️ Ya hay una grabación en curso.');
        return;
    }

    const stream = await ensureStream();
    const mimeType = pickMimeType();

    recordedChunks = [];

    mediaRecorder = mimeType
        ? new MediaRecorder(stream, { mimeType })
        : new MediaRecorder(stream);

    mediaRecorder.ondataavailable = (event: BlobEvent) => {
        if (event.data && event.data.size > 0) {
            recordedChunks.push(event.data);
        }
    };

    recordingStartedAt = Date.now();
    mediaRecorder.start(200);
    isRecording = true;

    console.log(`🎙️ Captura iniciada para jugador ${player}`);
}

export async function stopVoiceCaptureAndUpload(player: number): Promise<void> {
    if (!mediaRecorder || !isRecording) {
        console.warn('⚠️ No hay grabación activa para detener.');
        return;
    }

    const recorder = mediaRecorder;

    const stopPromise = new Promise<Blob>((resolve, reject) => {
        recorder.onstop = async () => {
            try {
                const blob = new Blob(recordedChunks, {
                    type: recorder.mimeType || 'audio/webm'
                });
                resolve(blob);
            } catch (error) {
                reject(error);
            }
        };

        recorder.onerror = (event) => {
            reject(event);
        };
    });

    recorder.stop();
    isRecording = false;

    const durationMs = Date.now() - recordingStartedAt;
    const rawBlob = await stopPromise;

    if (durationMs < 700) {
        console.warn(`⚠️ Grabación demasiado corta (${durationMs} ms), se descarta.`);
        return;
    }

    console.log(`📦 Audio crudo capturado: ${rawBlob.size} bytes | duración aprox: ${durationMs} ms`);

    const audioBuffer = await blobToAudioBuffer(rawBlob);
    const wavBlob = audioBufferToWav(audioBuffer, 16000);

    const formData = new FormData();
    formData.append('player', String(player));
    formData.append('audio_file', wavBlob, `player_${player}.wav`);

    const response = await fetch('http://192.168.1.70:8000/voice/upload', {
        method: 'POST',
        body: formData
    });

    if (!response.ok) {
        const text = await response.text();
        throw new Error(`Error subiendo audio: ${response.status} ${text}`);
    }

    const result = await response.json();
    console.log('✅ Audio subido al backend:', result);
}

export async function releaseVoiceCapture(): Promise<void> {
    if (mediaRecorder && isRecording) {
        try {
            mediaRecorder.stop();
        } catch {
            // noop
        }
    }

    if (mediaStream) {
        mediaStream.getTracks().forEach(track => track.stop());
    }

    mediaRecorder = null;
    mediaStream = null;
    recordedChunks = [];
    currentPlayer = null;
    isInitialized = false;
    isRecording = false;
}