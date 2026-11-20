# AJERRY CHANNEL STRIP – Compact

Eight-module JUCE plugin prototype: Gate, SPOTON (pitch/key UI scaffold), EQ, Compressor, Soften, Doubler, Delay, Reverb.

## Status
This is an active prototype. Gate, EQ, compressor, soften, doubler, delay and reverb DSP prototypes are present in the code; SPOTON pitch correction/key detection is not implemented as a production algorithm yet. The interface is not yet fully wired to every parameter. Not a finished commercial release.

## Windows VST3 build
1. Install Visual Studio 2022 with C++ Desktop tools, CMake and Git.
2. Clone this repository.
3. Clone JUCE 8.0.8 into the repository root as `JUCE`.
4. Run: `cmake -B build -DCMAKE_BUILD_TYPE=Release` then `cmake --build build --config Release --parallel`.
5. The VST3 appears under `build/AJERRY_artefacts/Release/VST3`.

GitHub Actions is configured to build Windows VST3 and upload an artifact when successful.

## AAX
AAX requires the Avid AAX SDK and JUCE AAX support. The SDK is not stored in this repository. AAX is not yet configured or built.
