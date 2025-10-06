import unreal

def diagnose_strafe_input_issue():
    """Диагностика проблемы с прыжками вместо стрейфа"""
    print("=== ДИАГНОСТИКА ПРОБЛЕМЫ С ПРЫЖКАМИ ===")
    
    # 1. Проверяем Input Actions
    print("\n1. ПРОВЕРКА INPUT ACTIONS:")
    input_actions = [
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_A.IA_A",
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_D.IA_D",
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_Space.IA_Space"
    ]
    
    for action_path in input_actions:
        action = unreal.load_asset(action_path)
        if action:
            print(f"✓ {action_path} - найден")
        else:
            print(f"❌ {action_path} - НЕ НАЙДЕН!")
    
    # 2. Проверяем IMC_Default
    print("\n2. ПРОВЕРКА IMC_DEFAULT:")
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if imc_default:
        print("✓ IMC_Default найден")
        
        # Проверяем наличие наших маппингов
        strafe_mappings = 0
        for mapping in imc_default.mappings:
            if mapping.action and any("IA_A" in str(mapping.action) or "IA_D" in str(mapping.action) or "IA_Space" in str(mapping.action)):
                strafe_mappings += 1
                print(f"  ✓ Найден маппинг: {mapping.action} -> {mapping.key}")
        
        if strafe_mappings == 0:
            print("  ❌ НЕТ МАППИНГОВ ДЛЯ СТРЕЙФА!")
    else:
        print("❌ IMC_Default НЕ НАЙДЕН!")
    
    # 3. Проверяем BP_Player
    print("\n3. ПРОВЕРКА BP_PLAYER:")
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if bp_player:
        print("✓ BP_Player найден")
        
        # Проверяем StrafeComponent
        strafe_component = None
        for component in bp_player.components:
            if component.get_name() == "StrafeComponent":
                strafe_component = component
                print("  ✓ StrafeComponent найден")
                
                # Проверяем настройки StrafeComponent
                print(f"    Strafe Distance: {strafe_component.get_editor_property('StrafeDistance')}")
                print(f"    Strafe Duration: {strafe_component.get_editor_property('StrafeDuration')}")
                print(f"    Strafe Cooldown: {strafe_component.get_editor_property('StrafeCooldown')}")
                print(f"    Space Double Press Window: {strafe_component.get_editor_property('SpaceDoublePressWindow')}")
                print(f"    Strafe Speed: {strafe_component.get_editor_property('StrafeSpeed')}")
                break
        
        if not strafe_component:
            print("  ❌ StrafeComponent НЕ НАЙДЕН!")
        
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
        
        # Проверяем Input Events
        input_events_found = 0
        for graph in bp_player.function_graphs:
            if "Input" in graph.get_name():
                input_events_found += 1
        
        if input_events_found > 0:
            print(f"  ✓ Найдено {input_events_found} Input Graph")
        else:
            print("  ❌ Input Events НЕ НАЙДЕНЫ!")
            
    else:
        print("❌ BP_Player НЕ НАЙДЕН!")
    
    print("\n=== ДИАГНОСТИКА ЗАВЕРШЕНА ===")

def check_common_input_issues():
    """Проверка частых проблем с Input"""
    print("\n=== ПРОВЕРКА ЧАСТЫХ ПРОБЛЕМ ===")
    
    print("\n🔍 ВОЗМОЖНЫЕ ПРИЧИНЫ ПРЫЖКОВ ВМЕСТО СТРЕЙФА:")
    print("1. ❌ Input Events не подключены к функциям Handle A/D/Space Input")
    print("2. ❌ Функции Handle A/D/Space Input не вызывают Parent Function")
    print("3. ❌ StrafeComponent не настроен правильно")
    print("4. ❌ Input Actions не настроены как Digital (bool)")
    print("5. ❌ IMC_Default не содержит правильных маппингов")
    print("6. ❌ Space Input конфликтует с Jump Input")
    
    print("\n🎯 РЕКОМЕНДАЦИИ ПО ИСПРАВЛЕНИЮ:")
    print("1. Проверьте, что IA_A/IA_D/IA_Space подключены к Handle A/D/Space Input")
    print("2. Убедитесь, что Handle функции вызывают Call Parent Function")
    print("3. Проверьте настройки StrafeComponent в BP_Player")
    print("4. Убедитесь, что Space Input не конфликтует с Jump")
    print("5. Проверьте, что все Input Actions имеют Value Type: Digital (bool)")

def main():
    """Основная функция диагностики"""
    try:
        diagnose_strafe_input_issue()
        check_common_input_issues()
        
        print("\n🚨 СРОЧНЫЕ ДЕЙСТВИЯ:")
        print("1. Откройте BP_Player")
        print("2. Проверьте Event Graph на наличие Input Events")
        print("3. Убедитесь, что все события подключены к правильным функциям")
        print("4. Проверьте, что функции вызывают Parent Function")
        print("5. Убедитесь, что StrafeComponent настроен правильно")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА ДИАГНОСТИКИ: {e}")

if __name__ == "__main__":
    main()


