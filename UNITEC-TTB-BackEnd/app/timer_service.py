import asyncio
import time
from typing import Optional, Callable, Awaitable


class TimerService:
    def __init__(self, callback_timeout: Optional[Callable[[], Awaitable[None]]] = None):
        self.callback = callback_timeout

        self.task: Optional[asyncio.Task] = None
        self.duration = 0.0
        self.remaining_time = 0.0

        self.started_at: Optional[float] = None
        self.state = "idle"   # idle | running | paused | finished

        self.tick_interval = 0.05
        self.session_id = 0   # invalida tareas viejas

    def get_remaining_time(self) -> float:
        if self.state == "running" and self.started_at is not None:
            elapsed = time.monotonic() - self.started_at
            return max(0.0, self.remaining_time - elapsed)

        return max(0.0, self.remaining_time)

    def is_running(self) -> bool:
        return self.state == "running"

    def is_paused(self) -> bool:
        return self.state == "paused"

    async def start(self, seconds: float = 10.0):
        self.stop()

        self.session_id += 1
        current_session = self.session_id

        self.duration = float(seconds)
        self.remaining_time = float(seconds)
        self.started_at = time.monotonic()
        self.state = "running"

        self.task = asyncio.create_task(self._run(current_session))
        print(f"⏱️ Temporizador de {seconds:.2f}s iniciado.")

    def pause(self):
        if self.state != "running":
            return

        self.remaining_time = self.get_remaining_time()
        self.started_at = None
        self.state = "paused"

        if self.task:
            self.task.cancel()
            self.task = None

        print(f"⏸️ Temporizador pausado. Quedan: {self.remaining_time:.2f}s")

    def resume(self):
        if self.state != "paused":
            return

        if self.remaining_time <= 0:
            self.state = "finished"
            return

        self.session_id += 1
        current_session = self.session_id

        self.started_at = time.monotonic()
        self.state = "running"
        self.task = asyncio.create_task(self._run(current_session))

        print(f"▶️ Temporizador reanudado con {self.remaining_time:.2f}s")

    def stop(self):
        if self.state == "idle" and not self.task:
            return

        self.session_id += 1  # invalida cualquier tarea anterior

        if self.task:
            self.task.cancel()
            self.task = None

        self.duration = 0.0
        self.remaining_time = 0.0
        self.started_at = None
        self.state = "idle"

        print("⏱️ Temporizador reseteado/detenido.")

    async def _run(self, run_session_id: int):
        try:
            while self.state == "running":
                if run_session_id != self.session_id:
                    return

                current_remaining = self.get_remaining_time()

                if current_remaining <= 0:
                    if run_session_id != self.session_id:
                        return

                    self.remaining_time = 0.0
                    self.started_at = None
                    self.state = "finished"
                    self.task = None

                    print("💥 ¡TIEMPO AGOTADO!")
                    if self.callback:
                        await self.callback()
                    return

                await asyncio.sleep(self.tick_interval)

        except asyncio.CancelledError:
            pass