import unreal

def setup_strafe_input_system():
    """Полная настройка системы ввода для стрейфа"""
    
    print("=== НАСТРОЙКА СИСТЕМЫ ВВОДА ДЛЯ СТРЕЙФА ===")
    
    # 1. Создаем Input Actions для A, D, Space
    print("1. Создание Input Actions...")
    
    # IA_A
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
    
    # IA_D
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
    
    # IA_Space
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
    
    # 2. Настраиваем Input Mapping Context
    print("2. Настройка Input Mapping Context...")
    
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("❌ IMC_Default не найден!")
        return
    
    # Загружаем созданные Input Actions
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
    
    # 3. Настраиваем Character Blueprint
    print("3. Настройка Character Blueprint...")
    
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    # Получаем Graph для Input Events
    input_graph = None
    for graph in bp_player.function_graphs:
        if graph.get_name() == "Input Events":
            input_graph = graph
            break
    
    if not input_graph:
        # Создаем новый Graph для Input Events
        input_graph = bp_player.add_function_graph("Input Events")
    
    print("✓ Найден Graph Input Events")
    
    # 4. Создаем Curve для стрейфа
    print("4. Создание Curve для стрейфа...")
    
    strafe_curve = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="CF_StrafeMovement",
        package_path="/Game/BackToZaraysk/Core/Data/Curves",
        asset_class=unreal.CurveFloat,
        factory=unreal.CurveFloatFactory()
    )
    
    if strafe_curve:
        # Настраиваем кривую для плавного движения
        # Time 0: Value 0
        # Time 0.5: Value 1 (пик)
        # Time 1: Value 0
        strafe_curve.add_key(0.0, 0.0)
        strafe_curve.add_key(0.5, 1.0)
        strafe_curve.add_key(1.0, 0.0)
        unreal.EditorAssetLibrary.save_asset(strafe_curve.get_path_name())
        print("✓ Создана CF_StrafeMovement")
    
    print("\n=== НАСТРОЙКА ЗАВЕРШЕНА ===")
    print("Теперь нужно вручную подключить Input Events в BP_Player:")
    print("1. IA_A → Handle A Input (Pressed/Released)")
    print("2. IA_D → Handle D Input (Pressed/Released)")
    print("3. IA_Space → Handle Space Input (Pressed)")
    print("4. Назначить CF_StrafeMovement в Strafe Curve")

def import_strafing_animation():
    """Импорт анимации strafing"""
    
    print("\n=== ИМПОРТ АНИМАЦИИ STRAFING ===")
    
    # Путь к FBX файлу
    fbx_path = "Content/BackToZaraysk/Characters/Mannequins/Animations/Strafing/strafing.fbx"
    
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

if __name__ == "__main__":
    setup_strafe_input_system()
    import_strafing_animation()
    print("\n🎮 Система стрейфа настроена! Запустите игру и протестируйте:")
    print("A + Space(2x) → Стрейф влево")
    print("D + Space(2x) → Стрейф вправо")

