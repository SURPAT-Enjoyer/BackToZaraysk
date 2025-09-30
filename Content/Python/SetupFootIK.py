import unreal

def setup_foot_ik():
    """Настраивает IK для ног в Animation Blueprint"""
    
    # Получаем все Animation Blueprint в проекте
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    print(f"Найдено Animation Blueprint: {len(anim_bp_assets)}")
    
    for asset_data in anim_bp_assets:
        asset_path = asset_data.asset_name
        print(f"Обрабатываем: {asset_path}")
        
        # Загружаем Animation Blueprint
        anim_bp = unreal.load_asset(asset_path)
        if not anim_bp:
            print(f"Не удалось загрузить: {asset_path}")
            continue
            
        # Получаем AnimGraph
        anim_graph = anim_bp.anim_graph
        if not anim_graph:
            print(f"AnimGraph не найден в: {asset_path}")
            continue
            
        # Создаем переменные для IK
        create_ik_variables(anim_bp)
        
        # Создаем IK ноды
        create_ik_nodes(anim_graph)
        
        print(f"IK настроен для: {asset_path}")

def create_ik_variables(anim_bp):
    """Создает переменные для IK в Animation Blueprint"""
    
    # Получаем список существующих переменных
    existing_vars = [var.variable_name for var in anim_bp.variables]
    
    # Создаем переменные для левой ноги
    if "LeftFootEffectorLocation" not in existing_vars:
        left_foot_var = unreal.EditorAnimBlueprintUtils.add_variable(
            anim_bp, 
            "LeftFootEffectorLocation", 
            unreal.Vector
        )
        left_foot_var.variable_name = "LeftFootEffectorLocation"
        left_foot_var.default_value = unreal.Vector(0, 0, 0)
        print("Создана переменная: LeftFootEffectorLocation")
    
    # Создаем переменные для правой ноги
    if "RightFootEffectorLocation" not in existing_vars:
        right_foot_var = unreal.EditorAnimBlueprintUtils.add_variable(
            anim_bp, 
            "RightFootEffectorLocation", 
            unreal.Vector
        )
        right_foot_var.variable_name = "RightFootEffectorLocation"
        right_foot_var.default_value = unreal.Vector(0, 0, 0)
        print("Создана переменная: RightFootEffectorLocation")
    
    # Создаем переменные для силы IK
    if "LeftFootIKAlpha" not in existing_vars:
        left_alpha_var = unreal.EditorAnimBlueprintUtils.add_variable(
            anim_bp, 
            "LeftFootIKAlpha", 
            unreal.Float
        )
        left_alpha_var.variable_name = "LeftFootIKAlpha"
        left_alpha_var.default_value = 1.0
        print("Создана переменная: LeftFootIKAlpha")
    
    if "RightFootIKAlpha" not in existing_vars:
        right_alpha_var = unreal.EditorAnimBlueprintUtils.add_variable(
            anim_bp, 
            "RightFootIKAlpha", 
            unreal.Float
        )
        right_alpha_var.variable_name = "RightFootIKAlpha"
        right_alpha_var.default_value = 1.0
        print("Создана переменная: RightFootIKAlpha")

def create_ik_nodes(anim_graph):
    """Создает IK ноды в AnimGraph"""
    
    print("Создание IK нод...")
    
    # Создаем Two Bone IK нод для левой ноги
    left_ik_node = unreal.EditorAnimBlueprintUtils.add_node(
        anim_graph, 
        "TwoBoneIK", 
        "LeftFootIK"
    )
    
    # Создаем Two Bone IK нод для правой ноги
    right_ik_node = unreal.EditorAnimBlueprintUtils.add_node(
        anim_graph, 
        "TwoBoneIK", 
        "RightFootIK"
    )
    
    print("IK ноды созданы")

def setup_character_ik():
    """Настраивает IK для персонажа"""
    
    # Находим BP_PlayerCharacter
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    character_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    for asset_data in character_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            print(f"Найден персонаж: {asset_data.asset_name}")
            
            # Загружаем Blueprint персонажа
            character_bp = unreal.load_asset(asset_data.asset_name)
            if character_bp:
                # Настраиваем IK параметры
                setup_character_ik_settings(character_bp)
                break

def setup_character_ik_settings(character_bp):
    """Настраивает параметры IK для персонажа"""
    
    print("Настройка параметров IK для персонажа...")
    
    # Получаем компонент скелета
    skeletal_mesh_comp = None
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            skeletal_mesh_comp = comp
            break
    
    if skeletal_mesh_comp:
        # Настраиваем сокеты для ног
        setup_foot_sockets(skeletal_mesh_comp)
        print("Сокеты для ног настроены")

def setup_foot_sockets(skeletal_mesh_comp):
    """Настраивает сокеты для ног"""
    
    # Проверяем существование сокетов
    left_socket_exists = skeletal_mesh_comp.does_socket_exist("foot_l")
    right_socket_exists = skeletal_mesh_comp.does_socket_exist("foot_r")
    
    print(f"Сокет foot_l существует: {left_socket_exists}")
    print(f"Сокет foot_r существует: {right_socket_exists}")
    
    if not left_socket_exists:
        print("Создание сокета foot_l...")
        # Создаем сокет для левой ноги
        left_socket = unreal.SkeletalMeshSocket()
        left_socket.socket_name = "foot_l"
        left_socket.bone_name = "foot_l"  # Или подходящая кость
        skeletal_mesh_comp.skeletal_mesh.sockets.append(left_socket)
    
    if not right_socket_exists:
        print("Создание сокета foot_r...")
        # Создаем сокет для правой ноги
        right_socket = unreal.SkeletalMeshSocket()
        right_socket.socket_name = "foot_r"
        right_socket.bone_name = "foot_r"  # Или подходящая кость
        skeletal_mesh_comp.skeletal_mesh.sockets.append(right_socket)

def main():
    """Основная функция"""
    print("=== Настройка IK для ног персонажа ===")
    
    try:
        # Настраиваем Animation Blueprint
        setup_foot_ik()
        
        # Настраиваем персонажа
        setup_character_ik()
        
        print("=== Настройка завершена ===")
        
    except Exception as e:
        print(f"Ошибка: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
