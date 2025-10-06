# 🎯 ПОШАГОВАЯ НАСТРОЙКА INPUT'ОВ И АНИМАЦИЙ ДЛЯ UE 5.3

## 📋 **ЧАСТЬ 1: СОЗДАНИЕ INPUT ACTIONS (UE 5.3)**

### 1.1 Откройте Content Browser
- Нажмите **Content Browser** в нижней части экрана
- Перейдите в папку: `Content/BackToZaraysk/Core/Input/Actions/Strafe/`

### 1.2 Создайте IA_A
1. **Правый клик** в папке Strafe
2. Выберите **Input → Input Action**
3. Назовите: `IA_A`
4. **Двойной клик** на созданном файле
5. В **Details Panel** установите:
   - **Value Type**: `Digital (bool)` (в UE 5.3 это новый тип)
   - **Consume Input**: `false`
   - **Trigger When Paused**: `false`
6. **Сохраните** (Ctrl+S)

### 1.3 Создайте IA_D
1. **Правый клик** в папке Strafe
2. Выберите **Input → Input Action**
3. Назовите: `IA_D`
4. **Двойной клик** на созданном файле
5. В **Details Panel** установите:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: `false`
   - **Trigger When Paused**: `false`
6. **Сохраните** (Ctrl+S)

### 1.4 Создайте IA_Space
1. **Правый клик** в папке Strafe
2. Выберите **Input → Input Action**
3. Назовите: `IA_Space`
4. **Двойной клик** на созданном файле
5. В **Details Panel** установите:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: `false`
   - **Trigger When Paused**: `false`
6. **Сохраните** (Ctrl+S)

---

## 📋 **ЧАСТЬ 2: НАСТРОЙКА INPUT MAPPING CONTEXT (UE 5.3)**

### 2.1 Откройте IMC_Default
1. В Content Browser найдите: `Content/BackToZaraysk/Core/Input/IMC_Default`
2. **Двойной клик** на файле

### 2.2 Добавьте маппинг для IA_A
1. В **Details Panel** найдите секцию **Mappings**
2. Нажмите **+** для добавления нового маппинга
3. В новом маппинге установите:
   - **Action**: `IA_A` (выберите из списка)
   - **Key**: `A` (выберите из списка клавиш)
   - **Modifiers**: оставьте пустым
   - **Triggers**: `Down` (в UE 5.3 новый интерфейс)

### 2.3 Добавьте маппинг для IA_D
1. Нажмите **+** для добавления нового маппинга
2. Установите:
   - **Action**: `IA_D`
   - **Key**: `D`
   - **Modifiers**: оставьте пустым
   - **Triggers**: `Down`

### 2.4 Добавьте маппинг для IA_Space
1. Нажмите **+** для добавления нового маппинга
2. Установите:
   - **Action**: `IA_Space`
   - **Key**: `Space Bar`
   - **Modifiers**: оставьте пустым
   - **Triggers**: `Down`

### 2.5 Сохраните IMC_Default
- **Сохраните** (Ctrl+S)

---

## 📋 **ЧАСТЬ 3: СОЗДАНИЕ КРИВОЙ ДВИЖЕНИЯ (UE 5.3)**

### 3.1 Создайте папку для кривых
1. В Content Browser перейдите в: `Content/BackToZaraysk/Core/Data/`
2. **Правый клик** → **New Folder**
3. Назовите папку: `Curves`

### 3.2 Создайте Curve Float
1. **Правый клик** в папке Curves
2. Выберите **Miscellaneous → Curve Float**
3. Назовите: `CF_StrafeMovement`
4. **Двойной клик** на созданном файле

### 3.3 Настройте кривую (UE 5.3 интерфейс)
1. В редакторе кривых нажмите **Add Key**
2. Добавьте точки:
   - **Time: 0.0, Value: 0.0** (левая кнопка мыши)
   - **Time: 0.5, Value: 1.0** (левая кнопка мыши)
   - **Time: 1.0, Value: 0.0** (левая кнопка мыши)
3. В **Details Panel** установите:
   - **Pre-Infinity**: `Cycle`
   - **Post-Infinity**: `Cycle`
4. **Сохраните** (Ctrl+S)

---

## 📋 **ЧАСТЬ 4: НАСТРОЙКА BLUEPRINT PLAYER (UE 5.3)**

### 4.1 Откройте BP_Player
1. Найдите: `Content/BackToZaraysk/Core/Characters/Player/BP_Player`
2. **Двойной клик** на файле

### 4.2 Создайте переменные для отслеживания клавиш
1. В **My Blueprint** найдите секцию **Variables**
2. Нажмите **+** для добавления переменной
3. Назовите: `bIsAPressed`
4. Тип: **Boolean**
5. **Default Value**: `False`
6. **Compile** (Ctrl+Alt+F11) - в UE 5.3 новая горячая клавиша

7. Создайте вторую переменную:
   - Название: `bIsDPressed`
   - Тип: **Boolean**
   - **Default Value**: `False`

### 4.3 Создайте Input Events (UE 5.3)
1. Перейдите в **Event Graph**
2. **Правый клик** в пустом месте
3. Введите: `IA_A`
4. Выберите **Add Event for IA_A**
5. Выберите **Started** (в UE 5.3 это аналог Pressed)

6. **Правый клик** в пустом месте
7. Введите: `IA_A`
8. Выберите **Add Event for IA_A**
9. Выберите **Completed** (в UE 5.3 это аналог Released)

### 4.4 Настройте IA_A Started Event
1. От **IA_A Started** проведите линию
2. Найдите **Set bIsAPressed**
3. Подключите к **Target**
4. Установите **New Value**: `True`

5. От **IA_A Started** проведите вторую линию
6. Найдите **Handle A Input** (если нет - создайте функцию)
7. Подключите и установите **bPressed**: `True`

### 4.5 Настройте IA_A Completed Event
1. От **IA_A Completed** проведите линию
2. Найдите **Set bIsAPressed**
3. Подключите к **Target**
4. Установите **New Value**: `False`

5. От **IA_A Completed** проведите вторую линию
6. Найдите **Handle A Input**
7. Подключите и установите **bPressed**: `False`

### 4.6 Повторите для IA_D
1. Создайте **IA_D Started** и **IA_D Completed** события
2. Подключите к **Set bIsDPressed** и **Handle D Input**
3. Аналогично настройте значения `True`/`False`

### 4.7 Создайте IA_Space Event
1. **Правый клик** → **IA_Space** → **Add Event for IA_Space** → **Started**
2. От события проведите линию к **Handle Space Input**
3. Установите **bPressed**: `True`

### 4.8 Создайте функции Handle Input
1. В **My Blueprint** найдите секцию **Functions**
2. Нажмите **+** для добавления функции
3. Назовите: `Handle A Input`

4. В функции **Handle A Input**:
   - **Input**: добавьте параметр `bPressed` (Boolean)
   - **Правый клик** → **Call Parent Function**
   - Найдите **Handle A Input**
   - Подключите **bPressed** к параметру

5. Создайте аналогично:
   - **Handle D Input**
   - **Handle Space Input**

### 4.9 Настройте StrafeComponent (UE 5.3)
1. В **Components** найдите **StrafeComponent**
2. Выберите его
3. В **Details Panel** настройте:
   - **Strafe Distance**: `200.0`
   - **Strafe Duration**: `0.3`
   - **Strafe Cooldown**: `1.0`
   - **Space Double Press Window**: `0.5`
   - **Strafe Speed**: `400.0`
   - **Smoothness Factor**: `2.0`
   - **Strafe Curve**: `CF_StrafeMovement` (выберите из списка)

### 4.10 Сохраните BP_Player
- **Compile** (Ctrl+Alt+F11)
- **Сохраните** (Ctrl+S)

---

## 📋 **ЧАСТЬ 5: НАСТРОЙКА ANIMATION BLUEPRINT (UE 5.3)**

### 5.1 Откройте ABP_Player
1. Найдите: `Content/BackToZaraysk/Core/Characters/Player/ABP_Player`
2. **Двойной клик** на файле

### 5.2 Создайте переменные стрейфа
1. В **My Blueprint** → **Variables**
2. Нажмите **+** для добавления переменной
3. Назовите: `bIsStrafing`
4. Тип: **Boolean**
5. **Default Value**: `False`

6. Создайте вторую переменную:
   - Название: `StrafeDirection`
   - Тип: **Float**
   - **Default Value**: `0.0`

### 5.3 Настройте AnimGraph (UE 5.3)
1. Перейдите в **AnimGraph**
2. **Правый клик** в пустом месте
3. Найдите **Blend Poses**
4. Выберите **Blend Poses by Bool** (в UE 5.3 улучшенный интерфейс)

### 5.4 Подключите анимации
1. К **Blend Poses by Bool** подключите:
   - **Pose A**: ваша базовая анимация (Idle/Walk)
   - **Pose B**: анимация strafing (если импортирована)
   - **Active**: переменная `bIsStrafing`

2. От **Blend Poses by Bool** проведите линию к **Output Pose**

### 5.5 Создайте Sequence Player для strafing (UE 5.3)
1. **Правый клик** → **Animations → Sequence Player**
2. В **Sequence** выберите анимацию strafing
3. В **Details Panel** установите:
   - **Play Rate**: `1.0`
   - **Start Position**: `0.0`
   - **Loop Animation**: `false`
4. Подключите к **Pose B** в Blend Poses by Bool

### 5.6 Сохраните ABP_Player
- **Compile** (Ctrl+Alt+F11)
- **Сохраните** (Ctrl+S)

---

## 📋 **ЧАСТЬ 6: ИМПОРТ АНИМАЦИИ STRAFING (UE 5.3)**

### 6.1 Найдите FBX файл
1. В Content Browser перейдите в: `Content/BackToZaraysk/Characters/Mannequins/Animations/Strafing/`
2. Найдите файл: `strafing.fbx`

### 6.2 Импортируйте анимацию (UE 5.3)
1. **Двойной клик** на `strafing.fbx`
2. В окне импорта установите:
   - **Skeleton**: `SK_Mannequin` (выберите из списка)
   - **Import Translation**: `X=0, Y=0, Z=0`
   - **Import Rotation**: `X=0, Y=0, Z=0`
   - **Import Scale**: `1.0`
   - **Use Default Sample Rate**: `true` (новая опция в UE 5.3)
   - **Import Baked Animation**: `true` (новая опция в UE 5.3)

### 6.3 Настройки анимации (UE 5.3)
1. После импорта откройте анимацию
2. В **Details Panel** установите:
   - **Play Rate**: `1.0`
   - **Enable Root Motion**: `false`
   - **Root Motion Root Lock**: `Ignore`

### 6.4 Проверьте импорт
1. Убедитесь, что анимация появилась в папке
2. **Двойной клик** на анимации для проверки
3. Анимация должна проигрываться корректно

---

## 📋 **ЧАСТЬ 7: ФИНАЛЬНАЯ ПРОВЕРКА (UE 5.3)**

### 7.1 Соберите проект
```bash
.\build_ue53.bat
```

### 7.2 Запустите игру

### 7.3 Протестируйте управление
1. **Удерживайте A** и быстро нажмите **Space дважды**
2. Персонаж должен сместиться влево с анимацией
3. **Удерживайте D** и быстро нажмите **Space дважды**
4. Персонаж должен сместиться вправо с анимацией

### 7.4 Проверьте консоль (UE 5.3)
В игре должны появиться сообщения:
- **Cyan**: Информация о движении
- **Green**: Успешный стрейф
- **Yellow**: Прогресс анимации

---

## 🔧 **ОСОБЕННОСТИ UE 5.3**

### **Новые возможности:**
- **Digital (bool)** вместо Boolean в Input Actions
- **Started/Completed** вместо Pressed/Released
- **Triggers** вместо Trigger в Input Mapping
- **Ctrl+Alt+F11** для Compile Blueprint
- Улучшенный интерфейс AnimGraph
- Новые опции импорта анимаций

### **Изменения в интерфейсе:**
- Более детальный Details Panel
- Улучшенная навигация в Content Browser
- Новые опции в Sequence Player
- Расширенные настройки кривых

---

## ✅ **ГОТОВО!**

После выполнения всех пунктов система стрейфа должна работать корректно в UE 5.3 с плавной анимацией и правильным управлением.


