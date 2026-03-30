# DustCrate

**A digital crate digger for your MPC workflow.**

DustCrate is a source instrument — not an MPC clone. Browse and play vintage sounds (dusty loops, one-shots, textures, melodic hits), then sample the output directly into your MPC or DAW via audio cable or Bluetooth. Think of it like flipping through dusty crates at a record shop, but on your phone or desktop.

---

## What It Does

- Browse curated packs of CC0/public domain vintage sounds
- Trigger samples chromatically via on-screen keyboard
- Shape sound with per-voice ADSR + LP/HP filter
- Run as VST3/AU plugin inside any DAW
- Run as a standalone Android APK on your phone
- Sample the output into your MPC via audio cable or Bluetooth

---

## Architecture

```
DustCrate/
├── Source/
│   ├── PluginProcessor.cpp     ← audio engine, sample playback
│   ├── PluginProcessor.h
│   ├── PluginEditor.cpp        ← UI (browser, keyboard, ADSR)
│   ├── PluginEditor.h
│   ├── SampleLibrary.cpp       ← sample metadata + streaming
│   ├── SampleLibrary.h
│   ├── SampleVoice.cpp         ← polyphonic voice (ADSR, pitch, filter)
│   └── SampleVoice.h
├── assets/
│   └── samples/                ← bundled CC0 sounds (FLAC/OGG)
├── packs/
│   └── factory_pack.json       ← sample metadata manifest
├── CMakeLists.txt              ← JUCE CMake build
├── DustCrate.jucer             ← Projucer config (optional)
└── README.md
```

---

## Build Targets

| Target | Format | Notes |
|--------|--------|-------|
| macOS | VST3 + AU + Standalone | Xcode via CMake |
| Windows | VST3 + Standalone | Visual Studio / MSVC |
| Linux | VST3 + Standalone | GCC / Clang |
| Android | Standalone APK | Android NDK + Oboe |

---

## Build Instructions

### Desktop (CMake)

```bash
git clone https://github.com/ncsound919/dustcrate.git
cd dustcrate
git submodule update --init --recursive   # pulls JUCE
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Android

1. Install Android Studio + NDK (r25c recommended)
2. Open `android/` folder in Android Studio
3. Set `JUCE_DIR` in `CMakeLists.txt`
4. Build → Run on device

> Keep buffer size at 128–256 samples. Stream samples from disk — do not bulk-load into RAM.

---

## Sound Sourcing

All bundled sounds are CC0 or public domain, curated from:

| Source | What's There |
|--------|--------------|
| [Freesound.org](https://freesound.org) | CC0 field recordings, vinyl crackle, old instruments |
| [Internet Archive](https://archive.org) | 78rpm records, old radio broadcasts |
| [NASA Audio](https://www.nasa.gov/audio-and-ringtones/) | Space transmissions, launches — weird textures |
| [Philharmonia Samples](https://philharmonia.co.uk/resources/sound-samples/) | Free orchestral one-shots |
| [VM-PC Sample Library](https://vm-pc.com) | Vintage drum machines, public domain |

### Pack Categories
- `drums` — kicks, snares, hats, vintage drum machine hits
- `melodic` — piano, strings, horn stabs, chromatic one-shots
- `textures` — pads, drones, shortwave radio, room tones
- `vocal` — chops, shouts, old broadcast voices
- `noise` — vinyl crackle, tape hiss, static, warble

---

## Sample Engine Specs

- **Polyphony:** 8–16 voices
- **Pitch mapping:** chromatic playback from root note
- **Envelope:** per-voice ADSR
- **Filter:** LP/HP with cutoff + resonance
- **Format support:** FLAC, OGG, WAV (via JUCE AudioFormatManager)
- **Streaming:** disk-based streaming via AudioFormatReaderSource

---

## UI Layout

```
┌─────────────────────────────────────┐
│  [CATEGORY ▾]   [PACK ▾]   🔍       │
├─────────────────────────────────────┤
│  ▶ dusty_strings_c3.flac            │
│  ▶ vinyl_crackle_loop.flac          │
│  ▶ old_piano_broken_d4.flac  ←─ sel │
│  ▶ shortwave_texture_01.flac        │
├─────────────────────────────────────┤
│  [   chromatic keyboard trigger   ] │
│  ADSR  ·  LP/HP Filter  ·  Pitch    │
└─────────────────────────────────────┘
```

---

## Build Roadmap

- [x] Repo scaffold + architecture plan
- [ ] CMakeLists.txt with JUCE targets (VST3 + Android)
- [ ] Single-voice FLAC playback with ADSR
- [ ] Polyphony (8 voices) + chromatic pitch mapping
- [ ] Sample browser UI (list + category filter)
- [ ] JSON pack/metadata system
- [ ] Factory sound pack (50–100 CC0 sounds)
- [ ] Android APK build + latency testing
- [ ] Release: v0.1.0 beta

---

## License

Source code: MIT  
Bundled sounds: CC0 / Public Domain (see `packs/factory_pack.json` for per-file attribution)
