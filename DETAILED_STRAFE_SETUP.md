# 🎯 ДЕТАЛЬНАЯ НАСТРОЙКА СИСТЕМЫ СТРЕЙФА

## ✅ Папки созданы! Теперь настройка в UE5 Editor.

### 📁 **Созданные папки:**
- `Content/BackToZaraysk/Core/Input/Actions/Strafe/`
- `Content/BackToZaraysk/Core/Data/Curves/`

---

## 🔧 **ШАГ 1: Создание Input Actions**

### 1.1 Откройте UE5 Editor
### 1.2 В Content Browser перейдите в папку:
```
Content/BackToZaraysk/Core/Input/Actions/Strafe/
```

### 1.3 Создайте 3 Input Action:
**Правый клик → Input → Input Action**

#### IA_A
- **Name**: `IA_A`
- **Value Type**: `Boolean`

#### IA_D  
- **Name**: `IA_D`
- **Value Type**: `Boolean`

#### IA_Space
- **Name**: `IA_Space`
- **Value Type**: `Boolean`

---

## 🔧 **ШАГ 2: Настройка Input Mapping Context**

### 2.1 Откройте:
```
Content/BackToZaraysk/Core/Input/IMC_Default
```

### 2.2 Добавьте новые маппинги:

#### Маппинг для IA_A:
- **Action**: `IA_A`
- **Key**: `A`
- **Modifiers**: `None`
- **Trigger**: `Impulse`

#### Маппинг для IA_D:
- **Action**: `IA_D`
- **Key**: `D`
- **Modifiers**: `None`
- **Trigger**: `Impulse`

#### Маппинг для IA_Space:
- **Action**: `IA_Space`
- **Key**: `Space Bar`
- **Modifiers**: `None`
- **Trigger**: `Impulse`

### 2.3 Сохраните IMC_Default

---

## 🔧 **ШАГ 3: Создание Curve для стрейфа**

### 3.1 В Content Browser перейдите в:
```
Content/BackToZaraysk/Core/Data/Curves/
```

### 3.2 Создайте Curve Float:
**Правый клик → Miscellaneous → Curve Float**

- **Name**: `CF_StrafeMovement`

### 3.3 Настройте кривую:
1. **Откройте кривую**
2. **Добавьте точки**:
   - **Time 0.0, Value 0.0**
   - **Time 0.5, Value 1.0**
   - **Time 1.0, Value 0.0**
3. **Сохраните кривую**

---

## 🔧 **ШАГ 4: Настройка BP_Player**

### 4.1 Откройте:
```
Content/BackToZaraysk/Core/Characters/Player/BP_Player
```

### 4.2 В Event Graph добавьте Input Events:

#### IA_A Event:
1. **Правый клик → Add Event for IA_A**
2. **Выберите Pressed**
3. **Создайте переменную**: `bIsAPressed` (Boolean)
4. **Set bIsAPressed = True**

1. **Правый клик → Add Event for IA_A**
2. **Выберите Released**
3. **Set bIsAPressed = False**

#### IA_D Event:
1. **Правый клик → Add Event for IA_D**
2. **Выберите Pressed**
3. **Создайте переменную**: `bIsDPressed` (Boolean)
4. **Set bIsDPressed = True**

1. **Правый клик → Add Event for IA_D**
2. **Выберите Released**
3. **Set bIsDPressed = False**

#### IA_Space Event:
1. **Правый клик → Add Event for IA_Space**
2. **Выберите Pressed**
3. **Call Parent Function: Handle Space Input**
4. **Pass: True**

### 4.3 Создайте Custom Functions:

#### Handle A Input:
1. **Функции → Add Function**
2. **Name**: `Handle A Input`
3. **Input**: `bPressed` (Boolean)
4. **Call Parent Function: Handle A Input**
5. **Pass: bPressed**

#### Handle D Input:
1. **Функции → Add Function**
2. **Name**: `Handle D Input`
3. **Input**: `bPressed` (Boolean)
4. **Call Parent Function: Handle D Input**
5. **Pass: bPressed**

#### Handle Space Input:
1. **Функции → Add Function**
2. **Name**: `Handle Space Input`
3. **Input**: `bPressed` (Boolean)
4. **Call Parent Function: Handle Space Input**
5. **Pass: bPressed**

### 4.4 Подключите события к функциям:
- **IA_A Pressed/Released → Handle A Input**
- **IA_D Pressed/Released → Handle D Input**

### 4.5 Настройте StrafeComponent:
1. **Выберите StrafeComponent в Details Panel**
2. **Назначьте параметры**:
   - **Strafe Distance**: `200.0`
   - **Strafe Duration**: `0.3`
   - **Strafe Cooldown**: `1.0`
   - **Space Double Press Window**: `0.5`
   - **Strafe Speed**: `400.0`
   - **Smoothness Factor**: `2.0`
   - **Strafe Curve**: `CF_StrafeMovement`

### 4.6 Сохраните BP_Player

---

## 🔧 **ШАГ 5: Импорт анимации strafing**

### 5.1 В Content Browser перейдите в:
```
Content/BackToZaraysk/Characters/Mannequins/Animations/Strafing/
```

### 5.2 Найдите файл `strafing.fbx`

### 5.3 Импортируйте анимацию:
1. **Выберите strafing.fbx**
2. **Нажмите Import**
3. **Настройте параметры**:
   - **Skeleton**: `SK_Mannequin`
   - **Import Translation**: `(0, 0, 0)`
   - **Import Rotation**: `(0, 0, 0)`
   - **Import Scale**: `1.0`
4. **Нажмите Import**

---

## 🎮 **ШАГ 6: Тестирование**

### 6.1 Соберите проект:
```bash
.\build_ue53.bat
```

### 6.2 Запустите игру

### 6.3 Тестируйте стрейф:
- **A + Space(2x)** → Стрейф влево
- **D + Space(2x)** → Стрейф вправо

---

## 🔍 **Отладка**

В игре будут отображаться сообщения:
- **Cyan**: Информация о движении
- **Green**: Успешный стрейф
- **Red**: Ошибки
- **Yellow**: Прогресс анимации

---

## ✅ **Готово!**

После выполнения всех шагов система стрейфа будет полностью настроена и готова к использованию!


