import unreal

def quick_strafe_setup():
    """Быстрая настройка стрейфа с использованием существующих Input Actions"""
    
    print("=== БЫСТРАЯ НАСТРОЙКА СТРЕЙФА ===")
    
    # 1. Загружаем существующие Input Actions
    print("1. Загрузка существующих Input Actions...")
    
    ia_move = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/IA_Move.IA_Move")
    ia_jump = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/IA_Jump.IA_Jump")
    
    if not ia_move:
        print("❌ IA_Move не найден!")
        return
    
    if not ia_jump:
        print("❌ IA_Jump не найден!")
        return
    
    print("✓ IA_Move и IA_Jump найдены")
    
    # 2. Создаем Curve для стрейфа
    print("2. Создание Curve для стрейфа...")
    
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
        print("✓ Создана CF_StrafeMovement")
    
    # 3. Загружаем BP_Player
    print("3. Настройка BP_Player...")
    
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    print("✓ BP_Player найден")
    
    # 4. Проверяем переменные стрейфа
    print("4. Проверка переменных стрейфа...")
    
    # Проверяем, есть ли переменные стрейфа в Blueprint
    has_strafe_vars = False
    for variable in bp_player.variables:
        if variable.get_name() in ["StrafeDistance", "StrafeDuration", "StrafeCooldown"]:
            has_strafe_vars = True
            break
    
    if has_strafe_vars:
        print("✓ Переменные стрейфа уже настроены")
    else:
        print("⚠️ Переменные стрейфа нужно добавить вручную")
    
    print("\n=== ИНСТРУКЦИИ ДЛЯ РУЧНОЙ НАСТРОЙКИ ===")
    print("1. Откройте BP_Player")
    print("2. В Input Events добавьте:")
    print("   - IA_Move → Разделить на X и Y")
    print("   - IA_Jump → Handle Space Input (Pressed)")
    print("3. Создайте функции:")
    print("   - Handle A Input (Boolean) → Call Parent: Handle A Input")
    print("   - Handle D Input (Boolean) → Call Parent: Handle D Input")
    print("   - Handle Space Input (Boolean) → Call Parent: Handle Space Input")
    print("4. Назначьте CF_StrafeMovement в Strafe Curve")
    print("\n🎮 Готово! Тестируйте: A/D + Space(2x)")

def create_input_blueprint_nodes():
    """Создание Blueprint нодов для ввода"""
    
    print("\n=== СОЗДАНИЕ BLUEPRINT НОДОВ ===")
    
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    # Получаем Event Graph
    event_graph = None
    for graph in bp_player.function_graphs:
        if graph.get_name() == "Event Graph":
            event_graph = graph
            break
    
    if not event_graph:
        event_graph = bp_player.add_function_graph("Event Graph")
    
    print("✓ Event Graph найден")
    
    # Создаем ноды для обработки ввода
    try:
        # IA_Move нод
        move_event = event_graph.add_node(unreal.K2Node_Event(), 
                                        unreal.Vector2D(0, 0))
        move_event.set_from_function(unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/IA_Move.IA_Move"))
        
        print("✓ Создан нод IA_Move")
        
        # IA_Jump нод  
        jump_event = event_graph.add_node(unreal.K2Node_Event(),
                                        unreal.Vector2D(0, 100))
        jump_event.set_from_function(unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/IA_Jump.IA_Jump"))
        
        print("✓ Создан нод IA_Jump")
        
        unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
        
    except Exception as e:
        print(f"⚠️ Не удалось создать ноды автоматически: {e}")
        print("Настройте Input Events вручную в BP_Player")

if __name__ == "__main__":
    quick_strafe_setup()
    create_input_blueprint_nodes()

