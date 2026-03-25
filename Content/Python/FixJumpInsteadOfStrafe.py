import unreal

def fix_jump_instead_of_strafe():
    """Автоматическое исправление проблемы с прыжками вместо стрейфа"""
    print("=== ИСПРАВЛЕНИЕ ПРОБЛЕМЫ С ПРЫЖКАМИ ===")
    
    # 1. Проверяем и создаем Input Actions
    print("\n1. ПРОВЕРКА INPUT ACTIONS:")
    check_and_fix_input_actions()
    
    # 2. Проверяем IMC_Default
    print("\n2. ПРОВЕРКА IMC_DEFAULT:")
    check_and_fix_input_mapping_context()
    
    # 3. Проверяем BP_Player
    print("\n3. ПРОВЕРКА BP_PLAYER:")
    check_and_fix_blueprint_player()
    
    # 4. Проверяем конфликты с Jump
    print("\n4. ПРОВЕРКА КОНФЛИКТОВ С JUMP:")
    check_jump_conflicts()
    
    print("\n✅ ИСПРАВЛЕНИЯ ЗАВЕРШЕНЫ!")

def check_and_fix_input_actions():
    """Проверка и исправление Input Actions"""
    actions = [
        ("IA_A", unreal.InputActionValueType.DIGITAL),
        ("IA_D", unreal.InputActionValueType.DIGITAL),
        ("IA_Space", unreal.InputActionValueType.DIGITAL)
    ]
    
    for action_name, value_type in actions:
        action_path = f"/Game/BackToZaraysk/Core/Input/Actions/Strafe/{action_name}.{action_name}"
        action = unreal.load_asset(action_path)
        
        if action:
            print(f"✓ {action_name} найден")
            
            # Проверяем настройки
            if action.value_type != value_type:
                action.value_type = value_type
                print(f"  ✓ Обновлен Value Type на Digital (bool)")
            
            if action.consume_input != False:
                action.consume_input = False
                print(f"  ✓ Consume Input установлен в false")
            
            if action.trigger_when_paused != False:
                action.trigger_when_paused = False
                print(f"  ✓ Trigger When Paused установлен в false")
            
            unreal.EditorAssetLibrary.save_asset(action.get_path_name())
        else:
            print(f"❌ {action_name} НЕ НАЙДЕН!")
            print(f"  Создайте {action_name} вручную с настройками:")
            print(f"    - Value Type: Digital (bool)")
            print(f"    - Consume Input: false")
            print(f"    - Trigger When Paused: false")

def check_and_fix_input_mapping_context():
    """Проверка и исправление Input Mapping Context"""
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("❌ IMC_Default не найден!")
        return
    
    print("✓ IMC_Default найден")
    
    # Проверяем маппинги для стрейфа
    strafe_mappings = []
    jump_mappings = []
    
    for mapping in imc_default.mappings:
        if mapping.action:
            action_name = mapping.action.get_name()
            if "IA_A" in action_name or "IA_D" in action_name or "IA_Space" in action_name:
                strafe_mappings.append((action_name, mapping.key))
            elif "Jump" in action_name or mapping.key == unreal.Key.Space_Bar:
                jump_mappings.append((action_name, mapping.key))
    
    print(f"  Найдено маппингов для стрейфа: {len(strafe_mappings)}")
    for action, key in strafe_mappings:
        print(f"    ✓ {action} -> {key}")
    
    print(f"  Найдено маппингов для прыжка: {len(jump_mappings)}")
    for action, key in jump_mappings:
        print(f"    ⚠️ {action} -> {key}")
    
    # Проверяем конфликты
    if any(key == unreal.Key.Space_Bar for _, key in jump_mappings):
        print("  ❌ КОНФЛИКТ: Jump использует Space Bar!")
        print("  РЕШЕНИЕ: Измените клавишу для Jump или удалите маппинг")
    
    # Проверяем отсутствующие маппинги
    required_mappings = [
        ("IA_A", unreal.Key.A),
        ("IA_D", unreal.Key.D),
        ("IA_Space", unreal.Key.Space_Bar)
    ]
    
    missing_mappings = []
    for required_action, required_key in required_mappings:
        if not any(action == required_action for action, _ in strafe_mappings):
            missing_mappings.append((required_action, required_key))
    
    if missing_mappings:
        print(f"  ❌ ОТСУТСТВУЮЩИЕ МАППИНГИ: {len(missing_mappings)}")
        for action, key in missing_mappings:
            print(f"    ❌ {action} -> {key}")
        print("  РЕШЕНИЕ: Добавьте маппинги вручную в IMC_Default")

def check_and_fix_blueprint_player():
    """Проверка и исправление Blueprint Player"""
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    print("✓ BP_Player найден")
    
    # Проверяем StrafeComponent
    strafe_component = None
    for component in bp_player.components:
        if component.get_name() == "StrafeComponent":
            strafe_component = component
            break
    
    if strafe_component:
        print("  ✓ StrafeComponent найден")
        
        # Проверяем настройки
        settings = {
            "StrafeDistance": 200.0,
            "StrafeDuration": 0.3,
            "StrafeCooldown": 1.0,
            "SpaceDoublePressWindow": 0.5,
            "StrafeSpeed": 400.0,
            "SmoothnessFactor": 2.0
        }
        
        for setting_name, expected_value in settings.items():
            current_value = strafe_component.get_editor_property(setting_name)
            if current_value != expected_value:
                strafe_component.set_editor_property(setting_name, expected_value)
                print(f"    ✓ {setting_name} обновлен: {current_value} -> {expected_value}")
        
        # Проверяем кривую
        curve = strafe_component.get_editor_property("StrafeCurve")
        if curve:
            print(f"    ✓ Strafe Curve назначена: {curve.get_name()}")
        else:
            print("    ❌ Strafe Curve НЕ НАЗНАЧЕНА!")
            print("    РЕШЕНИЕ: Назначьте CF_StrafeMovement в StrafeComponent")
    else:
        print("  ❌ StrafeComponent НЕ НАЙДЕН!")
        print("  РЕШЕНИЕ: Добавьте StrafeComponent в BP_Player")
    
    # Проверяем функции
    functions = ["Handle A Input", "Handle D Input", "Handle Space Input"]
    for func_name in functions:
        function_found = False
        for function in bp_player.functions:
            if function.get_name() == func_name:
                function_found = True
                break
        
        if function_found:
            print(f"  ✓ Функция {func_name} найдена")
        else:
            print(f"  ❌ Функция {func_name} НЕ НАЙДЕНА!")
            print(f"    РЕШЕНИЕ: Создайте функцию {func_name} вручную")
    
    # Проверяем Input Events
    input_events_found = 0
    for graph in bp_player.function_graphs:
        if "Input" in graph.get_name():
            input_events_found += 1
    
    if input_events_found > 0:
        print(f"  ✓ Найдено {input_events_found} Input Graph")
    else:
        print("  ❌ Input Events НЕ НАЙДЕНЫ!")
        print("  РЕШЕНИЕ: Добавьте Input Events вручную в Event Graph")

def check_jump_conflicts():
    """Проверка конфликтов с Jump"""
    print("Проверка конфликтов с Jump...")
    
    # Проверяем IMC_Default на конфликты
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if imc_default:
        space_mappings = []
        for mapping in imc_default.mappings:
            if mapping.key == unreal.Key.Space_Bar:
                action_name = mapping.action.get_name() if mapping.action else "Unknown"
                space_mappings.append(action_name)
        
        if len(space_mappings) > 1:
            print(f"  ⚠️ КОНФЛИКТ: {len(space_mappings)} действий используют Space Bar:")
            for action in space_mappings:
                print(f"    - {action}")
            print("  РЕШЕНИЕ: Убедитесь, что только IA_Space использует Space Bar")
        elif len(space_mappings) == 1:
            print(f"  ✓ Только {space_mappings[0]} использует Space Bar")
        else:
            print("  ❌ НЕТ МАППИНГОВ НА SPACE BAR!")
    
    print("  РЕКОМЕНДАЦИИ:")
    print("    1. Убедитесь, что только IA_Space использует Space Bar")
    print("    2. Если есть Jump Input, измените его клавишу")
    print("    3. Проверьте, что Input Events подключены правильно")

def main():
    """Основная функция исправления"""
    try:
        fix_jump_instead_of_strafe()
        
        print("\n🎯 СЛЕДУЮЩИЕ ШАГИ:")
        print("1. Откройте BP_Player")
        print("2. Проверьте Event Graph на наличие Input Events:")
        print("   - IA_A Started/Completed → Handle A Input")
        print("   - IA_D Started/Completed → Handle D Input")
        print("   - IA_Space Started → Handle Space Input")
        print("3. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function")
        print("4. Проверьте настройки StrafeComponent")
        print("5. Убедитесь, что нет конфликтов с Jump Input")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")

if __name__ == "__main__":
    main()


