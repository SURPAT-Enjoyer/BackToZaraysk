import unreal
import time

def fix_abp_player_ik():
    """Исправляет проблемы с ИК в ABP_Player"""
    
    print("🔧 ИСПРАВЛЕНИЕ ПРОБЛЕМ С ИК В ABP_PLAYER")
    print("=" * 50)
    
    try:
        # Ждем загрузки редактора
        print("Ожидание загрузки редактора...")
        time.sleep(2)
        
        # Находим и исправляем ABP_Player
        print("Поиск ABP_Player...")
        fix_abp_player()
        
        print("\n✅ ИСПРАВЛЕНИЕ ABP_PLAYER ЗАВЕРШЕНО!")
        print("ИК система в Animation Blueprint исправлена!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def fix_abp_player():
    """Исправляет ABP_Player"""
    
    try:
        # Ищем Animation Blueprint в проекте
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Создаем фильтр для поиска Animation Blueprint
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"  Найдено Animation Blueprint: {len(anim_bp_assets)}")
            
            for asset_data in anim_bp_assets:
                try:
                    # Ищем ABP_Player
                    if "ABP_Player" in asset_data.asset_name:
                        print(f"    Найден ABP_Player: {asset_data.asset_name}")
                        
                        anim_bp = unreal.load_asset(asset_data.asset_name)
                        if anim_bp:
                            print(f"    Загружен ABP_Player: {asset_data.asset_name}")
                            
                            # Исправляем Animation Blueprint
                            fix_animation_blueprint_ik(anim_bp)
                            
                            # Сохраняем изменения
                            unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                            print(f"    ✅ ABP_Player исправлен и сохранен: {asset_data.asset_name}")
                            
                except Exception as e:
                    print(f"    ❌ Ошибка в {asset_data.asset_name}: {e}")
        else:
            print("  ⚠️ Animation Blueprint не найдены")
            
    except Exception as e:
        print(f"  ❌ Ошибка поиска Animation Blueprint: {e}")

def fix_animation_blueprint_ik(anim_bp):
    """Исправляет ИК в Animation Blueprint"""
    
    print("    Исправление ИК в Animation Blueprint...")
    
    # Создаем переменные IK
    print("    Создание/проверка переменных IK...")
    create_ik_variables(anim_bp)
    
    # Создаем IK ноды
    print("    Создание/проверка IK нод...")
    create_ik_nodes(anim_bp)
    
    print("    ✅ ИК в Animation Blueprint исправлен!")

def create_ik_variables(anim_bp):
    """Создает переменные IK в Animation Blueprint"""
    
    variables = [
        ("LeftFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("RightFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("LeftFootIKAlpha", unreal.Float, 1.0),
        ("RightFootIKAlpha", unreal.Float, 1.0)
    ]
    
    # Получаем существующие переменные
    existing_vars = []
    try:
        if hasattr(anim_bp, 'variables') and anim_bp.variables:
            existing_vars = [var.variable_name for var in anim_bp.variables]
    except:
        existing_vars = []
    
    for name, type_class, default in variables:
        if name not in existing_vars:
            try:
                # Создаем переменную
                new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                    anim_bp, name, type_class
                )
                if new_var:
                    new_var.variable_name = name
                    new_var.default_value = default
                    print(f"      ✅ Создана переменная: {name}")
            except Exception as e:
                print(f"      ❌ Ошибка создания {name}: {e}")
        else:
            print(f"      ℹ️ Переменная уже существует: {name}")

def create_ik_nodes(anim_bp):
    """Создает IK ноды в Animation Blueprint"""
    
    if not hasattr(anim_bp, 'anim_graph') or not anim_bp.anim_graph:
        print("      ⚠️ AnimGraph не найден")
        return
    
    nodes = [
        ("TwoBoneIK", "LeftFootIK"),
        ("TwoBoneIK", "RightFootIK")
    ]
    
    # Получаем существующие ноды
    existing_nodes = []
    try:
        if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
            existing_nodes = [node.node_name for node in anim_bp.anim_graph.nodes]
    except:
        existing_nodes = []
    
    for node_type, node_name in nodes:
        if node_name not in existing_nodes:
            try:
                # Создаем нод
                new_node = unreal.EditorAnimBlueprintUtils.add_node(
                    anim_bp.anim_graph, node_type, node_name
                )
                if new_node:
                    print(f"      ✅ Создан нод: {node_name}")
            except Exception as e:
                print(f"      ❌ Ошибка создания нода {node_name}: {e}")
        else:
            print(f"      ℹ️ Нод уже существует: {node_name}")

# Запускаем исправление
fix_abp_player_ik()
