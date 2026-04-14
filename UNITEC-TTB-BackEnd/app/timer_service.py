import asyncio
import time

class TimerService:
    def __init__(self, callback_timeout):
        self.callback = callback_timeout
        self.task = None
        self.start_time = None
        self.remaining_time = 0
        self.is_paused = False

    async def start(self, seconds=10):
        """Inicia el temporizador desde cero."""
        self.stop()
        self.remaining_time = seconds
        self.is_paused = False
        self.task = asyncio.create_task(self._run())
        print(f"⏱️ Temporizador de {seconds}s iniciado.")

    def pause(self):
        """Pausa el temporizador y guarda el tiempo restante."""
        if self.task and not self.is_paused:
            # Calculamos cuánto tiempo ha pasado desde que inició el _run
            elapsed = time.time() - self.start_time
            self.remaining_time -= elapsed
            if self.remaining_time < 0: self.remaining_time = 0
            
            self.task.cancel()
            self.is_paused = True
            print(f"⏸️ Temporizador pausado. Quedan: {self.remaining_time:.2f}s")

    def resume(self):
        """Reanuda el temporizador con el tiempo que quedaba."""
        if self.is_paused and self.remaining_time > 0:
            self.is_paused = False
            self.task = asyncio.create_task(self._run())
            print(f"▶️ Temporizador reanudado con {self.remaining_time:.2f}s")

    async def _run(self):
        self.start_time = time.time()
        try:
            await asyncio.sleep(self.remaining_time)
            # Si llegamos aquí, el tiempo se agotó de verdad
            print("💥 ¡TIEMPO AGOTADO!")
            if self.callback:
                await self.callback()
        except asyncio.CancelledError:
            # Esto ocurre al pausar o detener
            pass

    def stop(self):
        """Detiene y limpia el temporizador por completo."""
        if self.task:
            self.task.cancel()
            self.task = None
        self.remaining_time = 0
        self.is_paused = False
        self.start_time = None
        print("⏱️ Temporizador reseteado/detenido.")