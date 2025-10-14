BackToZaraysk (UE5)

Сборка и запуск
- Требования: Unreal Engine 5.3+, Visual Studio 2022 (C++ Desktop, Windows SDK)
- Клонируйте репозиторий с Git LFS:
  - git lfs install
  - git clone <repo-url>
- Откройте `BackToZaraysk.uproject` (через UE Launcher или двойным кликом).
- Постройка из консоли: запустите `build_ue53.bat` (он автоматически закрывает редактор при сборке).

Git LFS
- Включены типичные UE-форматы: .uasset, .umap, .ubulk, .uexp, а также изображения/аудио/FBX.
- Не забывайте выполнять `git lfs install` перед первым клонированием.

Что не хранится в git
- Каталоги: `Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`.
- IDE и системный мусор: `.vs/`, `.idea/`, `.vscode/`, `Thumbs.db`, `.DS_Store` и т.д.

Полезно
- Скрипт `build_ue53.bat` завершает процесс редактора и собирает проект, затем вы можете запускать `.uproject`.
- Если сборка не проходит, проверьте логи в `Saved/Logs/` локально (не коммитится).

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