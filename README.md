# BackToZaraysk (UE 5.7, C++)

## Overview
BackToZaraysk is an Unreal Engine 5.7 C++ project with gameplay code in `Source/` and binary assets in `Content/`.

Main implemented systems:
- custom Foot IK (runtime traces + AnimInstance/ABP bridge),
- swimming/water detection via UE `Water` plugin,
- inventory + equipment,
- moddable equipment grids (runtime + editor tooling),
- Python helpers in `Content/Python/` for repetitive animation setup tasks.

## Requirements
- Unreal Engine **5.7** (installed build from Epic Games Launcher)
- Visual Studio **2022** (Desktop development with C++)
- Git **LFS** (strongly recommended)

### Plugins expected by the project
The `BackToZaraysk.uproject` enables (among others):
- `Water`
- `PythonScriptPlugin`
- `EditorScriptingUtilities`
- `ModelingToolsEditorMode`

`VisualStudioTools` is intentionally disabled in this project for stable opening/building on UE 5.7 installed engine setups.

## Git LFS setup
This repo uses LFS for large binary UE assets (`*.uasset`, `*.umap`, etc).

```bat
git lfs install
```

For verification:
```bat
git lfs ls-files
```

## Build

### Option A: helper script
```bat
build_ue53.bat
```

### Option B: MSBuild helper
```bat
build_msbuild_game.bat
```

### Option C: direct UE build
```bat
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" BackToZarayskEditor Win64 Development "C:\Path\To\BackToZaraysk\BackToZaraysk.uproject" -WaitMutex -NoHotReload
```

## Run
Open `BackToZaraysk.uproject`, or:

```bat
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Path\To\BackToZaraysk\BackToZaraysk.uproject" -log
```

## Repository structure

### Top-level
- `Source/` - C++ gameplay code.
- `Content/` - UE assets.
- `Config/` - UE config files.
- `BackToZaraysk.uproject` - main project file.
- `build_ue53.bat`, `build_msbuild_game.bat` - local build helpers.
- `Content/__ExternalActors__/`, `Content/__ExternalObjects__/` - World Partition external data.

### Main C++ module (`Source/BackToZaraysk/`)
- `Actors/` - world actors/helpers.
- `Characters/` - character, controller, animation bridge.
- `Components/` - movement/equipment/interaction components.
- `Inventory/` - item data, inventory logic, widgets.
- `GameData/Items/` - pickup/equipment item classes and defaults.
- `Editor/` - details customizations and grid editor windows.

## Key systems

### Foot IK
- Runtime trace solver in character code.
- Animation bridge in `BTZBaseCharacterAnimInstance`.
- Debug-friendly offsets/alphas exposed for ABP driving.

### Water / Swimming
- Uses UE `Water` subsystem queries.
- Switches movement/capsule behavior when water depth passes swim threshold.
- Supports buoyancy parameterization.

### Inventory / Equipment
- Grid inventory and drag-and-drop.
- Equipment slots and visual attach on character.
- Moddable equipment path via `bIsModdable` + grid side data.

### Equipment grid editor tooling
Editor-only tools in `Source/BackToZaraysk/Editor/` provide viewport-based grid editing for moddable equipment.

## Python scripts
`Content/Python/` contains helper scripts for animation/IK automation.

Example run from editor Python console:
```python
exec(open('Content/Python/SimpleIKSetup.py').read())
```

## Publishing To GitHub Checklist
Before pushing/releasing:

1. Ensure Git LFS is installed and active.
2. Ensure `Binaries/`, `Intermediate/`, `Saved/`, `.vs/` are not committed.
3. Build `BackToZarayskEditor` on UE 5.7 successfully.
4. Open project from `.uproject` and verify it starts without plugin errors.
5. Review binary changes in `Content/` and World Partition external folders.
6. Keep this README and build instructions in sync with current engine version.

## Notes
- Many gameplay features depend on matching C++ + Blueprint/assets state.
- Do not omit `__ExternalActors__` / `__ExternalObjects__` when committing World Partition map edits.

