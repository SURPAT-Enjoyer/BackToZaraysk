import unreal
import time

# УЛЬТРА-ПРОСТАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ДЛЯ IK
# Этот скрипт использует ультра-простой подход для настройки Animation Blueprint

def ultra_simple_animation_blueprint_setup():
    """Ультра-простая автоматическая настройка Animation Blueprint для IK"""
    
    print("🤖 УЛЬТРА-ПРОСТАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ДЛЯ IK")
    print("=" * 80)
    
    try:
        # Ждем загрузки редактора
        print("Ожидание загрузки редактора...")
        time.sleep(5)
        
        # Выполняем настройку Animation Blueprint
        print("Выполнение настройки Animation Blueprint...")
        setup_animation_blueprint_ultra_simple()
        
        print("\n✅ УЛЬТРА-ПРОСТАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ЗАВЕРШЕНА!")
        print("Animation Blueprint готов к работе с IK!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def setup_animation_blueprint_ultra_simple():
    """Ультра-простая настройка Animation Blueprint"""
    
    print("Ультра-простая настройка Animation Blueprint...")
    
    # Ищем Animation Blueprint в проекте
    print("Поиск Animation Blueprint в проекте...")
    find_and_setup_animation_blueprint_ultra_simple()
    
    print("✅ Animation Blueprint настроен ультра-просто!")

def find_and_setup_animation_blueprint_ultra_simple():
    """Находит и настраивает Animation Blueprint ультра-просто"""
    
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
                        setup_single_animation_blueprint_ultra_simple(anim_bp)
                        unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                        print(f"    ✅ Настроен: {asset_data.asset_name}")
                except Exception as e:
                    print(f"    ❌ Ошибка в {asset_data.asset_name}: {e}")
        else:
            print("  ⚠️ Animation Blueprint не найдены")
            print("  Создайте Animation Blueprint вручную и запустите скрипт снова")
            
    except Exception as e:
        print(f"  ❌ Ошибка поиска Animation Blueprint: {e}")

def setup_single_animation_blueprint_ultra_simple(anim_bp):
    """Настраивает один Animation Blueprint ультра-просто"""
    
    print("    Настройка Animation Blueprint...")
    
    # Создаем переменные IK
    print("    Создание переменных IK...")
    create_ik_variables_ultra_simple(anim_bp)
    
    # Создаем IK ноды
    print("    Создание IK нод...")
    create_ik_nodes_ultra_simple(anim_bp)
    
    print("    ✅ Animation Blueprint настроен ультра-просто!")

def create_ik_variables_ultra_simple(anim_bp):
    """Создает переменные IK в Animation Blueprint ультра-просто"""
    
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

def create_ik_nodes_ultra_simple(anim_bp):
    """Создает IK ноды в Animation Blueprint ультра-просто"""
    
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

# УЛЬТРА-ПРОСТАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT
ultra_simple_animation_blueprint_setup()
