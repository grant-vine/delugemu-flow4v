# Technical overview

The project has two related products:

1. a FLOW 4V machine model used to execute and inspect software without
   physical hardware; and
2. a DoomGeneric platform adaptation that targets the recovered display,
   controls, storage, timing, and lifecycle abstractions.

## Architecture

```text
            DoomGeneric engine + external WAD
                          |
                  FLOW platform API
             +------------+------------+
             |            |            |
          display        input      storage/time
             |            |            |
       FT811/EVE model  GPIO/SPI     RZ/A1 devices
             +------------+------------+
                          |
                 FLOW 4V machine model
                          |
                    QEMU ARM system
```

The platform interface keeps DoomGeneric independent of the temporary
emulator-only bring-up interface. Rendering converts Doom's indexed frame into
the display representation expected by the recovered graphics path. Input is
translated into Doom actions without hiding unresolved hardware identities.
WAD access is read-only and supplied by an external media implementation.

## Evidence levels

### Verified in the emulator

- repeatable boot of the locally supplied public FLOW update;
- FT811/EVE command and framebuffer behavior covered by synthetic fixtures;
- DoomGeneric display, input, storage, timing, exit, and restart paths;
- failure cases for missing or invalid external WAD data;
- packaging-boundary rejection tests that do not invoke a device writer.

### Reserved for private Behringer review

- the complete FLOW machine model and payload source;
- detailed register, startup, memory, and firmware contracts;
- vendor packager/uploader wrapper interfaces;
- release, rollback, and device evidence procedures;
- hardware observations that are useful to the submission but not necessary
  for the public case study.

### Not yet proven

- the final authorized update container;
- physical installation and readback;
- display and control behavior on a real FLOW 4V;
- recovery and rollback on the exact production hardware revision.

Those final items can only be completed by Behringer or an authorized operator
with the supported update and recovery process. The public samples must not be
treated as a flashable firmware package.

## Source samples

The selected files under [`samples/`](../samples/) show the architectural seams
without reproducing the entire submission:

- the DoomGeneric platform contract;
- Doom's platform-side frame/input integration;
- read-only external-WAD adaptation; and
- the FT811 display-list renderer used by the emulator.

They retain their GPL headers and original identifiers so that technical review
and attribution remain straightforward.
