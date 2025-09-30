import unreal
import time

def quick_diagnose_abp_player():
    """Быстрая диагностика проблем ABP_Player"""
    
    print("🔍 БЫСТРАЯ ДИАГНОСТИКА ABP_PLAYER")
    print("=" * 40)
    print("Поиск основных проблем, приводящих к завязыванию ног в узел...")
    
    try:
        time.sleep(1)
        
        # Ищем ABP_Player
        abp_paths = [
            "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player",
            "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player1"
        ]
        
        abp_found = False
        for abp_path in abp_paths:
            if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
                print(f"✅ Найден ABP_Player: {abp_path}")
                
                anim_bp = unreal.load_asset(abp_path)
                if anim_bp:
                    print(f"📂 ABP_Player загружен")
                    
                    # Быстрая диагностика
                    quick_check_variables(anim_bp)
                    quick_check_nodes(anim_bp)
                    quick_check_connections(anim_bp)
                    
                    abp_found = True
                    break
        
        if not abp_found:
            print("❌ ABP_Player не найден по стандартным путям")
            find_all_player_abp()
            
        print("\n🎯 БЫСТРАЯ ДИАГНОСТИКА ЗАВЕРШЕНА")
        
    except Exception as e:
        print(f"❌ Ошибка диагностики: {e}")

def find_all_player_abp():
    """Находит все Animation Blueprint с Player"""
    
    try:
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            player_abps = [asset for asset in anim_bp_assets if "Player" in asset.asset_name]
            
            if player_abps:
                print(f"📋 Найдено Player Animation Blueprint: {len(player_abps)}")
                for asset in player_abps:
                    print(f"  - {asset.asset_name}")
            else:
                print("❌ Player Animation Blueprint не найдены")
        else:
            print("❌ Animation Blueprint не найдены в проекте")
            
    except Exception as e:
        print(f"❌ Ошибка поиска: {e}")

def quick_check_variables(anim_bp):
    """Быстрая проверка переменных"""
    
    print("\n📝 ПРОВЕРКА ПЕРЕМЕННЫХ:")
    
    try:
        if hasattr(anim_bp, 'variables') and anim_bp.variables:
            existing_vars = [var.variable_name for var in anim_bp.variables]
            print(f"  Всего переменных: {len(existing_vars)}")
            
            # Критические переменные ИК
            critical_vars = [
                "LeftFootEffectorLocation",
                "RightFootEffectorLocation",
                "LeftFootIKAlpha",
                "RightFootIKAlpha"
            ]
            
            missing_critical = []
            for var in critical_vars:
                if var in existing_vars:
                    print(f"  ✅ {var}")
                else:
                    print(f"  ❌ {var} - КРИТИЧЕСКАЯ ОТСУТСТВУЕТ!")
                    missing_critical.append(var)
            
            if missing_critical:
                print(f"\n🚨 КРИТИЧЕСКИЕ ПРОБЛЕМЫ:")
                print(f"   Отсутствуют переменные ИК: {missing_critical}")
                print(f"   Это основная причина нерабочей ИК и завязывания ног!")
            
            # Проверяем дубликаты
            duplicates = []
            for var in existing_vars:
                if existing_vars.count(var) > 1 and var not in duplicates:
                    duplicates.append(var)
            
            if duplicates:
                print(f"\n⚠️ ДУБЛИРУЮЩИЕСЯ ПЕРЕМЕННЫЕ:")
                print(f"   {duplicates}")
                print(f"   Это может приводить к конфликтам!")
                
        else:
            print("  ❌ Нет доступа к переменным")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки переменных: {e}")

def quick_check_nodes(anim_bp):
    """Быстрая проверка нод"""
    
    print("\n🔗 ПРОВЕРКА НОД:")
    
    try:
        if hasattr(anim_bp, 'anim_graph') and anim_bp.anim_graph:
            if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
                nodes = [node.node_name for node in anim_bp.anim_graph.nodes]
                print(f"  Всего нод: {len(nodes)}")
                
                # Ищем ИК ноды
                ik_nodes = [node for node in nodes if "IK" in node or "TwoBone" in node]
                if ik_nodes:
                    print(f"  ✅ ИК ноды найдены: {ik_nodes}")
                else:
                    print(f"  ❌ ИК НОДЫ НЕ НАЙДЕНЫ!")
                    print(f"     Это критическая проблема - ИК не будет работать!")
                
                # Ищем выходные ноды
                output_nodes = [node for node in nodes if "Output" in node or "Pose" in node]
                if output_nodes:
                    print(f"  ✅ Выходные ноды: {output_nodes}")
                else:
                    print(f"  ❌ ВЫХОДНЫЕ НОДЫ НЕ НАЙДЕНЫ!")
                    print(f"     Это критическая проблема - анимация не будет работать!")
                
                # Проверяем дубликаты
                duplicates = []
                for node in nodes:
                    if nodes.count(node) > 1 and node not in duplicates:
                        duplicates.append(node)
                
                if duplicates:
                    print(f"\n⚠️ ДУБЛИРУЮЩИЕСЯ НОДЫ:")
                    print(f"   {duplicates}")
                    print(f"   Это может приводить к завязыванию ног в узел!")
                    
            else:
                print("  ❌ Нет доступа к нодам")
        else:
            print("  ❌ AnimGraph не найден")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки нод: {e}")

def quick_check_connections(anim_bp):
    """Быстрая проверка подключений"""
    
    print("\n🔌 ПРОВЕРКА ПОДКЛЮЧЕНИЙ:")
    
    try:
        if hasattr(anim_bp, 'anim_graph') and anim_bp.anim_graph:
            if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
                total_connections = 0
                ik_connections = 0
                
                for node in anim_bp.anim_graph.nodes:
                    if hasattr(node, 'pins'):
                        for pin in node.pins:
                            if pin.linked_to:
                                total_connections += 1
                                if "IK" in node.node_name:
                                    ik_connections += 1
                
                print(f"  Всего подключений: {total_connections}")
                print(f"  ИК подключений: {ik_connections}")
                
                if total_connections == 0:
                    print(f"  ❌ НЕТ ПОДКЛЮЧЕНИЙ В ГРАФЕ!")
                    print(f"     Это критическая проблема - анимация не работает!")
                
                if ik_connections == 0 and any("IK" in node.node_name for node in anim_bp.anim_graph.nodes):
                    print(f"  ❌ ИК НОДЫ НЕ ПОДКЛЮЧЕНЫ!")
                    print(f"     Это причина нерабочей ИК!")
                    
            else:
                print("  ❌ Нет доступа к нодам для проверки подключений")
        else:
            print("  ❌ AnimGraph не найден")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки подключений: {e}")

# Запускаем быструю диагностику
quick_diagnose_abp_player()
