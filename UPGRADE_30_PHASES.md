# AJERRY CHANNEL STRIP — 30-phase integrated upgrade

**Interface rule:** Preserve the existing 8 tabs, controls, dimensions, graphics and layout. No new GUI controls.
**Verification rule:** Mark code work done only after source changes; mark build/audio/DAW checks done only after evidence.

## Phase tracking

1. [x] Review current processor/editor source and existing parameter IDs.
2. [x] Add attachment support for the existing three knobs.
3. [x] Map all eight tabs to distinct stage parameter IDs.
4. [x] Add gate threshold, range, attack and release parameters.
5. [x] Implement a sample-wise gate envelope.
6. [ ] Add gate gain smoothing/hysteresis and noise-tail testing.
7. [x] Add SPOTON key control as an integer semitone key.
8. x] Implement block autocorrelation pitch detection.
9. [x] Calculate key-relative major-scale target and correction cents.
10. [ ] Implement audible pitch-shift/correction rendering (not yet present).
11. [ ] Implement correction-speed smoothing and preserve vibrato.
12. [ ] Add formant preservation and artifact reduction; evaluate with vocals.
13. [x] Connect high-pass and low-pass filters to controls.
14. [x] Connect EQ gain control to audio gain stage.
15. [ ] Implement a musical EQ curve/tilt and smooth coefficient transitions.
16. [x] Connect compressor threshold, ratio and output gain.
17. [ ] Implement adjustable attack/release, knee and gain reduction meter.
18. [ ] Add level compensation and transient/listening validation.
19. [x] Implement adjustable soften cutoff.
20. [x] Blend softened signal using amount and mix controls.
21. x] Implement short delay-based doubling.
22. [x] Connect doubler amount, width and delay.
23. [x] Implement delay time, wet mix and feedback.
24. [ ] Add delay parameter smoothing, tempo-sync option and feedback safety tests.
25. [x] Connect reverb size, mix and width.
26. [ ] Validate reverb decay, density, stereo image and dry vocal clarity.
27. [ ] Integrate all eight stages and optimize gain staging/clipping prevention.
28. [ ] Add parameter smoothing, mono/stereo edge cases, CPU and state-restore tests.
29. [ ] Build and verify the VST3 in CI; publish artifact only after successful build.
30. [ ] Load in a DAW and test live audio, automation, latency, presets and real vocal material.

## Current source work
- Existing three-knob layout retained.
- Knobs now attach to parameters on tab changes.
- Processor parameter model expanded to dedicated controls per stage.
- Sample-wise gate, EQ filters/gain, compressor, soften blend, doubler delay, feedback delay and reverb are implemented in source.
- SPOTON currently detects pitch and computes key-relative target/correction telemetry; **audible pitch-shifting renderer is not implemented**.
- Build and DAW validation remain unverified.
