import unreal

def quick_fix_strafe_ue53():
    """Быстрое исправление стрейфа для UE 5.3"""
    print("⚡ БЫСТРОЕ ИСПРАВЛЕНИЕ СТРЕЙФА ДЛЯ UE 5.3")
    print("=" * 50)
    
    try:
        # 1. Проверяем и создаем Input Actions
        print("\n🔧 1. ПРОВЕРКА INPUT ACTIONS:")
        fix_input_actions()
        
        # 2. Проверяем IMC_Default
        print("\n🔧 2. ПРОВЕРКА IMC_DEFAULT:")
        fix_input_mapping_context()
        
        # 3. Проверяем BP_Player
        print("\n🔧 3. ПРОВЕРКА BP_PLAYER:")
        fix_blueprint_player()
        
        # 4. Проверяем конфликты
        print("\n🔧 4. ПРОВЕРКА КОНФЛИКТОВ:")
        check_conflicts()
        
        print("\n✅ БЫСТРОЕ ИСПРАВЛЕНИЕ ЗАВЕРШЕНО!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")

def fix_input_actions():
    """Исправление Input Actions"""
    actions = ["IA_A", "IA_D", "IA_Space"]
    
    for action_name in actions:
        action_path = f"/Game/BackToZaraysk/Core/Input/Actions/Strafe/{action_name}.{action_name}"
        action = unreal.load_asset(action_path)
        
        if action:
            print(f"  ✓ {action_name} найден")
            # Обновляем настройки для UE 5.3
            action.value_type = unreal.InputActionValueType.DIGITAL
            action.consume_input = False
            action.trigger_when_paused = False
            unreal.EditorAssetLibrary.save_asset(action.get_path_name())
        else:
            print(f"  ❌ {action_name} НЕ НАЙДЕН!")
            print(f"    Создайте {action_name} вручную с Value Type: Digital (bool)")

def fix_input_mapping_context():
    """Исправление Input Mapping Context"""
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("  ❌ IMC_Default НЕ НАЙДЕН!")
        return
    
    print("  ✓ IMC_Default найден")
    
    # Проверяем маппинги
    strafe_mappings = 0
    for mapping in imc_default.mappings:
        if mapping.action and any("IA_A" in str(mapping.action) or "IA_D" in str(mapping.action) or "IA_Space" in str(mapping.action)):
            strafe_mappings += 1
    
    if strafe_mappings >= 3:
        print(f"  ✓ Найдено {strafe_mappings} маппингов для стрейфа")
    else:
        print(f"  ⚠️ Найдено только {strafe_mappings} маппингов для стрейфа")
        print("    Добавьте недостающие маппинги в IMC_Default")

def fix_blueprint_player():
    """Исправление Blueprint Player"""
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("  ❌ BP_Player НЕ НАЙДЕН!")
        return
    
    print("  ✓ BP_Player найден")
    
    # Проверяем StrafeComponent
    strafe_component = None
    for component in bp_player.components:
        if component.get_name() == "StrafeComponent":
            strafe_component = component
            break
    
    if strafe_component:
        print("  ✓ StrafeComponent найден")
        
        # Настраиваем параметры
        strafe_component.set_editor_property("StrafeDistance", 200.0)
        strafe_component.set_editor_property("StrafeDuration", 0.3)
        strafe_component.set_editor_property("StrafeCooldown", 1.0)
        strafe_component.set_editor_property("SpaceDoublePressWindow", 0.5)
        strafe_component.set_editor_property("StrafeSpeed", 400.0)
        strafe_component.set_editor_property("SmoothnessFactor", 2.0)
        
        # Назначаем кривую
        curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
        if curve:
            strafe_component.set_editor_property("StrafeCurve", curve)
            print("  ✓ Strafe Curve назначена")
        else:
            print("  ❌ Strafe Curve не найдена")
        
        unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
    else:
        print("  ❌ StrafeComponent НЕ НАЙДЕН!")
        print("    Добавьте StrafeComponent в BP_Player")

def check_conflicts():
    """Проверка конфликтов"""
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        return
    
    # Проверяем конфликты с Space Bar
    space_mappings = []
    for mapping in imc_default.mappings:
        if mapping.key == unreal.Key.Space_Bar:
            if mapping.action:
                action_name = mapping.action.get_name()
                space_mappings.append(action_name)
    
    if len(space_mappings) > 1:
        print(f"  ⚠️ КОНФЛИКТ: {len(space_mappings)} действий используют Space Bar:")
        for action in space_mappings:
            print(f"    - {action}")
        print("    РЕШЕНИЕ: Убедитесь, что только IA_Space использует Space Bar")
    elif len(space_mappings) == 1:
        print(f"  ✓ Только {space_mappings[0]} использует Space Bar")
    else:
        print("  ❌ НЕТ МАППИНГОВ НА SPACE BAR!")

def main():
    """Основная функция быстрого исправления"""
    print("🚀 Запуск быстрого исправления стрейфа для UE 5.3...")
    
    try:
        quick_fix_strafe_ue53()
        
        print("\n🎯 СЛЕДУЮЩИЕ ШАГИ:")
        print("1. Откройте BP_Player")
        print("2. Проверьте Event Graph на наличие Input Events")
        print("3. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function")
        print("4. Соберите проект: .\\build_ue53.bat")
        print("5. Протестируйте: A/D + Space(2x)")
        
        print("\n📋 ДОПОЛНИТЕЛЬНАЯ ПОМОЩЬ:")
        print("- Полное исправление: RunAllStrafeFixesUE53.py")
        print("- Детальные инструкции: FIX_JUMP_INSTEAD_OF_STRAFE.md")
        print("- Быстрая шпаргалка: QUICK_FIX_JUMP_ISSUE.md")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции FIX_JUMP_INSTEAD_OF_STRAFE.md")

if __name__ == "__main__":
    main()


