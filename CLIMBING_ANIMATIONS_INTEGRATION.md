# Интеграция анимаций лазания в систему преодоления препятствий

## ✅ **АНИМАЦИИ ЛАЗАНИЯ ИНТЕГРИРОВАНЫ!**

Добавлена поддержка анимаций лазания из Free Animation Library в систему преодоления препятствий.

## 🎯 **Что было добавлено:**

### **Переменные анимации лазания в AnimInstance:**
- `bIsClimbing` - общее состояние лазания
- `bIsVaulting` - анимация перепрыгивания (Vault)
- `bIsMantling` - анимация взбирания (Climb)
- `bIsLedgeClimbing` - анимация взбирания и спуска (ClimbOver)
- `ClimbingSpeed` - скорость лазания для блендинга
- `ClimbingHeight` - высота препятствия для блендинга

### **Синхронизация с системой лазания:**
- Связь с `ObstacleClimbingComponent`
- Автоматическое определение типа лазания
- Передача данных о скорости и высоте

## 🔧 **Как работает интеграция:**

### **1. Определение типа лазания:**
```cpp
switch (PlayerCharacter->ObstacleClimbingComponent->CurrentClimbType)
{
case EObstacleClimbType::Vault:
    bIsVaulting = true;
    bIsMantling = false;
    bIsLedgeClimbing = false;
    break;
case EObstacleClimbType::Climb:
    bIsVaulting = false;
    bIsMantling = true;
    bIsLedgeClimbing = false;
    break;
case EObstacleClimbType::ClimbOver:
    bIsVaulting = false;
    bIsMantling = false;
    bIsLedgeClimbing = true;
    break;
}
```

### **2. Передача данных для анимации:**
- **ClimbingSpeed**: Скорость движения персонажа
- **ClimbingHeight**: Высота препятствия для блендинга анимаций
- **bIsClimbing**: Общее состояние лазания

### **3. Автоматическое обновление:**
- Переменные обновляются каждый кадр в `NativeUpdateAnimation`
- Автоматическое сброс состояний при завершении лазания

## 🎮 **Доступные анимации из Free Animation Library:**

### **Vault (Перепрыгивание):**
- `anim_Vault.uasset` - основная анимация перепрыгивания

### **Mantle (Взбирание):**
- `anim_Mantle_1M_R.uasset` - взбирание на 1 метр
- `anim_Mantle_2M_R.uasset` - взбирание на 2 метра

### **LedgeClimb (Взбирание и спуск):**
- `anim_LedgeClimb_ClimbUp.uasset` - взбирание на край
- `anim_LedgeClimb_Idle.uasset` - ожидание на краю
- `anim_LedgeClimbing_Down.uasset` - спуск с края
- `anim_LedgeClimbing_Left.uasset` - движение влево по краю
- `anim_LedgeClimbing_Right.uasset` - движение вправо по краю
- `anim_LedgeClimbing_Up.uasset` - подъем по краю

### **Climbing (Общее лазание):**
- `anim_Climb_Idle.uasset` - ожидание при лазании
- `anim_ClimbUp_At_Top.uasset` - подъем наверх
- `anim_GrabWall_From_Top.uasset` - захват стены сверху
- `anim_Wall_Jump_Out.uasset` - прыжок от стены

## 🎮 **Использование в Animation Blueprint:**

### **1. Переменные для блендинга:**
- Используйте `bIsVaulting`, `bIsMantling`, `bIsLedgeClimbing` для выбора анимации
- Используйте `ClimbingHeight` для блендинга между разными высотами
- Используйте `ClimbingSpeed` для контроля скорости воспроизведения

### **2. Логика переключения:**
```
State Machine:
├── Locomotion (обычное движение)
├── Vaulting (bIsVaulting = true)
│   └── anim_Vault
├── Mantling (bIsMantling = true)
│   ├── Low Height (ClimbingHeight < 100) → anim_Mantle_1M_R
│   └── High Height (ClimbingHeight >= 100) → anim_Mantle_2M_R
└── LedgeClimbing (bIsLedgeClimbing = true)
    ├── ClimbUp → anim_LedgeClimb_ClimbUp
    ├── Idle → anim_LedgeClimb_Idle
    └── ClimbDown → anim_LedgeClimbing_Down
```

### **3. Блендинг анимаций:**
- Используйте `ClimbingSpeed` для контроля скорости воспроизведения
- Используйте `ClimbingHeight` для плавного перехода между анимациями разной высоты
- Добавьте Root Motion для корректного перемещения персонажа

## 🎮 **Тестирование:**

### **В редакторе:**
1. **Запустите игру** (Play)
2. **Подойдите к препятствию** и нажмите **Space** для лазания
3. **Наблюдайте:**
   - Правильное переключение анимаций в зависимости от типа препятствия
   - Плавный блендинг между анимациями
   - Корректную работу Root Motion

### **Debug информация:**
```
AnimInstance: bIsClimbing = true, bIsVaulting = true, ClimbingHeight = 150.0
Obstacle Climbing: Starting Vault (перепрыгивание)! Height: 150.0, Thickness: 25.0
```

## 🚀 **Результат:**

**Анимации лазания интегрированы в систему преодоления препятствий!**

- ✅ **Переменные анимации** добавлены в AnimInstance
- ✅ **Синхронизация** с ObstacleClimbingComponent
- ✅ **Автоматическое определение** типа лазания
- ✅ **Передача данных** для блендинга анимаций
- ✅ **Поддержка всех типов** лазания (Vault, Mantle, LedgeClimb)
- ✅ **Готовность к использованию** в Animation Blueprint

## 📋 **Технические детали:**

### **Переменные AnimInstance:**
- `bIsClimbing` - общее состояние лазания
- `bIsVaulting` - перепрыгивание через препятствие
- `bIsMantling` - взбирание на препятствие
- `bIsLedgeClimbing` - взбирание и спуск с препятствия
- `ClimbingSpeed` - скорость для блендинга
- `ClimbingHeight` - высота для блендинга

### **Связь с компонентами:**
- **ObstacleClimbingComponent** - источник данных о лазании
- **PlayerCharacter** - доступ к компоненту лазания
- **AnimInstance** - получатель данных для анимации

### **Обновление данных:**
- Каждый кадр в `NativeUpdateAnimation`
- Автоматическое сброс при завершении лазания
- Плавные переходы между состояниями

**Анимации лазания готовы к использованию в Animation Blueprint!** 🎯





