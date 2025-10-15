# BackToZaraysk

Проект на Unreal Engine 5.3+ с системой инвентаря, экипировки и взаимодействия с предметами.

## Основные возможности

- **Система инвентаря**: Полнофункциональный инвентарь с drag-and-drop, ротацией предметов (R), контекстными меню
- **Экипировка**: Автоматическая экипировка предметов (рюкзаки, жилеты) при подборе
- **Хранилища**: Рюкзак с сеткой, жилет с дополнительным хранилищем, пояс, 4 кармана 1x1
- **Предметы**: Кубы 1x1 и 2x2, рюкзаки, тактические жилеты с визуальным отображением на персонаже
- **Персистентность**: Сохранение содержимого рюкзаков при выбросе/подборе
- **UI**: Адаптивный интерфейс инвентаря с поддержкой клавиатуры и мыши

## Система управления

- **I** - Открыть/закрыть инвентарь
- **E** - Взаимодействие с предметами
- **G** - Выбросить последний предмет из рюкзака
- **R** - Повернуть предмет (при наведении курсора или перетаскивании)
- **ПКМ** - Контекстное меню предмета (Надеть/Снять/Выбросить)
- **Перетаскивание** - Перемещение предметов между слотами и хранилищами

## Сборка и запуск

### Требования
- Unreal Engine 5.3+
- Visual Studio 2022 (C++ Desktop, Windows SDK)
- Git LFS

### Установка
```bash
# Установка Git LFS
git lfs install

# Клонирование репозитория
git clone https://github.com/SURPAT-Enjoyer/BackToZaraysk.git
cd BackToZaraysk

# Открытие проекта
# Двойной клик на BackToZaraysk.uproject или через UE Launcher
```

### Сборка
```bash
# Автоматическая сборка (закрывает редактор и собирает проект)
.\build_ue53.bat

# Затем запустите проект через UE Launcher
```

## Архитектура проекта

### Основные компоненты
- `UInventoryComponent` - Управление инвентарем и экипировкой
- `UInventoryWidget` - UI инвентаря с drag-and-drop
- `UEquipmentComponent` - Визуальное отображение экипированных предметов
- `APickupBase` и наследники - Предметы в мире для подбора

### Система предметов
- `UInventoryItemData` - Базовые данные предмета
- `UEquippableItemData` - Данные экипируемых предметов с хранилищами
- `APickupCube`, `APickupBackpack`, `ATacticalVest` - Pickup-акторы

## Git LFS

Проект использует Git LFS для бинарных файлов:
- `.uasset`, `.umap`, `.ubulk`, `.uexp` - Ассеты UE5
- `.fbx` - 3D модели
- `.png`, `.jpg`, `.tga`, `.bmp` - Текстуры
- `.wav`, `.ogg`, `.mp3` - Аудио

## Что исключено из репозитория

- `Binaries/` - Скомпилированные файлы
- `DerivedDataCache/` - Кэш UE5
- `Intermediate/` - Промежуточные файлы сборки
- `Saved/` - Сохранения и логи
- IDE файлы: `.vs/`, `.idea/`, `.vscode/`
- Системные файлы: `Thumbs.db`, `.DS_Store`

## Разработка

### Структура кода
- `Source/BackToZaraysk/` - Основной код проекта
- `Content/` - Ассеты и Blueprint'ы
- `Config/` - Конфигурационные файлы

### Отладка
- Логи сборки: `Saved/Logs/`
- Отладочные сообщения в игре через `GEngine->AddOnScreenDebugMessage`

## Лицензия

Проект разработан для образовательных целей.

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