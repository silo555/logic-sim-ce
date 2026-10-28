# Development

## Calculator build

Install [CEdev v15.0](https://github.com/CE-Programming/toolchain/releases/tag/v15.0)
for your computer. Put its `bin` directory on PATH, then run:

```sh
make
```

Output: `bin/LOGICCE.8xp`. Build products are ignored by Git. The Makefile
sets the variable name, Cesium description, and icon. Launching the result
requires the external runtime libraries and launcher described in the install guide.

Regenerate the 16×16 icon from its pixel source with:

```sh
python3 tools/make_icon.py
```

## Host tests

A C compiler with AddressSanitizer and UndefinedBehaviorSanitizer is required.
The verified setup uses Clang on macOS; CI runs the same tests on Linux.

```sh
CC=clang sh tools/test.sh
```

The script builds in a temporary directory and removes its test executable
on exit. Tests use the real simulation/UI/renderer and a mock TI OS for storage.
They do not emulate eZ80 hardware or the TI OS.

Check a built calculator file before publishing:

```sh
python3 tools/verify_release.py bin/LOGICCE.8xp
```

This checks the TI variable envelope and checksum, program type/name,
Cesium icon header, all 256 icon bytes, and the exact spaced description.

## Desktop preview

```sh
python3 preview/run.py
# Optional custom port:
python3 preview/run.py 8765
```

Requires Python 3 and `cc` (or set `CC=clang`). No pip packages are needed.
macOS and Linux are the intended host environments; native Windows is untested.
The server binds to loopback only. The preview saves to `preview/circuit.raw`,
which is ignored by Git and is separate from calculator AppVars.

## Source layout

| Path | Responsibility |
| --- | --- |
| `src/logic.c` | Parts, wiring, cycle checks, evaluation, resizing, demos |
| `src/app.c` | Controls, menus, undo, camera |
| `src/render.c` | Shared 320×240 pixel rendering and font |
| `src/save.c` | Versioned encoding, CRC, legacy migration |
| `src/main.c` | CE keypad, graphx and fileioc adapters, alternating saves |
| `tests/test.c` | Engine/UI/render/storage regression tests |
| `preview/` | Browser controls and desktop backend for the same C code |
| `tools/` | Test runner, icon generator and release validation |

## Design limits

The board supports at most 12 columns × 8 rows (96 parts). A part has up to
two input references. Wires are connections, not occupied grid cells.
Resizing remaps indices by column/row and refuses to discard occupied cells.
Evaluation uses low/high/unknown signals; a known 0 dominates AND and a known
1 dominates OR. The graph is combinational: wiring that introduces a cycle is rejected.

The save codec writes `LGCE02` records (398 bytes), and accepts legacy
`LGCE01` records (108 bytes). The calculator alternates `LGSAVE0`/`LGSAVE1`,
verifies writes, and archives them. Undo holds 16 in-memory board snapshots.

## Publishing a release

1. Build with the documented toolchain; run host tests and the binary verifier.
2. Update the changelog and testing status. Report hardware results separately.
3. Package `LOGICCE.8xp`, README/install instructions, LICENSE and verification
   notes in a calculator ZIP. Keep external dependencies as official links.
4. Upload the ZIP, raw `.8xp`, and SHA-256 checksums to a GitHub release.
5. Use a prerelease until physical calculator testing is confirmed.

CI validates host logic, source assets, and desktop-backend compilation. It
currently does not download CEdev or produce a calculator build.
