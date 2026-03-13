import unreal
import time

# АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ДЛЯ IK
# Этот скрипт автоматически настроит Animation Blueprint для IK системы

def auto_setup_animation_blueprint():
    """Автоматическая настройка Animation Blueprint для IK"""
    
    print("🤖 АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ДЛЯ IK")
    print("=" * 70)
    
    try:
        # Ждем загрузки редактора
        print("Ожидание загрузки редактора...")
        time.sleep(5)
        
        # Выполняем настройку Animation Blueprint
        print("Выполнение настройки Animation Blueprint...")
        setup_animation_blueprint_auto()
        
        print("\n✅ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ЗАВЕРШЕНА!")
        print("Animation Blueprint готов к работе с IK!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def setup_animation_blueprint_auto():
    """Автоматическая настройка Animation Blueprint"""
    
    print("Автоматическая настройка Animation Blueprint...")
    
    # Ищем Animation Blueprint в проекте
    print("Поиск Animation Blueprint в проекте...")
    find_and_setup_animation_blueprint()
    
    print("✅ Animation Blueprint настроен автоматически!")

def find_and_setup_animation_blueprint():
    """Находит и настраивает Animation Blueprint"""
    
    try:
        # Ищем все Animation Blueprint в проекте
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Создаем фильтр для поиска Animation Blueprint
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_names = ["AnimBlueprint"]
        anim_bp_filter.recursive_classes = True
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"  Найдено Animation Blueprint: {len(anim_bp_assets)}")
            
            for asset_data in anim_bp_assets:
                try:
                    anim_bp = unreal.load_asset(asset_data.asset_name)
                    if anim_bp:
                        print(f"    Обработка: {asset_data.asset_name}")
                        setup_single_animation_blueprint(anim_bp)
                        unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                        print(f"    ✅ Настроен: {asset_data.asset_name}")
                except Exception as e:
                    print(f"    ❌ Ошибка в {asset_data.asset_name}: {e}")
        else:
            print("  ⚠️ Animation Blueprint не найдены")
            print("  Создайте Animation Blueprint вручную и запустите скрипт снова")
            
    except Exception as e:
        print(f"  ❌ Ошибка поиска Animation Blueprint: {e}")

def setup_single_animation_blueprint(anim_bp):
    """Настраивает один Animation Blueprint"""
    
    print("    Настройка Animation Blueprint...")
    
    # Создаем переменные IK
    print("    Создание переменных IK...")
    create_ik_variables_in_abp(anim_bp)
    
    # Создаем IK ноды
    print("    Создание IK нод...")
    create_ik_nodes_in_abp(anim_bp)
    
    # Подключаем переменные к нодам
    print("    Подключение переменных к нодам...")
    connect_variables_to_nodes(anim_bp)
    
    # Настраиваем кости для IK
    print("    Настройка костей для IK...")
    setup_ik_bones(anim_bp)
    
    print("    ✅ Animation Blueprint настроен!")

def create_ik_variables_in_abp(anim_bp):
    """Создает переменные IK в Animation Blueprint"""
    
    variables = [
        ("LeftFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("RightFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("LeftFootIKAlpha", unreal.Float, 1.0),
        ("RightFootIKAlpha", unreal.Float, 1.0)
    ]
    
    existing = [var.variable_name for var in anim_bp.variables]
    
    for name, type_class, default in variables:
        if name not in existing:
            try:
                new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                    anim_bp, name, type_class
                )
                if new_var:
                    new_var.variable_name = name
                    new_var.default_value = default
                    print(f"      ✅ Создана переменная: {name}")
            except Exception as e:
                print(f"      ❌ Ошибка создания {name}: {e}")

def create_ik_nodes_in_abp(anim_bp):
    """Создает IK ноды в Animation Blueprint"""
    
    if not anim_bp.anim_graph:
        print("      ⚠️ AnimGraph не найден")
        return
    
    nodes = [
        ("TwoBoneIK", "LeftFootIK"),
        ("TwoBoneIK", "RightFootIK")
    ]
    
    existing = [node.node_name for node in anim_bp.anim_graph.nodes]
    
    for node_type, node_name in nodes:
        if node_name not in existing:
            try:
                new_node = unreal.EditorAnimBlueprintUtils.add_node(
                    anim_bp.anim_graph, node_type, node_name
                )
                if new_node:
                    print(f"      ✅ Создан нод: {node_name}")
            except Exception as e:
                print(f"      ❌ Ошибка создания нода {node_name}: {e}")

def connect_variables_to_nodes(anim_bp):
    """Подключает переменные к IK нодам"""
    
    try:
        # Получаем переменные
        left_foot_effector = None
        right_foot_effector = None
        left_foot_alpha = None
        right_foot_alpha = None
        
        for var in anim_bp.variables:
            if var.variable_name == "LeftFootEffectorLocation":
                left_foot_effector = var
            elif var.variable_name == "RightFootEffectorLocation":
                right_foot_effector = var
            elif var.variable_name == "LeftFootIKAlpha":
                left_foot_alpha = var
            elif var.variable_name == "RightFootIKAlpha":
                right_foot_alpha = var
        
        # Получаем ноды
        left_foot_ik_node = None
        right_foot_ik_node = None
        
        for node in anim_bp.anim_graph.nodes:
            if node.node_name == "LeftFootIK":
                left_foot_ik_node = node
            elif node.node_name == "RightFootIK":
                right_foot_ik_node = node
        
        # Подключаем переменные к нодам
        if left_foot_ik_node and left_foot_effector and left_foot_alpha:
            try:
                # Подключаем Effector Location
                unreal.EditorAnimBlueprintUtils.add_pin_connection(
                    left_foot_effector, left_foot_ik_node, "Effector Location"
                )
                print("      ✅ Подключен LeftFootEffectorLocation к LeftFootIK")
                
                # Подключаем Alpha
                unreal.EditorAnimBlueprintUtils.add_pin_connection(
                    left_foot_alpha, left_foot_ik_node, "Alpha"
                )
                print("      ✅ Подключен LeftFootIKAlpha к LeftFootIK")
            except Exception as e:
                print(f"      ❌ Ошибка подключения LeftFootIK: {e}")
        
        if right_foot_ik_node and right_foot_effector and right_foot_alpha:
            try:
                # Подключаем Effector Location
                unreal.EditorAnimBlueprintUtils.add_pin_connection(
                    right_foot_effector, right_foot_ik_node, "Effector Location"
                )
                print("      ✅ Подключен RightFootEffectorLocation к RightFootIK")
                
                # Подключаем Alpha
                unreal.EditorAnimBlueprintUtils.add_pin_connection(
                    right_foot_alpha, right_foot_ik_node, "Alpha"
                )
                print("      ✅ Подключен RightFootIKAlpha к RightFootIK")
            except Exception as e:
                print(f"      ❌ Ошибка подключения RightFootIK: {e}")
                
    except Exception as e:
        print(f"      ❌ Ошибка подключения переменных: {e}")

def setup_ik_bones(anim_bp):
    """Настраивает кости для IK"""
    
    try:
        # Получаем ноды
        left_foot_ik_node = None
        right_foot_ik_node = None
        
        for node in anim_bp.anim_graph.nodes:
            if node.node_name == "LeftFootIK":
                left_foot_ik_node = node
            elif node.node_name == "RightFootIK":
                right_foot_ik_node = node
        
        # Настраиваем кости для LeftFootIK
        if left_foot_ik_node:
            try:
                # Устанавливаем IKBone
                unreal.EditorAnimBlueprintUtils.set_node_property(
                    left_foot_ik_node, "IKBone", "foot_l"
                )
                print("      ✅ Настроен IKBone для LeftFootIK: foot_l")
                
                # Устанавливаем Joint Target
                unreal.EditorAnimBlueprintUtils.set_node_property(
                    left_foot_ik_node, "Joint Target", "calf_l"
                )
                print("      ✅ Настроен Joint Target для LeftFootIK: calf_l")
                
                # Устанавливаем Effector Target
                unreal.EditorAnimBlueprintUtils.set_node_property(
                    left_foot_ik_node, "Effector Target", "thigh_l"
                )
                print("      ✅ Настроен Effector Target для LeftFootIK: thigh_l")
            except Exception as e:
                print(f"      ❌ Ошибка настройки LeftFootIK: {e}")
        
        # Настраиваем кости для RightFootIK
        if right_foot_ik_node:
            try:
                # Устанавливаем IKBone
                unreal.EditorAnimBlueprintUtils.set_node_property(
                    right_foot_ik_node, "IKBone", "foot_r"
                )
                print("      ✅ Настроен IKBone для RightFootIK: foot_r")
                
                # Устанавливаем Joint Target
                unreal.EditorAnimBlueprintUtils.set_node_property(
                    right_foot_ik_node, "Joint Target", "calf_r"
                )
                print("      ✅ Настроен Joint Target для RightFootIK: calf_r")
                
                # Устанавливаем Effector Target
                unreal.EditorAnimBlueprintUtils.set_node_property(
                    right_foot_ik_node, "Effector Target", "thigh_r"
                )
                print("      ✅ Настроен Effector Target для RightFootIK: thigh_r")
            except Exception as e:
                print(f"      ❌ Ошибка настройки RightFootIK: {e}")
                
    except Exception as e:
        print(f"      ❌ Ошибка настройки костей: {e}")

# АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT
auto_setup_animation_blueprint()
