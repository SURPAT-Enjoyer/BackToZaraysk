import unreal

def test_strafe_component():
    """Тестирование StrafeComponent"""
    print("=== ТЕСТИРОВАНИЕ STRAFE COMPONENT ===")
    
    # Загружаем BP_Player
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    # Находим StrafeComponent
    strafe_component = None
    for component in bp_player.components:
        if component.get_name() == "StrafeComponent":
            strafe_component = component
            break
    
    if not strafe_component:
        print("❌ StrafeComponent не найден в BP_Player!")
        return
    
    print("✓ StrafeComponent найден")
    
    # Проверяем настройки
    print("\nНастройки StrafeComponent:")
    print(f"  Strafe Distance: {strafe_component.get_editor_property('StrafeDistance')}")
    print(f"  Strafe Duration: {strafe_component.get_editor_property('StrafeDuration')}")
    print(f"  Strafe Cooldown: {strafe_component.get_editor_property('StrafeCooldown')}")
    print(f"  Space Double Press Window: {strafe_component.get_editor_property('SpaceDoublePressWindow')}")
    print(f"  Strafe Speed: {strafe_component.get_editor_property('StrafeSpeed')}")
    print(f"  Smoothness Factor: {strafe_component.get_editor_property('SmoothnessFactor')}")
    
    # Проверяем кривую
    curve = strafe_component.get_editor_property('StrafeCurve')
    if curve:
        print(f"  Strafe Curve: {curve.get_name()} (найдена)")
    else:
        print("  Strafe Curve: НЕ НАЗНАЧЕНА!")
    
    print("\n✅ Тест завершен")

def test_input_actions():
    """Тестирование Input Actions"""
    print("\n=== ТЕСТИРОВАНИЕ INPUT ACTIONS ===")
    
    actions = [
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_A.IA_A",
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_D.IA_D",
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_Space.IA_Space"
    ]
    
    for action_path in actions:
        action = unreal.load_asset(action_path)
        if action:
            print(f"✓ {action.get_name()} - {action.value_type}")
        else:
            print(f"❌ {action_path} - НЕ НАЙДЕН")

def test_imc_default():
    """Тестирование IMC_Default"""
    print("\n=== ТЕСТИРОВАНИЕ IMC_DEFAULT ===")
    
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("❌ IMC_Default не найден!")
        return
    
    print(f"✓ IMC_Default найден")
    print(f"  Количество маппингов: {len(imc_default.mappings)}")
    
    # Ищем маппинги для стрейфа
    strafe_mappings = []
    for mapping in imc_default.mappings:
        if mapping.action:
            action_name = mapping.action.get_name()
            if "IA_A" in action_name or "IA_D" in action_name or "IA_Space" in action_name:
                strafe_mappings.append(f"{action_name} -> {mapping.key}")
    
    if strafe_mappings:
        print("  Маппинги для стрейфа:")
        for mapping in strafe_mappings:
            print(f"    ✓ {mapping}")
    else:
        print("  ❌ НЕТ МАППИНГОВ ДЛЯ СТРЕЙФА!")

def main():
    """Основная функция тестирования"""
    try:
        test_strafe_component()
        test_input_actions()
        test_imc_default()
        
        print("\n🎯 РЕЗУЛЬТАТ ТЕСТИРОВАНИЯ:")
        print("Если все тесты пройдены успешно, проблема в настройке Blueprint:")
        print("1. Проверьте подключение Input Events в BP_Player")
        print("2. Проверьте вызов Parent Functions в Handle A/D/Space Input")
        print("3. Проверьте настройку анимации в ABP_Player")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА ТЕСТИРОВАНИЯ: {e}")

if __name__ == "__main__":
    main()


