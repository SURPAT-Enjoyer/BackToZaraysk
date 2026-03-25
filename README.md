# BackToZaraysk (UE 5.7, C++)

Игровой проект на **Unreal Engine 5.7** с основной логикой на **C++** и большим количеством ассетов в `Content/`.
Код расположен в `Source/`, все игровые данные/карты/blueprint-ы — в `Content/`.

## Содержание
- [Что внутри (крупные подсистемы)](#что-внутри-крупные-подсистемы)
- [Unreal модуль проекта](#unreal-модуль-проекта)
- [Подробно по папкам `Source/`](#подробно-по-папкам-source)
- [Редактор модификаций с вьюпортом](#редактор-модификаций-с-вьюпортом)
- [Python-скрипты](#python-скрипты)
- [Технические требования](#технические-требования)
- [Сборка и запуск](#сборка-и-запуск)
- [GitHub: подготовка репозитория](#github-подготовка-репозитория)
- [World Partition: важное про внешние акторы](#world-partition-важное-про-внешние-акторы)

## Что внутри (крупные подсистемы)

1. **Foot IK**
   - Трассировки стоп в рантайме (вычисление Z-отклонения).
   - Передача данных в `AnimInstance`/ABP, драйв эффекторов/альф.
2. **Вода и плавание**
   - Использование UE-плагина **Water**.
   - Определение глубины/поверхности и переключение поведения персонажа в воде.
3. **Инвентарь и экипировка**
   - Сетка инвентаря, drag-and-drop, контекстное меню.
   - Экипируемые предметы по слотам и визуальное прикрепление к персонажу.
4. **Модифицируемое снаряжение (moddable)**
   - У экипировки есть флаг `bIsModdable`.
   - Для `bIsModdable=true` доступно редактирование сеток модов и установка/снятие модулей.
   - Окно модификации построено на вьюпорте (как у `ArmorBase`), работает и для `BeltBase`.
5. **Редакторные инструменты**
   - Details customization: кнопка `Edit Mod Grids...` при `bIsModdable=true`.
   - Отдельное окно с `Viewport` для ручного редактирования сеток модификаций.
6. **Python-скрипты**
   - Автоматизация повторяющихся задач настройки ABP/IK.

## Unreal модуль проекта

Проект использует один основной C++ модуль:
- `BackToZaraysk` (runtime)

Файл сборки:
- `Source/BackToZaraysk/BackToZaraysk.Build.cs`

Ключевые зависимости:
- Public: `Core`, `CoreUObject`, `Engine`, `InputCore`, `UMG`, `Water`
- Private: `ProceduralMeshComponent`, а также `Slate/SlateCore` для UI
- При сборке Editor (`Target.bBuildEditor`) добавляются editor-only зависимости (`UnrealEd`, `PropertyEditor`, и т.п.)

В проекте используется `WITH_EDITOR` для кодовой части редакторных инструментов.

## Подробно по папкам `Source/`

### `Actors/`
Содержит акторы/модули взаимодействия уровня.

### `Characters/`
Содержит базовый персонаж, анимационные bridge и контроллеры.
Ключевые файлы:
- `Characters/BTZBaseCharacter.*` — логика Foot IK (трассировки и выбор контакта)
- `Characters/BTZBaseCharacterAnimInstance.*` — передача IK-отклонений в ABP
- `Characters/BTZBasePawnAnimInstance.*` — анимационный слой для pawn/переходов
- `Characters/Controllers/BTZPlayerController.*` — управление/ввод
- `Characters/PlayerCharacter.*` — поведение игрока + интеграция water/swim

### `Components/`
Компоненты игрового поведения.
Ключевые:
- `Components/EquipmentComponent.*` — визуальная часть экипировки (прицепление мешей, обновление модов)
- `Components/MovementComponents/*` — адаптация movement под swim/состояния
- `Components/Inventory` в `Source/BackToZaraysk/Inventory/*` (см. ниже) — это отдельный подраздел

### `Inventory/`
Инвентарь, UI и DnD.
Ключевые компоненты:
- `InventoryComponent.*` — хранение предметов, экипировка/снятие, размещение в сетках
- `InventoryWidget.*` — главное окно инвентаря и окна модификации
- `InventoryItemWidget.*` — элемент предмета + контекстное меню (в т.ч. установка/модификация)
- `InventoryGridWidget.*` — грид с ячейками (визуал + подсветка/выделение)
- `InventoryBlueprintLibrary.*` — blueprint-хелперы для UI/игрового кода
- `PickupItem.*` — pickup/выброс предметов в мир (визуал и трансферы)

### `GameData/Items/`
Классы данных и стартовых (default) параметров предметов/экипировки.
Важно:
- оборудование (например `ArmorBase`, `BackpackBase`, `VestBase`, `HelmetBase`, а также `BeltBase`)
- moddable модификации (`EquipModBase` + размеры/footprint + дополнительные хранилища)

### `Editor/`
Редакторные кастомизации и инструменты.
Ключевые классы:
- `ArmorBaseDetailsCustomization.*`
  - добавляет в Details кнопку `Edit Mod Grids...` при `bIsModdable=true`
- `ArmorModGridEditorWindow.*`
  - окно с viewport для редактирования сеток модов
- `EquipModBaseDetailsCustomization.*`
  - редактор Attachment Grid для конкретного мода (если у мода есть такая логика)

## Редактор модификаций с вьюпортом

Редактор сеток модов завязан на флаг `bIsModdable` в `AEquipmentBase`:

1. **Details customization**
   - Если у экипировки `bIsModdable=true`, появляется кнопка `Edit Mod Grids...`
2. **Окно с вьюпортом**
   - Открывается `ArmorModGridEditorWindow` и создаёт `AArmorModPreviewActor` для отображения меша и отрисовки сеток.
3. **Что именно редактируется**
   - `ModsFrontOrigin / ModsFrontRotation`
   - `ModsBackOrigin / ModsBackRotation`
   - `ModsLeftOrigin / ModsLeftRotation`
   - `ModsRightOrigin / ModsRightRotation`
   - и какие стороны реально включены (`bModsFront/Back/Left/Right`)

После правок нужно сохранять актор/данные предмета в UE.

## Python-скрипты

Папка `Content/Python/` содержит вспомогательные скрипты для:
- быстрой настройки ABP переменных/эффекторов
- повторяющихся шагов IK/анимации

Пример запуска:
```python
exec(open('Content/Python/SimpleIKSetup.py').read())
```

## Технические требования

- Unreal Engine **5.7** (установленная сборка через Epic Games Launcher)
- Visual Studio **2022**
- Платформа: **Win64**
- Рекомендуется **Git LFS** для больших бинарных ассетов

В `BackToZaraysk.uproject` отключён `VisualStudioTools`, чтобы:
- избежать проблем с зависимостями/сборкой на установленной версии движка
- стабилизировать открытие проекта

## Сборка и запуск

### Сборка редактора

1) Через батник (если они есть в корне):
```bat
build_ue53.bat
```

или напрямую:
```bat
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" BackToZarayskEditor Win64 Development "C:\Path\To\BackToZaraysk\BackToZaraysk.uproject" -WaitMutex -NoHotReload
```

### Запуск редактора
```bat
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Path\To\BackToZaraysk\BackToZaraysk.uproject" -log
```

## GitHub: подготовка репозитория

1. Убедитесь, что в папке проекта есть этот файл и `.gitignore`/`.gitattributes`:
   - если `git` не инициализирован, сделайте `git init`
2. Настройте LFS:
   ```bat
   git lfs install
   git lfs track "*.uasset" "*.umap"
   ```
3. Не коммитьте:
   - `Binaries/`, `Intermediate/`, `Saved/`, `.vs/`
4. Соберите `BackToZarayskEditor` на UE 5.7 перед пушем (чтобы CI/проверка прошла).

## World Partition: важное про внешние акторы

Если вы правили карты/уровни с **World Partition**, их правки обычно хранятся во внешних папках:
- `Content/__ExternalActors__/`
- `Content/__ExternalObjects__/`

Их **нельзя** пропускать при коммите изменений карты, иначе сцену можно загрузить неполностью.

## Подробно по подсистемам (модули логики)

Ниже — разбиение по основным «модулям» игры/редактора. В проекте обычно нет много Unreal-модулей (.Build.cs), поэтому под «модулем» понимается функциональная подсистема и набор ключевых классов.

### 1) Foot IK (модуль обратной кинематики стоп)

Цель: корректно приподнимать/опускать стопы по геометрии поверхности и передавать это в AnimBlueprint.

Основные элементы:
- `Characters/BTZBaseCharacter.*`
  - вычисляет контакты под левой/правой стопой (трассировки вниз/вверх + выбор приоритетного контакта)
  - формирует итоговые смещения (Z-отклонения) и альфы (силу IK)
- `Characters/BTZBaseCharacterAnimInstance.*`
  - читает смещения персонажа в `NativeUpdateAnimation`
  - переводит данные в нужное пространство и управляет переменными ABP

Типовые настройки/точки интеграции:
- привязка трасс к сокетам стоп (а не к капсуле)
- обработка «утопания» (когда стопа частично внутри поверхности)
- отладочные флаги (через переменные в character/anim instance)

### 2) Water / Swimming (модуль воды и плавания)

Цель: переключать движение персонажа при достижении порога глубины и применять соответствующие настройки капсулы/режима.

Ключевые части:
- `Characters/PlayerCharacter.*`
  - определяет ближайший водный объект
  - вычисляет глубину относительно контрольной точки персонажа (порог «считается/не считается водой»)
- `MovementComponents/*` / `BTZBaseCharMovementComponent.*`
  - вход/выход в «плавающий» режим
  - подстройка капсулы и режима движения

Параметры:
- плавучесть персонажа (в коде она параметризуется, но основная логика плавания запускается от глубины)

### 3) Inventory (модуль инвентаря и DnD UI)

Цель: хранить предметы, размещать их в сетках, обрабатывать drag-and-drop и контекстные действия.

Основные элементы:
- `Inventory/InventoryComponent.*`
  - массивы/сетки хранения предметов (включая карманы/слоты экипировки)
  - правила перемещения предметов по клеткам
  - функции `EquipItemFromInventory` / `UnequipItemToInventory`
- `Inventory/InventoryWidget.*`
  - строит UI окна инвентаря
  - рисует гриды (инвентарь рюкзака, карманы, слоты экипировки)
  - содержит логику drag-and-drop поверх UI
  - управляет окнами модификации для moddable экипировки
- `Inventory/InventoryItemWidget.*`
  - отображает конкретный предмет
  - формирует контекстное меню (в т.ч. «Модифицировать» при `bIsModdable=true`)
- `Inventory/InventoryGridWidget.*`
  - вспомогательная визуализация/подсветка ячеек гридов
- `Inventory/InventoryBlueprintLibrary.*`
  - функции-помощники для связки Blueprint и C++

### 4) Equipment (модуль экипировки и визуального крепления)

Цель: надевать/снимать предметы и прикреплять визуальные меши к персонажу.

Основные элементы:
- `Components/EquipmentComponent.*`
  - создаёт компоненты мешей (skeletal/static) для конкретного слота
  - прикрепляет их к нужному сокету персонажа
  - после установки/снятия модов обновляет визуальные «обвесы»
- `GameData/Items/EquipmentBase.*`
  - базовая логика экипировки на акторе (прикрепление к сокету, физика, визуал из `ItemInstance`)
  - хранит флаг модифицируемости `bIsModdable` и данные сеток модов

Слоты экипировки:
- `EquipmentSlotType.h` включает `Helmet`, `Vest`, `Backpack`, `Armor`, а также `Belt`.

### 5) Moddable equipment (модуль модифицируемого снаряжения)

Цель: если у экипировки включён `bIsModdable`, то появляется редактирование сетки и возможность установки/снятия модулей.

Как работает на уровне данных:
- `UEquippableItemData`
  - `bIsModdable` — ключевой флаг
  - `ModSideGrids` — список сторон/размеров сеток модов
  - `InstalledMods` — список установленных модов с координатами на сетке
  - методы:
    - `CanInstallModAt(...)`
    - `InstallMod(...)`
    - `UninstallMod(...)`
    - `GetInstalledModIndexAt(...)`
- `UEquipModBaseItemData`
  - данные мод-элемента (footprint, дополнительные мини-гриди, если мод хранит предметы)

Как работает в runtime/визуале:
- `EquipmentComponent` обеспечивает прикрепление визуальных компонентов модов к мешу экипировки
- `ArmorModPreviewActor` используется во **вьюпортном редакторе** (см. ниже)

Как работает в UI:
- `InventoryItemWidget` показывает пункт «Модифицировать» для любого экипируемого предмета с `bIsModdable=true`
- при выборе — открывается окно редактирования сеток во вьюпорте

### 6) Редактор сеток модификаций (Viewport Editor)

Инструменты в редакторе:
- `Editor/ArmorBaseDetailsCustomization.*`
  - кастомизация Details: кнопка `Edit Mod Grids...` появляется при `bIsModdable=true`
- `Editor/ArmorModGridEditorWindow.*`
  - окно редактора с `Viewport`
  - рисует сетки модов и обрабатывает ввод (выбор ячеек, перенос origin/rotation)
- `GameData/Items/ArmorModPreviewActor.*`
  - актор предпросмотра: отображает меш экипировки и подсвечивает сетку/клетки

Важный момент:
- редактирование сеток привязано к 4 сторонам: Front/Back/Left/Right (в зависимости от включённых `bMods*`).

### 7) BeltBase (подсистема пояса)

Пояс сделан как полноценная экипировка по аналогии с `ArmorBase`/`BackpackBase`:
- новый слот `Belt` (в `EquipmentSlotType.h`)
- базовый актор `ABeltBase`
- базовые item data `UBeltBaseItemData`

Поддержка:
- надеваемость/снятие/возврат в инвентарь
- возможность включить `bIsModdable` и редактировать/устанавливать моды
- окно модификаций во вьюпорте работает так же, как для armor


