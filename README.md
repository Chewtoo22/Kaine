# Kaine

Kaine is a local-first AI command center inspired by cinematic personal assistants, built as an original assistant with its own identity, memory, voice hooks, and safe operator workflows.

## Run

```powershell
.\Start-Kaine.ps1
```

Or run the server directly:

```powershell
python .\kaine_server.py
```

Then open:

```text
http://127.0.0.1:8766
```

The app runs without API keys. The finished MVP includes a local reasoning fallback, durable memory in `data/kaine_state.json`, mission lifecycle controls, voice input/output where the browser supports it, and an allowlisted read-only action layer for project inspection.

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
- Mission queue generation plus active/complete mission status controls.
- Safe project snapshot, Git status, UE5 readiness, memory digest, and runtime status actions.
- Audited action history in local state.
- Browser speech synthesis and speech recognition hooks with graceful fallback.
- Native Unreal Engine 5.7 photoreal-style command lab in `KaineUE5/`.

## Scope

This is the finished local MVP. It is intentionally secure-by-default: Kaine exposes allowlisted read-only actions, logs every action invocation, and does not expose arbitrary shell execution from the UI or API. Future write-capable tools should require explicit allowlists, confirmations, logs, and scoped workspace permissions.
