$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root 'build-core-tests'
cmake -S $root -B $build -DR387_CORE_TESTS_ONLY=ON
cmake --build $build --config Release
& (Join-Path $build 'R387CoreTests.exe')
