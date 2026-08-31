# Licensing and WAD handling

## Source

The selected implementation samples are GPL-2.0-or-later and retain SPDX
headers. The project originated as a fork of the GPL-licensed DelugEmu codebase
and uses QEMU-style device modeling conventions. See [`LICENSE`](../LICENSE)
and [`THIRD_PARTY_NOTICES.md`](../THIRD_PARTY_NOTICES.md).

## Doom engine and game data

DoomGeneric is an engine portability project; it does not grant rights to
commercial Doom game data. Freedoom provides freely licensed replacement game
data, but its WAD files and assets are not stored in this repository.

Users and vendor reviewers must supply WAD data separately and comply with its
license. The engineering submission validates the expected external file by
name, size, provenance, and hash before packaging or testing it.

## Behringer firmware

The fact that an official update can be downloaded publicly does not by itself
grant permission to redistribute its contents or derived proprietary images.
This repository therefore contains no Behringer firmware download, extracted
payload, encryption or wrapping material, or device-installable update.

The private submission is designed so Behringer can combine the independently
licensed Doom payload with its authorized update process inside its own
environment without publishing those protected inputs.
