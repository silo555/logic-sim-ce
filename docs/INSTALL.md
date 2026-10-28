# Install and play

## Requirements

- TI-84 Plus CE or TI-84 Plus CE Python. Monochrome TI-84 Plus and TI-84 Evo
  use different platforms and cannot run this build.
- [TI Connect CE](https://education.ti.com/en/products/computer-software/ti-connect-ce-sw)
  and a USB data cable.
- A working native-program launcher such as
  [Cesium](https://github.com/mateoconlechuga/cesium).
- [CE C libraries v15](https://github.com/CE-Programming/libraries/releases/tag/v15.0),
  including LibLoad, GRAPHX, KEYPADC, and FILEIOC.

The game is native C, not TI-BASIC. A normal PRGM launch may not work on modern
CE OS versions. If you do not already have a working launcher, consult the
[official arTIfiCE compatibility instructions](https://yvantt.github.io/arTIfiCE/)
and Cesium's instructions for your exact model and OS. Launcher compatibility
is separate from testing this game; no OS update or calculator reset is needed
as part of this project's installation instructions.

Back up calculator files before changing your setup.

## Send the game

1. Download `LogicSimCE-v0.1.0-calculator.zip` from the
   [release page](https://github.com/silo555/logic-sim-ce/releases/tag/v0.1.0).
2. Extract it. Locate `LOGICCE.8xp`; do not send the ZIP or the GitHub source ZIP.
3. Connect the calculator and open TI Connect CE's Calculator Explorer.
4. Drag `LOGICCE.8xp` into Explorer, choose **Archive**, and send it.
5. Open Cesium, select **LOGICCE**, and press ENTER.

If libraries are missing, install the official CE C libraries package through
TI Connect CE as directed by its release instructions. Libraries and launchers
are not included in the game download.

## First circuit

The initial board is a half adder. Toggle cell 01 with ENTER. Move down twice
to cell 13 and toggle that switch. The upper output light is sum; the lower
light is carry. Both inputs on should produce sum 0 and carry 1.

Press GRAPH for an empty board or another demo. ENTER on an empty cell opens
the parts palette. Select a source and press Y= to begin a wire. Move to the
receiving part, use WINDOW for A/B, and press ENTER to connect it.

ZOOM opens the board-size menu. Arrows scroll the expanded board. Wires use
the gaps between parts and leave the building cells free.

## Updating

Save your circuit and back up `LGSAVE0` and `LGSAVE1`. Send the new
`LOGICCE.8xp`, replacing the old program when TI Connect CE asks. Keep those
AppVars to preserve the saved circuit. The current build loads old 24-cell
saves; after saving in the new format, use the current build to open them.

## Troubleshooting

| Symptom | What to check |
| --- | --- |
| TI Connect rejects the file | Extract the release ZIP and send the actual `.8xp`. Compare its SHA-256 with the release checksum file. |
| Native program will not launch | Use a working native-program launcher compatible with the calculator OS; check Cesium/arTIfiCE documentation. |
| Missing-library message | Install CE C libraries v15 from the official release. |
| Gate shows `?` | An input may be unconnected. Check both inputs with WINDOW while wiring. |
| Wire connection is rejected | The target needs an input, and the connection must not create a feedback loop. |
| Board refuses to shrink | A part is outside the requested smaller area. Keep the larger size or remove that part. |
| Save fails | Check free RAM/archive and retry. Unrelated AppVars named `LGSAVE0` or `LGSAVE1` are preserved; back them up before managing a name collision. |
| Spaced name is missing from list | Cesium lists the short variable name `LOGICCE`; `logic sim CE` is its selected-program description. |

Hardware execution is still unverified for this preview release. When reporting
a problem, include calculator model, OS, launcher version, steps, and any error.
