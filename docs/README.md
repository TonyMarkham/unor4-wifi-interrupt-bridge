# Docs

**To build and flash the stock firmware, read only
[build-original-procedure.md](build-original-procedure.md), from top to
bottom.** Everything you need is in it.

**To build and flash the interrupt fork (UDP push mode),** follow the
stock procedure once first, then
[build-interrupt-procedure.md](build-interrupt-procedure.md), then
[flash-interrupt-procedure.md](flash-interrupt-procedure.md).

| File | Read it when |
|---|---|
| [ROADMAP.md](ROADMAP.md) | The plan for this fork: phases, status, decisions. |
| [build-original-procedure.md](build-original-procedure.md) | **Start here.** The step-by-step build and flash procedure for the stock firmware. |
| [build-original-reference.md](build-original-reference.md) | Optional. Only if you want to know *why* a step is the way it is. The procedure never requires it. |
| [build-interrupt-procedure.md](build-interrupt-procedure.md) | Building the interrupt fork's image. Assumes the stock procedure was done. |
| [flash-interrupt-procedure.md](flash-interrupt-procedure.md) | Flashing the interrupt fork and testing push mode with [`examples/UDPPushSleep`](../examples/UDPPushSleep/UDPPushSleep.ino); also how to go back to stock. |
| [releases/](releases/) | The text of each GitHub release: [`v0.6.0-stock`](releases/v0.6.0-stock.md) and [`v0.6.0-interrupt.1`](releases/v0.6.0-interrupt.1.md). |
