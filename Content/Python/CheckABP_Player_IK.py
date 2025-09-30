import unreal
import time

def check_abp_player_ik():
    """Проверяет настройку ИК в ABP_Player"""
    
    print("🔍 ПРОВЕРКА НАСТРОЙКИ ИК В ABP_PLAYER")
    print("=" * 40)
    
    try:
        # Ждем загрузки
        time.sleep(1)
        
        # Находим Animation Blueprint
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"📋 Найдено Animation Blueprint: {len(anim_bp_assets)}")
            
            for asset_data in anim_bp_assets:
                if "ABP_Player" in asset_data.asset_name or "Player" in asset_data.asset_name:
                    print(f"\n🎯 Проверка ABP_Player: {asset_data.asset_name}")
                    
                    anim_bp = unreal.load_asset(asset_data.asset_name)
                    if anim_bp:
                        check_ik_variables(anim_bp)
                        check_ik_nodes(anim_bp)
                    else:
                        print("❌ Не удалось загрузить Animation Blueprint")
        else:
            print("❌ Animation Blueprint не найдены")
            
    except Exception as e:
        print(f"❌ Ошибка проверки: {e}")

def check_ik_variables(anim_bp):
    """Проверяет переменные ИК"""
    
    print("🔍 Проверка переменных ИК...")
    
    required_vars = [
        "LeftFootEffectorLocation",
        "RightFootEffectorLocation", 
        "LeftFootIKAlpha",
        "RightFootIKAlpha"
    ]
    
    existing_vars = []
    try:
        if hasattr(anim_bp, 'variables') and anim_bp.variables:
            existing_vars = [var.variable_name for var in anim_bp.variables]
    except:
        pass
    
    print(f"📝 Найдено переменных: {len(existing_vars)}")
    
    for var_name in required_vars:
        if var_name in existing_vars:
            print(f"✅ {var_name}")
        else:
            print(f"❌ {var_name} - ОТСУТСТВУЕТ")
    
    if all(var in existing_vars for var in required_vars):
        print("🎯 Все переменные ИК настроены правильно!")
    else:
        print("⚠️ Некоторые переменные ИК отсутствуют")

def check_ik_nodes(anim_bp):
    """Проверяет ИК ноды"""
    
    print("🔍 Проверка ИК нод...")
    
    try:
        if hasattr(anim_bp, 'anim_graph') and anim_bp.anim_graph:
            if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
                nodes = [node.node_name for node in anim_bp.anim_graph.nodes]
                print(f"📝 Найдено нод: {len(nodes)}")
                
                ik_nodes = [node for node in nodes if "IK" in node or "TwoBone" in node]
                if ik_nodes:
                    print("✅ Найдены ИК ноды:")
                    for node in ik_nodes:
                        print(f"  - {node}")
                else:
                    print("⚠️ ИК ноды не найдены")
            else:
                print("⚠️ Нет доступа к нодам AnimGraph")
        else:
            print("⚠️ AnimGraph не найден")
    except Exception as e:
        print(f"❌ Ошибка проверки нод: {e}")

# Запускаем проверку
check_abp_player_ik()
