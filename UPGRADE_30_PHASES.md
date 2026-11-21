# AJERRY CHANNEL STRIP — Integrated 30-Phase Upgrade

**Branch:** ajerry-integrated-30-phase-upgrade  
**Interface:** Existing GUI/layout preserved. No new visual controls.

## 30 phases
1. Parameter IDs and control-to-engine mapping
2. Parameter ranges and units sanity
3. Host automation/state compatibility
4. Audio input level/peak/RMS analysis
5. Gate threshold behavior
6. Gate envelope/attack/release smoothing
7. Gate attenuation range and noise floor handling
8. SPOTON pitch tracking (fundamental detection)
9. SPOTON voiced/unvoiced/silence discrimination
10. SPOTON key/tonic selection
11. SPOTON scale-aware note targets
12. SPOTON correction strength/speed behavior
13. SPOTON natural vibrato/transient preservation
14. Pitch-shift artifact and latency evaluation
15. EQ high-pass and low-pass stability
16. EQ amount/response mapping
17. EQ parameter smoothing and resonance safety
18. Compressor threshold/ratio mapping
19. Compressor attack/release and gain staging
20. Softening filter and blend behavior
21. Doubler timing and independent wet control
22. Doubler stereo width and mono compatibility
23. Delay time/mix mapping
24. Delay feedback bounds and stability
25. Reverb size/decay mapping
26. Reverb wet/dry and pre-delay behavior
27. Eight-stage signal-chain order and gain consistency
28. Peak protection/clipping prevention and denormal handling
29. Real-time safety, mono/stereo, host-state and CPU checks
30. Build, render/listen tests, DAW loading and final release validation

## Status
- Phase 1 is underway: the GUI's existing 3 knobs per page are being attached to parameters, and additional parameter IDs have been introduced for their mappings.
- The audio engine still requires implementation work: most of these newly added parameters are not yet consumed by DSP, and SPOTON pitch correction is not implemented.
- Compilation, audio render tests, VST3 validation, and AAX validation have not been performed. AAX also requires the Avid SDK and a compatible build setup.
