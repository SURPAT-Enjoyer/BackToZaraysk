import unreal

def setup_blueprint_strafe():
    """Настройка Blueprint для стрейфа"""
    
    print("=== НАСТРОЙКА BLUEPRINT ДЛЯ СТРЕЙФА ===")
    
    # 1. Загружаем BP_Player
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    print("✓ BP_Player загружен")
    
    # 2. Получаем Event Graph
    event_graph = None
    for graph in bp_player.function_graphs:
        if graph.get_name() == "Event Graph":
            event_graph = graph
            break
    
    if not event_graph:
        event_graph = bp_player.add_function_graph("Event Graph")
        print("✓ Event Graph создан")
    else:
        print("✓ Event Graph найден")
    
    # 3. Создаем Input Events
    print("3. Создание Input Events...")
    
    # IA_Move Event
    try:
        move_event = event_graph.add_node(unreal.K2Node_Event(), unreal.Vector2D(0, 0))
        move_event.set_from_function(unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/IA_Move.IA_Move"))
        print("✓ IA_Move Event создан")
    except:
        print("⚠️ Не удалось создать IA_Move Event")
    
    # IA_Jump Event
    try:
        jump_event = event_graph.add_node(unreal.K2Node_Event(), unreal.Vector2D(0, 200))
        jump_event.set_from_function(unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/IA_Jump.IA_Jump"))
        print("✓ IA_Jump Event создан")
    except:
        print("⚠️ Не удалось создать IA_Jump Event")
    
    # 4. Создаем Custom Functions
    print("4. Создание Custom Functions...")
    
    # Handle A Input Function
    handle_a_func = bp_player.add_function_graph("Handle A Input")
    if handle_a_func:
        print("✓ Handle A Input Function создана")
    
    # Handle D Input Function  
    handle_d_func = bp_player.add_function_graph("Handle D Input")
    if handle_d_func:
        print("✓ Handle D Input Function создана")
    
    # Handle Space Input Function
    handle_space_func = bp_player.add_function_graph("Handle Space Input")
    if handle_space_func:
        print("✓ Handle Space Input Function создана")
    
    # 5. Сохраняем Blueprint
    unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
    print("✓ BP_Player сохранен")
    
    print("\n=== РУЧНАЯ НАСТРОЙКА В BLUEPRINT ===")
    print("1. Откройте BP_Player")
    print("2. В Event Graph:")
    print("   - IA_Move → Break Vector2D → Get X")
    print("   - IA_Jump → Handle Space Input")
    print("3. В Handle A Input:")
    print("   - Input: bPressed (Boolean)")
    print("   - Call Parent Function: Handle A Input")
    print("4. В Handle D Input:")
    print("   - Input: bPressed (Boolean)")
    print("   - Call Parent Function: Handle D Input")
    print("5. В Handle Space Input:")
    print("   - Input: bPressed (Boolean)")
    print("   - Call Parent Function: Handle Space Input")

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

def setup_player_character_properties():
    """Настройка свойств PlayerCharacter"""
    
    print("\n=== НАСТРОЙКА СВОЙСТВ PLAYER CHARACTER ===")
    
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    # Загружаем кривую
    strafe_curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
    
    if strafe_curve:
        # Назначаем кривую в Strafe Curve
        bp_player.set_editor_property("StrafeCurve", strafe_curve)
        print("✓ Strafe Curve назначена")
    
    # Настраиваем параметры стрейфа
    bp_player.set_editor_property("StrafeDistance", 200.0)
    bp_player.set_editor_property("StrafeDuration", 0.3)
    bp_player.set_editor_property("StrafeCooldown", 1.0)
    bp_player.set_editor_property("SpaceDoublePressWindow", 0.5)
    
    print("✓ Параметры стрейфа настроены")
    
    # Сохраняем
    unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
    print("✓ BP_Player обновлен")

if __name__ == "__main__":
    setup_blueprint_strafe()
    create_strafe_curve()
    setup_player_character_properties()
    
    print("\n🎮 НАСТРОЙКА ЗАВЕРШЕНА!")
    print("Теперь в BP_Player:")
    print("1. Подключите Input Events к функциям")
    print("2. Протестируйте: A/D + Space(2x)")

