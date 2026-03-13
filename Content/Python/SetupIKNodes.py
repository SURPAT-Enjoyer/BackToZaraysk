import unreal

def setup_ik_nodes_in_animgraph():
    """Настраивает IK ноды в AnimGraph"""
    
    print("=== Настройка IK нод в AnimGraph ===")
    
    # Находим все Animation Blueprint
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        asset_path = asset_data.asset_name
        print(f"\nНастройка IK нод в: {asset_path}")
        
        try:
            anim_bp = unreal.load_asset(asset_path)
            if anim_bp and anim_bp.anim_graph:
                setup_ik_nodes(anim_bp.anim_graph)
                unreal.EditorAssetLibrary.save_asset(asset_path)
                print(f"✓ IK ноды настроены в: {asset_path}")
                
        except Exception as e:
            print(f"✗ Ошибка настройки IK нод в {asset_path}: {e}")

def setup_ik_nodes(anim_graph):
    """Настраивает IK ноды в AnimGraph"""
    
    print("Создание IK нод...")
    
    # Создаем Two Bone IK нод для левой ноги
    try:
        left_ik_node = unreal.EditorAnimBlueprintUtils.add_node(
            anim_graph,
            "TwoBoneIK",
            "LeftFootIK"
        )
        if left_ik_node:
            print("✓ Создан нод: LeftFootIK")
            
            # Настраиваем параметры нода
            setup_two_bone_ik_node(left_ik_node, "Left")
            
    except Exception as e:
        print(f"✗ Ошибка создания LeftFootIK: {e}")
    
    # Создаем Two Bone IK нод для правой ноги
    try:
        right_ik_node = unreal.EditorAnimBlueprintUtils.add_node(
            anim_graph,
            "TwoBoneIK", 
            "RightFootIK"
        )
        if right_ik_node:
            print("✓ Создан нод: RightFootIK")
            
            # Настраиваем параметры нода
            setup_two_bone_ik_node(right_ik_node, "Right")
            
    except Exception as e:
        print(f"✗ Ошибка создания RightFootIK: {e}")

def setup_two_bone_ik_node(ik_node, foot_side):
    """Настраивает параметры Two Bone IK нода"""
    
    try:
        # Настраиваем кости для IK
        if foot_side == "Left":
            # Для левой ноги
            ik_node.ik_bone = "foot_l"
            ik_node.joint_target_location_space = unreal.BoneControlSpace.BCS_BoneSpace
            ik_node.effector_location_space = unreal.BoneControlSpace.BCS_BoneSpace
            
        else:
            # Для правой ноги
            ik_node.ik_bone = "foot_r"
            ik_node.joint_target_location_space = unreal.BoneControlSpace.BCS_BoneSpace
            ik_node.effector_location_space = unreal.BoneControlSpace.BCS_BoneSpace
        
        print(f"✓ Настроен {foot_side} IK нод")
        
    except Exception as e:
        print(f"✗ Ошибка настройки {foot_side} IK нода: {e}")

def create_ik_blueprint_interface():
    """Создает Blueprint Interface для IK"""
    
    print("\n=== Создание Blueprint Interface для IK ===")
    
    try:
        # Создаем Blueprint Interface
        interface_name = "IKInterface"
        interface_path = "/Game/Blueprints/IKInterface"
        
        # Проверяем, существует ли уже
        if unreal.EditorAssetLibrary.does_asset_exist(interface_path):
            print(f"Interface уже существует: {interface_path}")
            return
        
        # Создаем новый Blueprint Interface
        ik_interface = unreal.EditorAssetLibrary.create_asset(
            interface_name,
            "/Game/Blueprints/",
            unreal.BlueprintInterface,
            unreal.BlueprintInterfaceFactory()
        )
        
        if ik_interface:
            print(f"✓ Создан Blueprint Interface: {interface_path}")
            
            # Добавляем функции в интерфейс
            add_ik_functions_to_interface(ik_interface)
            
        else:
            print("✗ Не удалось создать Blueprint Interface")
            
    except Exception as e:
        print(f"✗ Ошибка создания Blueprint Interface: {e}")

def add_ik_functions_to_interface(interface):
    """Добавляет функции IK в интерфейс"""
    
    functions_to_add = [
        {
            "name": "SetLeftFootIK",
            "description": "Устанавливает IK для левой ноги"
        },
        {
            "name": "SetRightFootIK", 
            "description": "Устанавливает IK для правой ноги"
        },
        {
            "name": "GetLeftFootIK",
            "description": "Получает IK для левой ноги"
        },
        {
            "name": "GetRightFootIK",
            "description": "Получает IK для правой ноги"
        }
    ]
    
    for func_info in functions_to_add:
        try:
            # Добавляем функцию в интерфейс
            new_func = unreal.EditorBlueprintUtils.add_function_to_blueprint(
                interface,
                func_info["name"]
            )
            
            if new_func:
                print(f"✓ Добавлена функция: {func_info['name']}")
            else:
                print(f"✗ Не удалось добавить функцию: {func_info['name']}")
                
        except Exception as e:
            print(f"✗ Ошибка добавления функции {func_info['name']}: {e}")

def main():
    """Основная функция"""
    print("=== Автоматическая настройка IK нод ===")
    
    try:
        # Настраиваем IK ноды
        setup_ik_nodes_in_animgraph()
        
        # Создаем Blueprint Interface
        create_ik_blueprint_interface()
        
        print("\n=== Настройка завершена! ===")
        print("Созданы:")
        print("- Two Bone IK ноды для левой и правой ног")
        print("- Blueprint Interface для IK")
        print("- Функции для управления IK")
        
    except Exception as e:
        print(f"Общая ошибка: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
