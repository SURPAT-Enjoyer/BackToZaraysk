# Автоматическая настройка IK системы

## Описание
Этот набор скриптов автоматически настраивает систему инверсной кинематики (IK) для ног персонажа в Unreal Engine 5.

## Что делают скрипты

### 1. SimpleIKSetup.py
- Создает необходимые переменные в Animation Blueprint
- Переменные: `LeftFootEffectorLocation`, `RightFootEffectorLocation`, `LeftFootIKAlpha`, `RightFootIKAlpha`

### 2. RunInEditor.py
- Полная настройка IK системы
- Создание переменных и функций
- Проверка сокетов персонажа

### 3. SetupCompleteIK.py
- Комплексная настройка всей IK системы
- Создание Blueprint Interface
- Настройка персонажа

## Как использовать

### Способ 1: В консоли Unreal Editor
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для открытия консоли Python
3. Выполните команду:
```python
exec(open('Content/Python/SimpleIKSetup.py').read())
```

### Способ 2: Через Blueprint
1. Создайте Blueprint Actor
2. Добавьте Event BeginPlay
3. Выполните Python скрипт через Blueprint

### Способ 3: Через консоль
1. Откройте консоль в игре (`~`)
2. Выполните команду:
```
py Content/Python/SimpleIKSetup.py
```

## Создаваемые переменные

### В Animation Blueprint:
- **LeftFootEffectorLocation** (Vector) - Позиция эффектора левой ноги
- **RightFootEffectorLocation** (Vector) - Позиция эффектора правой ноги  
- **LeftFootIKAlpha** (Float) - Сила IK для левой ноги
- **RightFootIKAlpha** (Float) - Сила IK для правой ноги

## Следующие шаги после запуска скрипта

### 1. Настройка Animation Blueprint
1. Откройте Animation Blueprint персонажа
2. В AnimGraph добавьте **Two Bone IK** ноды
3. Подключите переменные к IK нодам:
   - `LeftFootEffectorLocation` → Effector Location (Left IK)
   - `RightFootEffectorLocation` → Effector Location (Right IK)
   - `LeftFootIKAlpha` → Alpha (Left IK)
   - `RightFootIKAlpha` → Alpha (Right IK)

### 2. Настройка костей для IK
Для каждой ноги настройте:
- **IKBone**: `foot_l` / `foot_r`
- **Joint Target**: `calf_l` / `calf_r` 
- **Effector Target**: `thigh_l` / `thigh_r`

### 3. Проверка сокетов
Убедитесь, что в скелете персонажа есть сокеты:
- `foot_l` - для левой ноги
- `foot_r` - для правой ноги

## Отладка

### Проверка работы IK
1. Запустите игру
2. Посмотрите на отладочные сообщения
3. Должны появиться сообщения:
   - "IK Final - Left: X.XX, Right: X.XX"
   - "IK Updated in ABP - Left: X.XX, Right: X.XX"

### Если IK не работает
1. Проверьте, что переменные созданы в Animation Blueprint
2. Убедитесь, что сокеты `foot_l` и `foot_r` существуют
3. Проверьте подключение переменных к IK нодам
4. Убедитесь, что персонаж стоит на земле

## Требования
- Unreal Engine 5.3+
- Python скрипты в папке `Content/Python/`
- Animation Blueprint для персонажа
- Скелет с сокетами для ног

## Поддержка
Если возникают проблемы:
1. Проверьте логи в консоли
2. Убедитесь, что все файлы на месте
3. Перезапустите редактор
4. Проверьте права доступа к файлам
