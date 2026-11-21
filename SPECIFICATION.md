# AJERRY CHANNEL STRIP - Compact
## Authoritative final specification

This document records the user's two final submissions. Preserve the specified visual layout, text, control positions, colours, and signal order. Do not redesign, add, remove, or rearrange features.

## Signal flow
Clean → Tune → Shape → Control → Soften → Widen → Space.

The fixed page order is:
1. GATE
2. SPOTON
3. EQ
4. COMP
5. SOFTEN
6. DOUBLER
7. DELAY
8. REVERB

## Audio logic

### 1. GATE
Listen to the room for 1 minute while the user is not singing. Learn noise versus silence and suggest Threshold and Range; the user sets both manually. Close below threshold and open when the voice rises above it. Attack, Hold, and Release are automatic and always on.

### 2. SPOTON
Detect song key and BPM from the beat, lock the key, then tune the vocal to that locked key. Key Lock is ON and AUTO APPLIED. Formant Preservation keeps the voice natural. Amount and Speed start at 0 and are user-adjusted.

### 3. EQ
CLEAN: High-Pass cuts low rumble; Low-Pass cuts high hiss.
SHAPE: four bands: LOW = bass; LOW-MID = muddy body; MID = presence/clarity; HIGH = air. Drag up to boost, down to cut, left/right to change frequency. Gold curve follows the dots.
Learning Meter: GREEN = gentle/natural; YELLOW = careful/listen; RED = too much, bring the dot back to 0. LISTEN suggests, user APPLYs, then shapes manually.

### 4. COMP
One compressor with two serial stages: Stage 1 LA-2A-style gentle leveler; Stage 2 1176-style fast peak catcher. LISTEN suggests Threshold -18 and Ratio 3. Gain Normal +4dB is manual, not automatic makeup. Gain-reduction meter shows actual compression: GREEN good, YELLOW careful, RED too much.

### 5. SOFTEN
After compression, soften harsh S/SH sounds. Modes: Guitar, Vocals, Mix Bus; Vocals selected. Amount 12 o'clock / 50%. Analysis shows harshness. Learning Meter uses GREEN/YELLOW/RED guidance.

### 6. DOUBLER
Widen only after cleaning, tuning, shaping, controlling, and softening. Modes: Verse, Chorus, Rap, Trap, Hook; Chorus selected. Mix is 60% doubled / 40% original. Learning Meter as in SOFTEN.

### 7. DELAY
Echo. Time controls repeat speed: 300ms. Delay amount: 10 percent. Analysis waveform fades from left to right. No meter, LISTEN, or BYPASS.

### 8. REVERB
Room effect, last in chain. Time/Size displays Studio - 300ms. Reverb amount 20% CLEAR VOICE. Analysis shows a long decaying tail, distinct from discrete delay repeats.

## Global interface
- Black background; thin gold outer border.
- Centered gold title: AJERRY CHANNEL STRIP - Compact.
- Eight equal tabs in one row: 1 GATE | 2 SPOTON | 3 EQ | 4 COMP | 5 SOFTEN | 6 DOUBLER | 7 DELAY | 8 REVERB.
- Active tab solid gold with black text; inactive tabs black with thin gold border and gold text.
- Gold round knobs with wavy/gear edge and black marker line.
- Centered gold/light-yellow text; one line of small gold footer text.
- Layout: title and tabs → middle two or three boxes → bottom meter/footer.
- Only one page open at a time.

## Page 1 — GATE
Two equal-height side-by-side boxes.
Left LISTEN box: title LISTEN; large red circular LISTEN button with black text; “Listen to room noise 1 min”; “SUGGESTED THRESHOLD: ---”; “SUGGESTED RANGE: ---”; “Press LISTEN to show suggestion”; “Suggestion Only, You Set It Yourself”; bottom split into “GATE OPEN | GATE CLOSED”.
Right box: THRESHOLD / 0 knob turned left; RANGE / 0 knob at 12 o'clock; centered gold-brown BYPASS button.
Footer: “Step 1 of 8 - Only one opens at a time, Inbuilt: Attack Hold Release AUTO - Always ON”.

## Page 2 — SPOTON
Two boxes.
Left BEAT KEY DETECTOR: red circular LISTEN; “Key: A Minor”; “Key Lock ON”; “BPM: 110”; “Confidence: 95%”; “AUTO APPLIED”; blue “AUTO DETECT KEY - 1 MIN” button; green “LOCK THIS KEY” button; bottom “DETECTED: ---”.
Right VOCAL AUTO-TUNE: piano keyboard at top, white keys with some green; AMOUNT / 0 knob turned left; SPEED / 0 knob at 12 o'clock; dropdown “KEY: C” with down arrow; BYPASS button.
Footer: “AJERRY - Formant Preservation - Smooth Correction - Better Pitch Detection - Always ON”.

## Page 3 — EQ
Subheader: “AJERRY EQ PRO - CLEAN + SHAPE”.
Three top boxes:
- LISTEN: large red circular LISTEN button; “Listen to your vocal 1 min”; “SUGGESTED HIGH-PASS: ---”; “SUGGESTED LOW-PASS: ---”; “Press LISTEN to show suggestion”; “Suggestion Only, You Set It Yourself”; green “APPLY SUGGESTION” at bottom of same box.
- HIGH-PASS / LOW-PASS: HIGH-PASS / 80Hz and LOW-PASS / 16kHz knobs, both at 12 o'clock; centered gold-brown BYPASS.
- ANALYSIS - EQ CURVE: blue rounded graph body. Horizontal grid labels +12 / +6 / 0 / -6 / -12. Vertical labels 20Hz / 100Hz / 1kHz / 10kHz / 20kHz. Gold curve. Four dots inside graph only: LOW cyan at 120Hz, 0dB (Low Shelf/bass); LOW-MID green at 400Hz, 0dB (Peak/muddy body); MID orange at 1000Hz, 0dB (Peak/presence clarity); HIGH purple at 5000Hz, 0dB (High Shelf/air brightness). Drag up = boost, down = cut, left/right = frequency; curve follows dots. Labels only below graph, inside same box, one row aligned under matching dots: LOW cyan, LOW-MID green, MID orange, HIGH purple. Do not put band names inside graph.
Below top boxes, two stacked boxes:
1. “SUGGESTED HIGH-PASS: 80Hz / SUGGESTED LOW-PASS: 16kHz”.
2. “UPDATED AJERRY LEARNING METER”; numbers 0 -3 -6 -12 -18; green/yellow/red bar; white downward triangle on green near 0 to -3; “GREEN GOOD / YELLOW CAREFUL / RED TOO MUCH”; explainer: “GREEN = Gentle natural keep arrow here, YELLOW = Careful listen, RED = Too much bring dot back to 0”.
Footer: “AJERRY EQ PRO - HP cuts rumble, LP cuts hiss - Drag dots to shape tone - Listen 1 min - Press APPLY”.

## Page 4 — COMP
Subheader: “AJERRY COMP PRO - SERIAL INSIDE - NORMAL GAIN”.
Three top boxes:
- THRESHOLD: gold knob at 12 o'clock; value -18.
- RATIO: small red LISTEN pill overlapping top edge; gold knob at 12 o'clock; value 3; green inset at bottom containing “Stage 1 GENTLE LA-2A -> Stage 2 CATCHER 1176” and “SERIAL INSIDE - ONE COMPRESSOR DOES BOTH”.
- GAIN NORMAL: gold knob at 12 o'clock; +4dB; “GAIN NORMAL - YOU SET IT YOURSELF”.
Below: box “SUGGESTED THRESHOLD: -18 / SUGGESTED RATIO: 3”; then GAIN REDUCTION METER with green/yellow/red bar, white downward triangle on yellow/green border, and “GREEN GOOD / YELLOW CAREFUL / RED TOO MUCH”. No numbers on this meter.
Footer: “AJERRY COMP PRO - SERIAL INSIDE - NORMAL GAIN - Listen 1 min - Attack Auto Release Auto”.

## Page 5 — SOFTEN
Three top boxes:
- MODE: stacked Guitar / Vocals / Mix Bus buttons; Vocals selected, solid blue with white text.
- AMOUNT: gold knob at 12 o'clock; “12 O'CLOCK - 50%”.
- ANALYSIS - HARSHNESS / ESSES: small rounded blue waveform box, tall in middle and smaller at sides.
Bottom LEARNING METER box: “AJERRY LEARNING METER”; numbers 0 -3 -6 -12 -18; green/yellow/red bar; white downward triangle near yellow/green border; “GREEN GOOD / YELLOW CAREFUL / RED TOO MUCH”.
Footer: “AJERRY LEARNING METER - PRESS LISTEN - Listen 1 min - Sing S and SH words - PRESS STOP”.

## Page 6 — DOUBLER
Three top boxes, same structure as SOFTEN:
- MODE: stacked Verse / Chorus / Rap / Trap / Hook buttons; Chorus selected solid blue. This box is taller than SOFTEN's MODE box.
- AMOUNT: gold knob at 12 o'clock; “MIX 60% 12 o'clock”.
- ANALYSIS - DOUBLED VOICE / STEREO: same blue waveform box as SOFTEN.
Bottom meter identical to SOFTEN, including numbers, arrow, and labels.
Footer identical to SOFTEN.

## Page 7 — DELAY
Three equal tall boxes; no bottom meter, LISTEN, or BYPASS.
- TIME / SPEED: gold knob at 12 o'clock; “300ms - 12 o'clock”.
- DELAY / ECHO: gold knob at 12 o'clock; “10 percent”.
- ANALYSIS: blue waveform starts tall on left and gets smaller to right (fading echo).
Footer: “AJERRY SIMPLE DELAY - TIME controls speed - DELAY controls echo amount”.

## Page 8 — REVERB
Three equal tall boxes, same layout as DELAY.
- TIME / SIZE: gold knob turned slightly right of top; “Studio - 300ms”.
- REVERB / ROOM: gold knob turned slightly left; “20% CLEAR VOICE”.
- ANALYSIS: blue waveform starts tall then has a long decaying tail, different from DELAY.
Footer: “AJERRY IMPROVED REVERB - TIME shows room name - REVERB keeps voice clear”.

## Three rules
1. LISTEN suggests; user decides. Only SPOTON Key Lock is forced.
2. GREEN GOOD, YELLOW CAREFUL, RED TOO MUCH.
3. Only one page opens at a time.

## Build targets and release requirements
Target formats: VST3 and AAX. AAX must be genuinely compiled with an authorized Avid AAX SDK and a compatible build environment; an upload-artifact step alone is not an AAX build. Do not label either format complete until compilation and artifact verification succeed.
