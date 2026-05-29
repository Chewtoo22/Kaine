# Kaine

Kaine is a local-first AI command center inspired by cinematic personal assistants, built as an original assistant with its own identity, memory, voice hooks, and safe operator workflows.

## Run

```powershell
python .\kaine_server.py
```

Then open:

```text
http://127.0.0.1:8766
```

The app runs without API keys. The first slice includes a local reasoning fallback, durable memory in `data/kaine_state.json`, voice input/output where the browser supports it, and a safe project/status action layer.

## Unreal Rebuild

The native UE5 rebuild lives in `KaineUE5/`.

Build:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" KaineUE5Editor Win64 Development -Project="C:\Users\matth\OneDrive\Documents\Kaine\KaineUE5\KaineUE5.uproject" -WaitMutex -MaxParallelActions=2
```

Run standalone:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\matth\OneDrive\Documents\Kaine\KaineUE5\KaineUE5.uproject" "/Engine/Maps/Templates/OpenWorld" -game -windowed -ResX=1280 -ResY=720 -log
```

The UE5 slice spawns a real-time 3D command lab: animated Kaine core with a procedural faceted shell, premium in-world UI graphics, tabbed command rails, a radial scanner, memory-node graph, framed status cards, authored beveled server racks, segmented glass halo panels, animated telemetry bars, cinematic camera pawn, Lumen/VSM renderer settings, post processing, fog, softbox lighting, workstations, and a polished cinematic HUD.

## Verify

```powershell
python -m unittest discover -s tests
python -m py_compile .\kaine_server.py
```

## Current Capabilities

- Live assistant UI with animated Kaine identity core.
- Chat with operator, engineer, security, strategy, and creative modes.
- Local memory capture from explicit "remember..." prompts.
- Mission queue generation for build and planning requests.
- Safe project snapshot and runtime status APIs.
- Browser speech synthesis and speech recognition hooks with graceful fallback.
- Native Unreal Engine 5.7 photoreal-style command lab in `KaineUE5/`.

## Scope

This is the foundation slice. It is intentionally secure-by-default: Kaine does not execute arbitrary commands yet. Future tool execution should require explicit allowlists, confirmations, logs, and scoped workspace permissions.
