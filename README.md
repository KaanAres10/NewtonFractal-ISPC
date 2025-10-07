# Newton Fractal (ISPC)

An **interactive Newton Fractal** accelerated with [ISPC (Intel Implicit SPMD Program Compiler)](https://ispc.github.io/)
---

## Features
- Fast fractal rendering using ISPC (SPMD-on-SIMD).
- Adjustable polynomial degree *n* and iteration count.
---

## Running the Prebuilt Binary
If you just want to try it out:
1. Download the `NewtonFractal.zip` from the [Releases](../../releases) page.
2. Extract it anywhere.
3. Run `newton.exe`.

**Note**:  
- The provided build is compiled for **modern CPUs with AVX2 support**.  
- On older CPUs without AVX2, you may need to rebuild with a lower ISPC target (change of CMakeLists.txt `sse2-i32x4`).

---

## Building from Source

### Prerequisites
- CMake 3.16+
- C++17 compiler (Visual Studio 2019/2022 on Windows, GCC/Clang on Linux/macOS)
- [ISPC](https://ispc.github.io/downloads.html)

### Build Steps
```bash
# Configure (Release mode)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
