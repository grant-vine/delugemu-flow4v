# FLOW 4V Doom engineering notes

This repository is the public-facing record of an independent engineering
project to bring DoomGeneric to the Behringer FLOW 4V architecture. It contains
the project story, a high-level technical description, licensing information,
and selected GPL-licensed source samples.

It is deliberately not a firmware repository. It contains no Behringer
firmware, extracted proprietary code, WAD data, secrets, vendor packaging
tools, or device-installable update image. Nothing here should be flashed to a
FLOW 4V.

## What was achieved

- The public FLOW 4V update was booted reproducibly in a custom emulator
  without redistributing that update.
- The emulator gained FLOW-specific display, input, storage, timer, reset, and
  watchdog behavior needed by the research target.
- The display investigation converged on an FT811/EVE-style graphics path, and
  DoomGeneric was adapted to the recovered display and control abstractions.
- Doom/Freedoom runs through those emulated hardware paths rather than the
  temporary test-only framebuffer interface used during early bring-up.
- A reproducible, source-reviewable engineering package, vendor packaging
  boundary, rollback plan, and physical validation checklist were prepared for
  Behringer in a separate private submission.

The software path is emulator-verified. We do **not** claim that Doom has booted
or been played on physical FLOW 4V hardware. That claim requires an authorized
Behringer packaging process and evidence from Behringer or another authorized
hardware operator.

## Repository guide

- [`docs/engineering-story.md`](docs/engineering-story.md) describes the major
  hypotheses, corrections, and decisions that moved the work forward.
- [`docs/technical-overview.md`](docs/technical-overview.md) explains the
  architecture and the boundary between verified emulation and pending physical
  validation.
- [`docs/licensing-and-wads.md`](docs/licensing-and-wads.md) covers source
  licensing and the external-WAD policy.
- [`samples/`](samples/) contains selected implementation samples. They are
  illustrative extracts, not a complete build or update system.
- [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) records project origins and
  third-party obligations.

The privacy-safe development visualization is attached to the
[`development-history-2026-08-30`](https://github.com/grant-vine/delugemu-flow4v/releases/tag/development-history-2026-08-30)
release.

## Development scale

The engineering run was audited from local session telemetry:

| Metric | Audited value |
| --- | ---: |
| Non-cached input plus output tokens | **25,427,934** |
| Cached input tokens reused | **975,379,328** |
| Aggregate tokens processed | **1,000,807,262** |
| Cached share of input | 97.689% |
| Wall-clock window | 27 hours, 40 minutes, 51 seconds |

Cached tokens represent reused context rather than newly generated work. The
25.4-million-token figure is the non-cached workload; the one-billion-token
aggregate includes cache reuse.

## License and trademarks

The included source samples are provided under GPL-2.0-or-later; see
[`LICENSE`](LICENSE). Documentation is provided as part of the same project
unless a file says otherwise.

Behringer, FLOW, Doom, DoomGeneric, Freedoom, QEMU, and Deluge are names or
marks of their respective owners. This independent project is not an official
Behringer firmware release and carries no warranty.
