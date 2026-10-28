# Contributing

Bug reports, calculator testing and small improvements are welcome.

For a bug, include the calculator model, OS, launcher version, steps to
reproduce, and expected/actual behavior. Say whether it happened in the desktop
preview, CEmu, or on a physical calculator.

Before opening a code change:

1. Read [development notes](docs/DEVELOPMENT.md).
2. Run `CC=clang sh tools/test.sh`.
3. Add focused regression coverage for behavior that changes.
4. Build with CEdev and run the release verifier if calculator code changes.
5. Keep hardware test claims separate from desktop test results.

Do not include calculator ROMs, downloaded toolchains, personal saves,
credentials, or third-party assets without permission.
