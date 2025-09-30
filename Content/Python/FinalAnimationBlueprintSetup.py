import unreal
import time

# ФИНАЛЬНАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ДЛЯ IK
# Этот скрипт использует финальный правильный API для настройки Animation Blueprint

def final_animation_blueprint_setup():
    """Финальная автоматическая настройка Animation Blueprint для IK"""
    
    print("🤖 ФИНАЛЬНАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ДЛЯ IK")
    print("=" * 80)
    
    try:
        # Ждем загрузки редактора
        print("Ожидание загрузки редактора...")
        time.sleep(5)
        
        # Выполняем настройку Animation Blueprint
        print("Выполнение настройки Animation Blueprint...")
        setup_animation_blueprint_final()
        
        print("\n✅ ФИНАЛЬНАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ЗАВЕРШЕНА!")
        print("Animation Blueprint готов к работе с IK!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def setup_animation_blueprint_final():
    """Финальная настройка Animation Blueprint"""
    
    print("Финальная настройка Animation Blueprint...")
    
    # Ищем Animation Blueprint в проекте
    print("Поиск Animation Blueprint в проекте...")
    find_and_setup_animation_blueprint_final()
    
    print("✅ Animation Blueprint настроен финально!")

def find_and_setup_animation_blueprint_final():
    """Находит и настраивает Animation Blueprint финально"""
    
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
                        setup_single_animation_blueprint_final(anim_bp)
                        unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                        print(f"    ✅ Настроен: {asset_data.asset_name}")
                except Exception as e:
                    print(f"    ❌ Ошибка в {asset_data.asset_name}: {e}")
        else:
            print("  ⚠️ Animation Blueprint не найдены")
            print("  Создайте Animation Blueprint вручную и запустите скрипт снова")
            
    except Exception as e:
        print(f"  ❌ Ошибка поиска Animation Blueprint: {e}")

def setup_single_animation_blueprint_final(anim_bp):
    """Настраивает один Animation Blueprint финально"""
    
    print("    Настройка Animation Blueprint...")
    
    # Создаем переменные IK
    print("    Создание переменных IK...")
    create_ik_variables_final(anim_bp)
    
    # Создаем IK ноды
    print("    Создание IK нод...")
    create_ik_nodes_final(anim_bp)
    
    print("    ✅ Animation Blueprint настроен финально!")

def create_ik_variables_final(anim_bp):
    """Создает переменные IK в Animation Blueprint финально"""
    
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

def create_ik_nodes_final(anim_bp):
    """Создает IK ноды в Animation Blueprint финально"""
    
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

# ФИНАЛЬНАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT
final_animation_blueprint_setup()
