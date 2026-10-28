# Testing and known limitations

## v0.1.0 preview status

**Verified locally:**

- CEdev v15.0 native build succeeds. The toolchain linker emits a GNU-stack
  metadata warning; the C sources compile with warnings treated as errors.
- Clang AddressSanitizer/UndefinedBehaviorSanitizer tests pass for all gate
  truth tables including unknown inputs, all demo input combinations,
  95-way fanout, deep chains, cycle rejection, and 10,000 mixed edits.
- Tests cover the actual UI, both input pins, deletion/disconnection, undo,
  every board size, safe resizing and connection remapping, all 96 camera
  positions, offscreen wires and drawing bounds.
- Storage tests cover CRC corruption, bad lengths, invalid graphs, failed and
  interrupted writes, alternate-slot recovery, name collisions, generation
  wraparound and legacy-save migration. TI OS storage calls are mocked.
- Desktop browser play checks cover toggling the half adder, board expansion,
  scrolling to cell 96 and back, placing a part along an existing wire without
  disrupting outputs, undo, save, and the new title/icon.
- The packaged `.8xp` passes TI header/length/checksum checks. Its Cesium icon
  and spaced description match the generated metadata byte for byte.

**Not yet verified:** physical calculator execution, CEmu execution, hardware
keypad behavior, real archive/garbage-collection behavior, and calculator performance.
The desktop preview is not a substitute for those tests.

## Current scope

- Combinational circuits only. No clocks, latches, flip-flops or feedback loops.
- Maximum 96 parts, two inputs per part, one saved user circuit.
- No drag/move tool; delete and replace parts as needed.
- Wire crossings are visual crossings, not electrical junctions.
- Undo is session-only. Save format upgrades are forward-readable by the new
  version; older programs cannot read expanded saves.
- Dependencies and launcher setup are external to this project.

## Helpful calculator test report

Please report model, OS version, launcher version, runtime library version,
what you tried, expected/actual output, and a photo if useful. Start with:

1. Launch the game and toggle all half-adder input combinations.
2. Create two switches, a gate and a light; connect both gate inputs.
3. Expand to 12×8, place something near cell 96, and scroll back.
4. Save, exit and relaunch. Verify size, parts and connections survived.
5. Check undo, changing parts, disconnection, deletion and quitting.

Do not describe a test as hardware-tested unless it was run on that hardware.
