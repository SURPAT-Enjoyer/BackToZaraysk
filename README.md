# BackToZaraysk (UE 5.3, C++)

## Overview
BackToZaraysk is an Unreal Engine 5.3 C++ project with gameplay code in `Source/` and a large amount of binary assets in `Content/`.

This repository also includes:
- a custom Foot IK system (runtime C++ traces + AnimInstance/ABP wiring helpers),
- swimming/water detection using UE's built-in **Water** plugin (with buoyancy parameterization),
- inventory + equipment framework,
- editor tooling for equipment/armor UI customization,
- Python scripts under `Content/Python/` to automate repetitive ABP IK setup tasks.

## Requirements
- Unreal Engine **5.3**
- Visual Studio **2022** (Desktop development with C++)
- Git **LFS** (recommended; the repo contains many large binary UE assets)

### Plugins expected by the project
The `BackToZaraysk.uproject` enables (among others):
- `Water`
- `PythonScriptPlugin`
- `EditorScriptingUtilities`
- `VisualStudioTools` (Win64)
- `ModelingToolsEditorMode`

## Git LFS setup (recommended)
This repo already ships a `.gitattributes` that tracks `*.uasset`, `*.umap`, and other large binaries via LFS.

1. Install Git LFS.
2. Run:
```bat
git lfs install
```

If you already have the repo locally and want to ensure LFS tracking is applied:
```bat
git lfs track "*.uasset" "*.umap"
git add .gitattributes
git commit -m "chore: ensure git-lfs tracking"
```

## Build

### Option A (recommended): build editor via script
```bat
build_ue53.bat
```

### Option B: build editor via MSBuild script
Generate project files if needed, then run:
```bat
build_msbuild_game.bat
```

## Run
Open:
`BackToZaraysk.uproject`

## Repository structure

### Top-level
- `Source/`  
  C++ gameplay code (single main runtime module: `BackToZaraysk`).
- `Content/`  
  All Unreal assets (characters, blueprints, maps, materials, etc).
- `Config/`  
  Unreal config files.
- `build_ue53.bat`, `build_msbuild_game.bat`  
  Local build helpers.
- `BackToZaraysk.sln`, `BackToZaraysk.uproject`  
  The main project.
- `Content/__ExternalActors__/` and `Content/__ExternalObjects__/`  
  World Partition external actor data used by UE.

### Source module layout (`Source/BackToZaraysk/`)
Main folders in the C++ module:
- `Actors/`
  - Example: `PlatformBase`, `PlatformInvokator` (world interaction helpers).
- `Characters/`
  - `BTZBaseCharacter.*`: base character + Foot IK runtime trace logic.
  - `PlayerCharacter.*`: the player implementation + Water/swing/swim control.
  - `Animations/BTZBaseCharacterAnimInstance.*`: AnimInstance that consumes IK offsets and drives ABP variables.
  - `Controllers/BTZPlayerController.*`: input/control layer.
- `Components/`
  - `EquipmentComponent.*`: equipment management.
  - `InvokatorClientComponent.*`: invocator-related networking/client logic.
  - `ObstacleClimbingComponent.*`, `StrafeComponent.*`
  - `MovementComponents/*`: movement logic split for pawn vs base character.
- `Inventory/`
  - `InventoryComponent`, `InventoryWidget`, `PickupItem`, and data types for inventory items.
- `GameData/Items/`
  - Item data definitions and prototypes.
- `Editor/`
  - Editor-only customizations (details panels / grid editors for armor & equipment).

## Key systems

### 1) Foot IK (Inverse Kinematics)
The Foot IK system is implemented as two parts:

1. **C++ trace solver** (character-side)  
   - File(s):  
     - `Characters/BTZBaseCharacter.cpp`
     - `Characters/BTZBaseCharacter.h`
   - Logic highlights:
     - Trace origin is anchored to the mesh foot socket (not the capsule).
     - Two traces are performed:
       - downward (finding “top” contact),
       - upward (to help when foot is inside/near underside surfaces).
     - A penetration overlap test biases the chosen contact to resolve “utopanie”.
     - Debug drawing:
       - red/blue lines for down/up traces
       - green/cyan spheres for raw hits
       - yellow sphere for chosen contact
     - The computed Z offset for each foot is stored as:
       - `IKLeftFootOffset`, `IKRightFootOffset`

2. **AnimInstance / ABP bridge** (animation-side)
   - File(s):  
     - `Characters/Animations/BTZBaseCharacterAnimInstance.cpp/.h`
   - Logic highlights:
     - `NativeUpdateAnimation` reads raw offsets from the character.
     - Converts offsets into effector targets (component space).
     - Drives:
       - `LeftFootEffectorLocation`
       - `RightFootEffectorLocation`
       - `LeftFootIKAlpha`
       - `RightFootIKAlpha`
     - Pelvis offset is computed based on both feet (and includes filtering/deadzones).
     - Optional foot-lock curves are supported, but there is a fallback to always-on IK when curves are missing/not working.

#### Foot IK debug parameters
Relevant knobs exist in `BTZBaseCharacter.h` / `BTZBaseCharacterAnimInstance.h` such as:
- `bDebugFootIK`
- trace distances, offsets, and penetration resolve lift
- FLAT/noise suppression parameters (epsilon + stability + max speed gate)

### 2) Water / Swimming (UE Water plugin)
Swimming is detected using the built-in **Water** plugin:
- `PlayerCharacter.cpp` queries the closest water body via `UWaterSubsystem` / `UWaterBodyComponent`
- It computes whether the water surface is above a chest-based threshold
- If true, movement switches to swimming capsule settings:
  - `BTZBaseCharMovementComponent::EnterSwimmingCapsule()`
  - `BTZBaseCharMovementComponent::ExitSwimmingCapsule()`
- Movement mode uses `MOVE_Flying` while swimming (controls are adapted in code).

Buoyancy behavior:
- A per-character buoyancy value is applied to vertical input.
- The current design (per earlier tuning) intentionally avoids “increasing force with depth” that would otherwise press the character into the bottom.

### 3) Inventory + Equipment
Core C++ pieces:
- `InventoryComponent` and UI widgets:
  - `InventoryWidget*`, `InventoryGridWidget*`, `InventoryItemWidget*`
- Pickup logic:
  - `PickupItem.*`
- Equipment system:
  - `EquipmentComponent.*`
  - equipment slot definitions and item data are located under `Source/BackToZaraysk/Inventory/` and `GameData/Items/`

### 4) Equipment/Armor editor tooling
Editor-only classes in `Source/BackToZaraysk/Editor/` provide custom details panels and editor windows for armor mods/equipment grids.

## Python scripts (Content/Python)
The folder `Content/Python/` contains helper scripts to automate ABP IK setup for UE Animation Blueprints.

Notable docs included:
- `README_IK_Setup.md` (overview of IK setup scripts)
- `WORKING_IK_SETUP_INSTRUCTIONS.md` (working auto setup flow)
- `MANUAL_ABP_PLAYER_IK_FIX.md` (manual ABP_Player correction instructions)

Typical usage pattern:
1. Open Unreal Editor
2. Open Python console: `Ctrl + Shift + P`
3. Run scripts like:
```python
exec(open('Content/Python/SimpleIKSetup.py').read())
```

## Asset storage notes (World Partition external actors)
This project uses UE World Partition external actor storage:
- `Content/__ExternalActors__/`
- `Content/__ExternalObjects__/`

When uploading to GitHub, make sure those folders are included (otherwise the map can load incompletely).

## Build/Editor notes
- `Binaries/`, `Intermediate/`, `Saved/`, and common IDE caches are ignored via `.gitignore`.
- Use Git LFS to avoid hitting GitHub size limits.

## Current development status
This repo may contain local uncommitted changes (C++ code and some binary assets). Before publishing a release, ensure:
- build scripts are present and executable in CI/local
- your final commit includes the required `.uasset` changes for ABP/characters
- you commit or discard untracked UE assets depending on whether they belong to your intended project state

