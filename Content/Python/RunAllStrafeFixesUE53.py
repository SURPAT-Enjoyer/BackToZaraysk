import unreal

def run_all_strafe_fixes_ue53():
    """Запуск всех исправлений системы стрейфа для UE 5.3"""
    print("🚀 ЗАПУСК ВСЕХ ИСПРАВЛЕНИЙ СТРЕЙФА ДЛЯ UE 5.3")
    print("=" * 60)
    
    try:
        # 1. Автоматическое исправление
        print("\n🔧 ЭТАП 1: АВТОМАТИЧЕСКОЕ ИСПРАВЛЕНИЕ")
        print("-" * 40)
        
        # Импортируем и запускаем автоматическое исправление
        try:
            exec(open('Content/Python/AutoFixStrafeUE53.py').read())
        except Exception as e:
            print(f"❌ Ошибка автоматического исправления: {e}")
            print("Продолжаем с ручной проверкой...")
        
        # 2. Проверка Blueprint
        print("\n🔍 ЭТАП 2: ПРОВЕРКА BLUEPRINT")
        print("-" * 40)
        
        try:
            exec(open('Content/Python/FixBlueprintInputEventsUE53.py').read())
        except Exception as e:
            print(f"❌ Ошибка проверки Blueprint: {e}")
            print("Продолжаем с финальной проверкой...")
        
        # 3. Финальная проверка
        print("\n✅ ЭТАП 3: ФИНАЛЬНАЯ ПРОВЕРКА")
        print("-" * 40)
        
        final_check()
        
        # 4. Инструкции по тестированию
        print("\n🎮 ЭТАП 4: ИНСТРУКЦИИ ПО ТЕСТИРОВАНИЮ")
        print("-" * 40)
        
        testing_instructions()
        
        print("\n🎉 ВСЕ ИСПРАВЛЕНИЯ ЗАВЕРШЕНЫ!")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции FIX_JUMP_INSTEAD_OF_STRAFE.md")

def final_check():
    """Финальная проверка всех компонентов"""
    print("Выполняется финальная проверка...")
    
    # Проверяем Input Actions
    print("\n1. Проверка Input Actions:")
    input_actions_ok = True
    actions_to_check = ["IA_A", "IA_D", "IA_Space"]
    
    for action_name in actions_to_check:
        action_path = f"/Game/BackToZaraysk/Core/Input/Actions/Strafe/{action_name}.{action_name}"
        action = unreal.load_asset(action_path)
        if action:
            print(f"  ✓ {action_name} найден")
        else:
            print(f"  ❌ {action_name} НЕ НАЙДЕН!")
            input_actions_ok = False
    
    # Проверяем IMC_Default
    print("\n2. Проверка IMC_Default:")
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if imc_default:
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
    else:
        print("  ❌ IMC_Default НЕ НАЙДЕН!")
    
    # Проверяем BP_Player
    print("\n3. Проверка BP_Player:")
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if bp_player:
        print("  ✓ BP_Player найден")
        
        # Проверяем StrafeComponent
        strafe_component = None
        for component in bp_player.components:
            if component.get_name() == "StrafeComponent":
                strafe_component = component
                break
        
        if strafe_component:
            print("  ✓ StrafeComponent найден")
        else:
            print("  ❌ StrafeComponent НЕ НАЙДЕН!")
    else:
        print("  ❌ BP_Player НЕ НАЙДЕН!")
    
    # Проверяем кривую
    print("\n4. Проверка кривой:")
    curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
    if curve:
        print("  ✓ CF_StrafeMovement найдена")
    else:
        print("  ❌ CF_StrafeMovement НЕ НАЙДЕНА!")
    
    # Итоговая оценка
    print("\n📊 ИТОГОВАЯ ОЦЕНКА:")
    if input_actions_ok and imc_default and bp_player and curve:
        print("  ✅ ВСЕ ОСНОВНЫЕ КОМПОНЕНТЫ НАЙДЕНЫ")
        print("  🎯 СИСТЕМА ГОТОВА К ТЕСТИРОВАНИЮ")
    else:
        print("  ⚠️ ЕСТЬ ПРОБЛЕМЫ, ТРЕБУЕТСЯ РУЧНАЯ НАСТРОЙКА")
        print("  📋 Используйте FIX_JUMP_INSTEAD_OF_STRAFE.md")

def testing_instructions():
    """Инструкции по тестированию"""
    print("Инструкции по тестированию системы стрейфа:")
    
    print("\n🔧 ПЕРЕД ТЕСТИРОВАНИЕМ:")
    print("1. Соберите проект: .\\build_ue53.bat")
    print("2. Запустите игру")
    print("3. Откройте консоль (Tab или `)")
    
    print("\n🎮 ТЕСТИРОВАНИЕ УПРАВЛЕНИЯ:")
    print("1. Удерживайте клавишу A")
    print("2. Быстро нажмите Space дважды")
    print("3. Персонаж должен выполнить стрейф влево")
    print("4. Повторите с клавишей D для стрейфа вправо")
    
    print("\n📺 ПРОВЕРКА КОНСОЛИ:")
    print("В консоли должны появиться сообщения:")
    print("- Cyan: 'Strafe Animation: Starting from...'")
    print("- Green: 'Strafe: Starting Left/Right strafe!'")
    print("- Yellow: 'Strafe Progress: X.X/1.0'")
    print("- Green: 'Strafe: Completed!'")
    
    print("\n❌ ЕСЛИ НЕ РАБОТАЕТ:")
    print("1. Проверьте, что Input Events подключены в BP_Player")
    print("2. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function")
    print("3. Проверьте, что нет конфликтов с Jump Input")
    print("4. Убедитесь, что StrafeComponent настроен правильно")
    
    print("\n📋 ДОПОЛНИТЕЛЬНАЯ ПОМОЩЬ:")
    print("- Детальные инструкции: FIX_JUMP_INSTEAD_OF_STRAFE.md")
    print("- Быстрая шпаргалка: QUICK_FIX_JUMP_ISSUE.md")
    print("- Ручная настройка: MANUAL_INPUT_ANIMATION_SETUP_UE53.md")

def main():
    """Основная функция запуска всех исправлений"""
    print("🎯 ЗАПУСК ПОЛНОГО ИСПРАВЛЕНИЯ СИСТЕМЫ СТРЕЙФА ДЛЯ UE 5.3")
    print("Этот скрипт выполнит все необходимые исправления автоматически.")
    print("=" * 70)
    
    try:
        run_all_strafe_fixes_ue53()
        
        print("\n🎉 ИСПРАВЛЕНИЯ ЗАВЕРШЕНЫ!")
        print("Теперь вы можете протестировать систему стрейфа.")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции FIX_JUMP_INSTEAD_OF_STRAFE.md")

if __name__ == "__main__":
    main()


