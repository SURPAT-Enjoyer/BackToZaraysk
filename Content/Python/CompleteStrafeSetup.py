import unreal

def create_input_actions():
    """Создание Input Actions для стрейфа"""
    print("=== СОЗДАНИЕ INPUT ACTIONS ===")
    
    # Создаем IA_A
    ia_a = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="IA_A",
        package_path="/Game/BackToZaraysk/Core/Input/Actions/Strafe",
        asset_class=unreal.InputAction,
        factory=unreal.InputActionFactory()
    )
    if ia_a:
        ia_a.value_type = unreal.InputActionValueType.BOOLEAN
        unreal.EditorAssetLibrary.save_asset(ia_a.get_path_name())
        print("✓ Создан IA_A")
    
    # Создаем IA_D
    ia_d = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="IA_D",
        package_path="/Game/BackToZaraysk/Core/Input/Actions/Strafe",
        asset_class=unreal.InputAction,
        factory=unreal.InputActionFactory()
    )
    if ia_d:
        ia_d.value_type = unreal.InputActionValueType.BOOLEAN
        unreal.EditorAssetLibrary.save_asset(ia_d.get_path_name())
        print("✓ Создан IA_D")
    
    # Создаем IA_Space
    ia_space = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="IA_Space",
        package_path="/Game/BackToZaraysk/Core/Input/Actions/Strafe",
        asset_class=unreal.InputAction,
        factory=unreal.InputActionFactory()
    )
    if ia_space:
        ia_space.value_type = unreal.InputActionValueType.BOOLEAN
        unreal.EditorAssetLibrary.save_asset(ia_space.get_path_name())
        print("✓ Создан IA_Space")

def setup_input_mapping():
    """Настройка Input Mapping Context"""
    print("\n=== НАСТРОЙКА INPUT MAPPING CONTEXT ===")
    
    # Загружаем IMC_Default
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("❌ IMC_Default не найден!")
        return
    
    # Загружаем Input Actions
    ia_a = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_A.IA_A")
    ia_d = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_D.IA_D")
    ia_space = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_Space.IA_Space")
    
    if not (ia_a and ia_d and ia_space):
        print("❌ Input Actions не найдены!")
        return
    
    # Создаем маппинги
    mappings = []
    
    # A Key mapping
    a_mapping = unreal.EnhancedActionKeyMapping()
    a_mapping.action = ia_a
    a_mapping.key = unreal.Key.A
    a_mapping.modifiers = []
    a_mapping.trigger = unreal.TriggerType.IMPULSE
    mappings.append(a_mapping)
    
    # D Key mapping
    d_mapping = unreal.EnhancedActionKeyMapping()
    d_mapping.action = ia_d
    d_mapping.key = unreal.Key.D
    d_mapping.modifiers = []
    d_mapping.trigger = unreal.TriggerType.IMPULSE
    mappings.append(d_mapping)
    
    # Space Key mapping
    space_mapping = unreal.EnhancedActionKeyMapping()
    space_mapping.action = ia_space
    space_mapping.key = unreal.Key.Space_Bar
    space_mapping.modifiers = []
    space_mapping.trigger = unreal.TriggerType.IMPULSE
    mappings.append(space_mapping)
    
    # Добавляем маппинги в IMC
    for mapping in mappings:
        imc_default.mappings.append(mapping)
    
    unreal.EditorAssetLibrary.save_asset(imc_default.get_path_name())
    print("✓ IMC_Default обновлен с новыми маппингами")

def create_strafe_curve():
    """Создание кривой для стрейфа"""
    print("\n=== СОЗДАНИЕ КРИВОЙ СТРЕЙФА ===")
    
    # Создаем папку для кривых если её нет
    unreal.EditorAssetLibrary.make_directory("/Game/BackToZaraysk/Core/Data/Curves")
    
    # Создаем Curve Float
    strafe_curve = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="CF_StrafeMovement",
        package_path="/Game/BackToZaraysk/Core/Data/Curves",
        asset_class=unreal.CurveFloat,
        factory=unreal.CurveFloatFactory()
    )
    
    if strafe_curve:
        # Настраиваем кривую для плавного движения
        strafe_curve.add_key(0.0, 0.0)
        strafe_curve.add_key(0.5, 1.0)
        strafe_curve.add_key(1.0, 0.0)
        
        unreal.EditorAssetLibrary.save_asset(strafe_curve.get_path_name())
        print("✓ CF_StrafeMovement создана")
        
        return strafe_curve
    
    return None

def setup_blueprint_events():
    """Настройка Blueprint Events"""
    print("\n=== НАСТРОЙКА BLUEPRINT EVENTS ===")
    
    # Загружаем BP_Player
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    # Загружаем кривую стрейфа
    strafe_curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
    
    # Настраиваем свойства StrafeComponent
    if strafe_curve:
        # Находим StrafeComponent в BP_Player
        for component in bp_player.components:
            if component.get_name() == "StrafeComponent":
                # Назначаем кривую
                component.set_editor_property("StrafeCurve", strafe_curve)
                print("✓ Strafe Curve назначена в StrafeComponent")
                break
    
    # Настраиваем параметры стрейфа
    bp_player.set_editor_property("StrafeDistance", 200.0)
    bp_player.set_editor_property("StrafeDuration", 0.3)
    bp_player.set_editor_property("StrafeCooldown", 1.0)
    
    print("✓ Параметры стрейфа настроены")
    
    # Сохраняем BP_Player
    unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
    print("✓ BP_Player обновлен")

def import_strafing_animation():
    """Импорт анимации strafing"""
    print("\n=== ИМПОРТ АНИМАЦИИ STRAFING ===")
    
    # Путь к FBX файлу
    fbx_path = "Content/BackToZaraysk/Characters/Mannequins/Animations/Strafing/strafing.fbx"
    
    # Проверяем существование файла
    if not unreal.EditorAssetLibrary.does_asset_exist("/Game/BackToZaraysk/Characters/Mannequins/Animations/Strafing/strafing"):
        print("❌ FBX файл strafing.fbx не найден!")
        print("Убедитесь, что файл находится в:")
        print("Content/BackToZaraysk/Characters/Mannequins/Animations/Strafing/strafing.fbx")
        return
    
    # Настройки импорта
    import_options = unreal.FbxImportUI()
    import_options.import_mesh = False
    import_options.import_animations = True
    import_options.import_materials = False
    import_options.import_textures = False
    import_options.automated_import_should_detect_type = False
    import_options.original_import_type = unreal.FBXImportType.FBXIT_ANIMATION
    
    # Настройки анимации
    import_options.anim_import_option = unreal.FbxAnimImportOption()
    import_options.anim_import_option.import_translation = unreal.Vector(0, 0, 0)
    import_options.anim_import_option.import_rotation = unreal.Rotator(0, 0, 0)
    import_options.anim_import_option.import_uniform_scale = 1.0
    
    # Найти скелет Mannequin
    skeleton = unreal.load_asset("/Game/BackToZaraysk/Characters/Mannequins/Meshes/SK_Mannequin.SK_Mannequin")
    if skeleton:
        import_options.skeleton = skeleton
        print("✓ Найден скелет SK_Mannequin")
    else:
        print("❌ Скелет SK_Mannequin не найден!")
        return
    
    # Импорт
    try:
        unreal.AssetToolsHelpers.get_asset_tools().import_asset(
            fbx_path,
            "/Game/BackToZaraysk/Characters/Mannequins/Animations/Strafing",
            import_options
        )
        print("✓ Анимация strafing импортирована")
    except Exception as e:
        print(f"❌ Ошибка импорта: {e}")

def create_blueprint_functions():
    """Создание функций в Blueprint"""
    print("\n=== СОЗДАНИЕ BLUEPRINT ФУНКЦИЙ ===")
    
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    # Создаем функции для обработки ввода
    functions_to_create = [
        "Handle A Input",
        "Handle D Input", 
        "Handle Space Input"
    ]
    
    for func_name in functions_to_create:
        # Проверяем, существует ли функция
        function_exists = False
        for function in bp_player.functions:
            if function.get_name() == func_name:
                function_exists = True
                break
        
        if not function_exists:
            # Создаем новую функцию
            new_function = bp_player.add_function(func_name)
            if new_function:
                print(f"✓ Создана функция: {func_name}")
            else:
                print(f"❌ Не удалось создать функцию: {func_name}")
        else:
            print(f"✓ Функция уже существует: {func_name}")
    
    # Сохраняем BP_Player
    unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())

def main():
    """Основная функция настройки"""
    print("🎯 ПОЛНАЯ НАСТРОЙКА СИСТЕМЫ СТРЕЙФА")
    print("=" * 50)
    
    try:
        create_input_actions()
        setup_input_mapping()
        create_strafe_curve()
        setup_blueprint_events()
        create_blueprint_functions()
        import_strafing_animation()
        
        print("\n" + "=" * 50)
        print("✅ НАСТРОЙКА ЗАВЕРШЕНА!")
        print("\n🎮 Теперь в BP_Player:")
        print("1. Подключите Input Events к функциям:")
        print("   - IA_A (Pressed/Released) → Handle A Input")
        print("   - IA_D (Pressed/Released) → Handle D Input")
        print("   - IA_Space (Pressed) → Handle Space Input")
        print("\n2. В функциях добавьте:")
        print("   - Call Parent Function: Handle A/D/Space Input")
        print("\n3. Тестируйте: A/D + Space(2x)")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        print("Попробуйте выполнить настройку вручную по инструкции")

if __name__ == "__main__":
    main()


