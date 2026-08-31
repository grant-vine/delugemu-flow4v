# Source samples

These files are selected extracts from the complete private engineering tree.
They illustrate the core seams of the work while keeping the public repository
small.

| File | Purpose |
| --- | --- |
| [`doomgeneric/flow4v_platform.h`](doomgeneric/flow4v_platform.h) | Display, input, timing, lifecycle, and external-WAD platform contract. |
| [`doomgeneric/doomgeneric_adapter.c`](doomgeneric/doomgeneric_adapter.c) | Public frame, timing, and edge-triggered input adapter excerpt. |
| [`doomgeneric/flow4v_wad.c`](doomgeneric/flow4v_wad.c) | Read-only WAD adapter over the platform media contract. |
| [`emulation/flow4v_ft811.c`](emulation/flow4v_ft811.c) | FT811/EVE display-list interpreter used by the emulator model. |

The extracts preserve their GPL-2.0-or-later headers. The public adapter removes
private lifecycle and memory-placement details while retaining the platform
design. These files are not a standalone build: the complete QEMU machine,
DoomGeneric source, startup code, linker layouts, tests, and vendor packaging
boundary are intentionally part of the private Behringer submission.

Do not infer a device update format from these samples and do not flash them to
hardware.
