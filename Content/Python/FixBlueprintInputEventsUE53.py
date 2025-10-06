import unreal

def fix_blueprint_input_events_ue53():
    """Исправление Input Events в Blueprint для UE 5.3"""
    print("=== ИСПРАВЛЕНИЕ INPUT EVENTS В BLUEPRINT ДЛЯ UE 5.3 ===")
    
    try:
        # 1. Проверяем BP_Player
        print("\n1. ПРОВЕРКА BP_PLAYER:")
        bp_player = check_blueprint_player()
        
        if not bp_player:
            print("❌ BP_Player не найден!")
            return
        
        # 2. Проверяем Input Events
        print("\n2. ПРОВЕРКА INPUT EVENTS:")
        check_input_events(bp_player)
        
        # 3. Проверяем функции Handle Input
        print("\n3. ПРОВЕРКА ФУНКЦИЙ HANDLE INPUT:")
        check_handle_functions(bp_player)
        
        # 4. Проверяем StrafeComponent
        print("\n4. ПРОВЕРКА STRAFE COMPONENT:")
        check_strafe_component(bp_player)
        
        print("\n✅ ПРОВЕРКА BLUEPRINT ЗАВЕРШЕНА!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА ПРОВЕРКИ BLUEPRINT: {e}")

def check_blueprint_player():
    """Проверка Blueprint Player"""
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if bp_player:
        print("✓ BP_Player найден")
        return bp_player
    else:
        print("❌ BP_Player не найден!")
        return None

def check_input_events(bp_player):
    """Проверка Input Events"""
    print("Проверка Input Events...")
    
    # Список необходимых Input Events
    required_events = [
        "IA_A Started",
        "IA_A Completed", 
        "IA_D Started",
        "IA_D Completed",
        "IA_Space Started"
    ]
    
    # Проверяем наличие Input Events
    found_events = []
    for graph in bp_player.function_graphs:
        if "Input" in graph.get_name():
            found_events.append(graph.get_name())
    
    print(f"Найдено Input Events: {len(found_events)}")
    for event in found_events:
        print(f"  ✓ {event}")
    
    # Проверяем отсутствующие события
    missing_events = []
    for required_event in required_events:
        if not any(required_event in found_event for found_event in found_events):
            missing_events.append(required_event)
    
    if missing_events:
        print(f"\n❌ ОТСУТСТВУЮЩИЕ INPUT EVENTS: {len(missing_events)}")
        for event in missing_events:
            print(f"  ❌ {event}")
        
        print("\n🔧 ИНСТРУКЦИИ ПО СОЗДАНИЮ:")
        print("1. Откройте BP_Player")
        print("2. Перейдите в Event Graph")
        print("3. Правый клик → введите имя события")
        print("4. Выберите 'Add Event for [Action]' → [Started/Completed]")
        print("5. Подключите к соответствующей функции Handle Input")
    else:
        print("✓ Все необходимые Input Events найдены")

def check_handle_functions(bp_player):
    """Проверка функций Handle Input"""
    print("Проверка функций Handle Input...")
    
    # Список необходимых функций
    required_functions = [
        "Handle A Input",
        "Handle D Input", 
        "Handle Space Input"
    ]
    
    # Проверяем наличие функций
    found_functions = []
    for function in bp_player.functions:
        if "Handle" in function.get_name() and "Input" in function.get_name():
            found_functions.append(function.get_name())
    
    print(f"Найдено Handle функций: {len(found_functions)}")
    for func in found_functions:
        print(f"  ✓ {func}")
    
    # Проверяем отсутствующие функции
    missing_functions = []
    for required_func in required_functions:
        if not any(required_func in found_func for found_func in found_functions):
            missing_functions.append(required_func)
    
    if missing_functions:
        print(f"\n❌ ОТСУТСТВУЮЩИЕ ФУНКЦИИ: {len(missing_functions)}")
        for func in missing_functions:
            print(f"  ❌ {func}")
        
        print("\n🔧 ИНСТРУКЦИИ ПО СОЗДАНИЮ:")
        print("1. Откройте BP_Player")
        print("2. Перейдите в My Blueprint → Functions")
        print("3. Нажмите + для добавления функции")
        print("4. Назовите функцию (например, 'Handle A Input')")
        print("5. Добавьте параметр: bPressed (Boolean)")
        print("6. Добавьте Call Parent Function: Handle A Input")
        print("7. Подключите параметр bPressed к Parent Function")
    else:
        print("✓ Все необходимые Handle функции найдены")

def check_strafe_component(bp_player):
    """Проверка StrafeComponent"""
    print("Проверка StrafeComponent...")
    
    # Ищем StrafeComponent
    strafe_component = None
    for component in bp_player.components:
        if component.get_name() == "StrafeComponent":
            strafe_component = component
            break
    
    if strafe_component:
        print("✓ StrafeComponent найден")
        
        # Проверяем настройки
        settings = {
            "StrafeDistance": 200.0,
            "StrafeDuration": 0.3,
            "StrafeCooldown": 1.0,
            "SpaceDoublePressWindow": 0.5,
            "StrafeSpeed": 400.0,
            "SmoothnessFactor": 2.0
        }
        
        print("Проверка настроек StrafeComponent:")
        for setting_name, expected_value in settings.items():
            current_value = strafe_component.get_editor_property(setting_name)
            if current_value == expected_value:
                print(f"  ✓ {setting_name}: {current_value}")
            else:
                print(f"  ⚠️ {setting_name}: {current_value} (ожидается: {expected_value})")
        
        # Проверяем кривую
        curve = strafe_component.get_editor_property("StrafeCurve")
        if curve:
            print(f"  ✓ Strafe Curve: {curve.get_name()}")
        else:
            print("  ❌ Strafe Curve не назначена")
            print("    РЕШЕНИЕ: Назначьте CF_StrafeMovement в StrafeComponent")
        
    else:
        print("❌ StrafeComponent не найден!")
        print("  РЕШЕНИЕ: Добавьте StrafeComponent в BP_Player")

def check_input_connections():
    """Проверка подключений Input Events"""
    print("\n5. ПРОВЕРКА ПОДКЛЮЧЕНИЙ INPUT EVENTS:")
    print("Проверьте вручную в BP_Player:")
    print("1. IA_A Started → Handle A Input (True)")
    print("2. IA_A Completed → Handle A Input (False)")
    print("3. IA_D Started → Handle D Input (True)")
    print("4. IA_D Completed → Handle D Input (False)")
    print("5. IA_Space Started → Handle Space Input (True)")
    
    print("\nВ каждой функции Handle Input должно быть:")
    print("1. Параметр: bPressed (Boolean)")
    print("2. Call Parent Function: Handle A/D/Space Input")
    print("3. Подключение: bPressed → Parent Function")

def main():
    """Основная функция проверки Blueprint"""
    print("🔍 Запуск проверки Blueprint Input Events для UE 5.3...")
    
    try:
        fix_blueprint_input_events_ue53()
        check_input_connections()
        
        print("\n🎯 РЕЗУЛЬТАТ ПРОВЕРКИ:")
        print("Если все проверки пройдены успешно:")
        print("1. Соберите проект: .\\build_ue53.bat")
        print("2. Запустите игру")
        print("3. Протестируйте: A/D + Space(2x)")
        
        print("\nЕсли есть ошибки:")
        print("1. Исправьте их вручную по инструкциям выше")
        print("2. Используйте FIX_JUMP_INSTEAD_OF_STRAFE.md для детальных инструкций")
        print("3. Используйте QUICK_FIX_JUMP_ISSUE.md для быстрого исправления")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции FIX_JUMP_INSTEAD_OF_STRAFE.md")

if __name__ == "__main__":
    main()


