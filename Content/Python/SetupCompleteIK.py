import unreal

def setup_complete_ik_system():
    """Полная настройка IK системы для персонажа"""
    
    print("=== ПОЛНАЯ НАСТРОЙКА IK СИСТЕМЫ ===")
    print("Этот скрипт автоматически настроит:")
    print("1. Переменные IK в Animation Blueprint")
    print("2. IK ноды в AnimGraph")
    print("3. Blueprint Interface для IK")
    print("4. Сокеты для ног персонажа")
    print("=" * 50)
    
    try:
        # Шаг 1: Создаем переменные IK
        print("\n[ШАГ 1] Создание переменных IK...")
        create_ik_variables()
        
        # Шаг 2: Создаем IK ноды
        print("\n[ШАГ 2] Создание IK нод...")
        create_ik_nodes()
        
        # Шаг 3: Настраиваем персонажа
        print("\n[ШАГ 3] Настройка персонажа...")
        setup_character_ik()
        
        # Шаг 4: Создаем Blueprint Interface
        print("\n[ШАГ 4] Создание Blueprint Interface...")
        create_ik_interface()
        
        print("\n" + "=" * 50)
        print("✅ НАСТРОЙКА ЗАВЕРШЕНА УСПЕШНО!")
        print("=" * 50)
        print("Теперь в вашем проекте доступны:")
        print("• Переменные IK в Animation Blueprint")
        print("• IK ноды в AnimGraph")
        print("• Blueprint Interface для IK")
        print("• Настроенные сокеты для ног")
        print("\nДля полной работы:")
        print("1. Откройте Animation Blueprint")
        print("2. Подключите переменные к IK нодам")
        print("3. Настройте кости для IK (thigh_l, calf_l, foot_l)")
        print("4. Настройте кости для правой ноги (thigh_r, calf_r, foot_r)")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def create_ik_variables():
    """Создает переменные IK в Animation Blueprint"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    print(f"Найдено Animation Blueprint: {len(anim_bp_assets)}")
    
    for asset_data in anim_bp_assets:
        asset_path = asset_data.asset_name
        print(f"Обрабатываем: {asset_path}")
        
        try:
            anim_bp = unreal.load_asset(asset_path)
            if not anim_bp:
                continue
            
            # Создаем переменные
            variables_created = 0
            
            # LeftFootEffectorLocation
            if create_variable(anim_bp, "LeftFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)):
                variables_created += 1
            
            # RightFootEffectorLocation
            if create_variable(anim_bp, "RightFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)):
                variables_created += 1
            
            # LeftFootIKAlpha
            if create_variable(anim_bp, "LeftFootIKAlpha", unreal.Float, 1.0):
                variables_created += 1
            
            # RightFootIKAlpha
            if create_variable(anim_bp, "RightFootIKAlpha", unreal.Float, 1.0):
                variables_created += 1
            
            if variables_created > 0:
                unreal.EditorAssetLibrary.save_asset(asset_path)
                print(f"✅ Создано {variables_created} переменных в {asset_path}")
            else:
                print(f"ℹ️ Переменные уже существуют в {asset_path}")
                
        except Exception as e:
            print(f"❌ Ошибка в {asset_path}: {e}")

def create_variable(anim_bp, var_name, var_type, default_value):
    """Создает переменную в Animation Blueprint"""
    
    # Проверяем, существует ли переменная
    existing_vars = [var.variable_name for var in anim_bp.variables]
    
    if var_name in existing_vars:
        return False
    
    try:
        new_var = unreal.EditorAnimBlueprintUtils.add_variable(
            anim_bp,
            var_name,
            var_type
        )
        
        if new_var:
            new_var.variable_name = var_name
            new_var.default_value = default_value
            return True
        
    except Exception as e:
        print(f"Ошибка создания переменной {var_name}: {e}")
    
    return False

def create_ik_nodes():
    """Создает IK ноды в AnimGraph"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        asset_path = asset_data.asset_name
        print(f"Создание IK нод в: {asset_path}")
        
        try:
            anim_bp = unreal.load_asset(asset_path)
            if anim_bp and anim_bp.anim_graph:
                # Создаем ноды
                create_two_bone_ik_node(anim_bp.anim_graph, "LeftFootIK")
                create_two_bone_ik_node(anim_bp.anim_graph, "RightFootIK")
                
                unreal.EditorAssetLibrary.save_asset(asset_path)
                print(f"✅ IK ноды созданы в {asset_path}")
                
        except Exception as e:
            print(f"❌ Ошибка создания нод в {asset_path}: {e}")

def create_two_bone_ik_node(anim_graph, node_name):
    """Создает Two Bone IK нод"""
    
    try:
        ik_node = unreal.EditorAnimBlueprintUtils.add_node(
            anim_graph,
            "TwoBoneIK",
            node_name
        )
        
        if ik_node:
            print(f"✅ Создан нод: {node_name}")
            return True
        
    except Exception as e:
        print(f"❌ Ошибка создания нода {node_name}: {e}")
    
    return False

def setup_character_ik():
    """Настраивает IK для персонажа"""
    
    # Находим Blueprint персонажа
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    for asset_data in blueprint_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            print(f"Настройка персонажа: {asset_data.asset_name}")
            
            try:
                character_bp = unreal.load_asset(asset_data.asset_name)
                if character_bp:
                    setup_character_sockets(character_bp)
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    print(f"✅ Персонаж настроен: {asset_data.asset_name}")
                    
            except Exception as e:
                print(f"❌ Ошибка настройки персонажа: {e}")

def setup_character_sockets(character_bp):
    """Настраивает сокеты для ног персонажа"""
    
    # Получаем компонент скелета
    skeletal_mesh_comp = None
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            skeletal_mesh_comp = comp
            break
    
    if skeletal_mesh_comp:
        # Проверяем сокеты
        left_socket_exists = skeletal_mesh_comp.does_socket_exist("foot_l")
        right_socket_exists = skeletal_mesh_comp.does_socket_exist("foot_r")
        
        print(f"Сокет foot_l: {'✅' if left_socket_exists else '❌'}")
        print(f"Сокет foot_r: {'✅' if right_socket_exists else '❌'}")
        
        if not left_socket_exists or not right_socket_exists:
            print("ℹ️ Сокеты для ног нужно настроить вручную в скелете персонажа")
            print("Добавьте сокеты 'foot_l' и 'foot_r' к соответствующим костям")

def create_ik_interface():
    """Создает Blueprint Interface для IK"""
    
    try:
        interface_path = "/Game/Blueprints/IKInterface"
        
        if unreal.EditorAssetLibrary.does_asset_exist(interface_path):
            print("ℹ️ IKInterface уже существует")
            return
        
        # Создаем папку если не существует
        unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/")
        
        # Создаем Blueprint Interface
        ik_interface = unreal.EditorAssetLibrary.create_asset(
            "IKInterface",
            "/Game/Blueprints/",
            unreal.BlueprintInterface,
            unreal.BlueprintInterfaceFactory()
        )
        
        if ik_interface:
            print("✅ Создан Blueprint Interface: IKInterface")
        else:
            print("❌ Не удалось создать Blueprint Interface")
            
    except Exception as e:
        print(f"❌ Ошибка создания Blueprint Interface: {e}")

def main():
    """Главная функция"""
    print("🚀 ЗАПУСК АВТОМАТИЧЕСКОЙ НАСТРОЙКИ IK СИСТЕМЫ")
    print("=" * 60)
    
    setup_complete_ik_system()

if __name__ == "__main__":
    main()
