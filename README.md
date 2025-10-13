# BackToZaraysk (UE5)

Unreal Engine 5 project.

## Requirements
- Git + Git LFS (`git lfs install`)
- Visual Studio 2022 (C++ toolchain)
- Unreal Engine 5.3+
- Windows PowerShell

## Clone
```bash
git clone <your-repo-url>
cd BackToZaraysk
git lfs install
```

## Build
```powershell
# From project root
./build_ue53.bat
```

## Run Editor
```powershell
Start-Process .\BackToZaraysk.uproject
```

## Notes
- Large assets (`*.uasset`, `*.umap`, `*.fbx`, textures, audio) are tracked by Git LFS per `.gitattributes`.
- Do NOT commit `Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`.
- Source code and config live in `Source/` and `Config/`.