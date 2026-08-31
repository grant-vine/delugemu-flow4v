# Engineering story

This is the public, high-level account of the FLOW 4V Doom investigation. The
private Behringer submission contains the detailed hardware contracts,
packaging procedures, validation evidence templates, and complete buildable
source tree.

## 1. Start from a working emulator

The work began as a fork of DelugEmu, which already modeled the Renesas RZ/A1
family used by the original project. That supplied a realistic CPU and
peripheral foundation while keeping FLOW-specific behavior in separate device
models.

## 2. Establish reproducible firmware boot

The first milestone was repeatable execution of the public FLOW 4V update in
the emulator using local, excluded inputs. Loader placement, entry behavior,
memory aliases, and early peripheral accesses were captured as reproducible
tests rather than informal observations.

## 3. Use a temporary interface only for bring-up

An emulator-only framebuffer and control interface allowed the DoomGeneric port
to run early. It was useful scaffolding, but it could not demonstrate that the
real device paths were understood. The project therefore treated it as a test
aid, moved it away from suspected hardware ranges, and made removal from the
production path an explicit goal.

## 4. Iterate with competing hardware hypotheses

Firmware traces, public processor documentation, peripheral access patterns,
and small regression fixtures were used in a repeated loop: state several
hypotheses, predict observable behavior, run the emulator, reject weak models,
and create the next hypotheses from the evidence. One early display-region
interpretation was wrong; checking the processor manual corrected it before it
became a permanent API.

## 5. Identify the graphics-controller path

The display investigation eventually converged on FT811/EVE-style commands and
memory structures. A focused emulator participant and synthetic command-list
tests made the interpretation repeatable. The Doom port could then present
frames through the recovered display abstraction instead of the fake
framebuffer contract.

## 6. Reuse public driver knowledge

A decisive user suggestion was to look for existing driver libraries once the
controller family was known. Public Bridgetek/EVE documentation and established
driver conventions provided a much faster way to check command semantics than
continuing to infer every field independently. The implementation still kept
only behavior supported by traces and regression fixtures.

## 7. Recover controls and supporting peripherals

The same evidence-driven method was applied to GPIO/SPI input behavior,
rotaries, buttons, storage, timers, reset, and watchdog handling. Ambiguous
control bits stayed explicitly unresolved instead of being assigned convenient
but unproven meanings.

## 8. Adapt DoomGeneric and external WAD loading

The DoomGeneric platform layer was separated from the underlying hardware
implementation. Rendering, input, timing, exit behavior, and WAD reads were
routed through a small platform contract. No commercial Doom data or Freedoom
WAD is stored in Git; WADs remain explicit external inputs with provenance and
hash checks.

## 9. Define the real-device takeover boundary

Emulator success is not sufficient for a safe device update. The private
submission therefore records startup state, memory ownership, interrupt and
cache expectations, watchdog behavior, rollback requirements, and the vendor
hooks needed to wrap the payload using Behringer's supported update process.

## 10. Package for review without proprietary material

The review package is reproducible from source and excludes the public
Behringer download, locally recovered executables, proprietary update images,
private packagers, secrets, and WAD files. Behringer can reproduce the final
device image inside its authorized environment while reviewing the complete
engineering source and tests.

## 11. Separate public history from the vendor submission

The complete engineering tree and detailed handoff belong in the private
Behringer repository. This lighter public repository preserves the story,
licensing position, and representative GPL source samples without publishing
vendor-specific packaging detail or claiming unverified physical success.
