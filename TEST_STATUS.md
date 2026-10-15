# Test status

Last local verification: 2026-10-15

## Passed

The portable image-conversion engine was configured, compiled, and tested on the available Linux build host with GCC 14.2 using C++23 and strict warnings:

```text
cmake -S . -B /tmp/r387-core-build -DR387_CORE_TESTS_ONLY=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS='-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror'
cmake --build /tmp/r387-core-build --config Release -j2
ctest --test-dir /tmp/r387-core-build -C Release --output-on-failure
```

Result: **1/1 tests passed**. The suite checks low object limits, a one-object budget, fully transparent input, the bounded unlimited mode, and invalid input.

The `mod.json` file was parsed successfully as JSON and `.github/workflows/build.yml` was parsed as YAML.

## Not tested yet

- The full Geode shared library has not been compiled because this environment does not have the Geode CLI/SDK or Windows toolchain.
- The popup, Geode hooks, object insertion APIs, and Windows GDI+ image loader have not been compiled against the exact target SDK.
- The `.geode` package has not been produced.
- The mod has not been launched in Geometry Dash, and save/undo behavior is not verified.

Treat this as a source project with a tested portable core, not as a ready-to-install mod package. The GitHub Actions workflow is included to run the actual Windows build once the project is pushed to a repository.
