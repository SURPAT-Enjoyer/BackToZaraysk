import unreal
import time

def diagnose_abp_player_issues():
    """Диагностирует проблемы в ABP_Player, приводящие к завязыванию ног в узел"""
    
    print("🔍 ДИАГНОСТИКА ПРОБЛЕМ ABP_PLAYER")
    print("=" * 50)
    print("Поиск ошибок, приводящих к завязыванию ног в узел...")
    
    try:
        # Ждем загрузки
        time.sleep(1)
        
        # Находим ABP_Player
        abp_paths = [
            "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player",
            "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player1",
            "/Game/Characters/Player/ABP_Player",
            "/Game/Blueprints/ABP_Player"
        ]
        
        abp_found = False
        for abp_path in abp_paths:
            if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
                print(f"✅ Найден ABP_Player: {abp_path}")
                
                anim_bp = unreal.load_asset(abp_path)
                if anim_bp:
                    print(f"📂 ABP_Player загружен: {abp_path}")
                    
                    # Диагностируем проблемы
                    diagnose_animation_blueprint(anim_bp)
                    abp_found = True
                    break
        
        if not abp_found:
            print("❌ ABP_Player не найден по стандартным путям")
            print("🔍 Поиск всех Animation Blueprint в проекте...")
            find_and_diagnose_all_abp()
            
    except Exception as e:
        print(f"❌ Ошибка диагностики: {e}")
        import traceback
        traceback.print_exc()

def find_and_diagnose_all_abp():
    """Находит и диагностирует все Animation Blueprint"""
    
    try:
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"📋 Найдено Animation Blueprint: {len(anim_bp_assets)}")
            
            for asset_data in anim_bp_assets:
                if "Player" in asset_data.asset_name or "Character" in asset_data.asset_name:
                    print(f"\n🎯 Анализ: {asset_data.asset_name}")
                    
                    anim_bp = unreal.load_asset(asset_data.asset_name)
                    if anim_bp:
                        diagnose_animation_blueprint(anim_bp)
        else:
            print("❌ Animation Blueprint не найдены")
            
    except Exception as e:
        print(f"❌ Ошибка поиска: {e}")

def diagnose_animation_blueprint(anim_bp):
    """Диагностирует проблемы в Animation Blueprint"""
    
    print("\n🔧 ДИАГНОСТИКА ANIMATION BLUEPRINT")
    print("-" * 40)
    
    # Проверяем переменные
    diagnose_variables(anim_bp)
    
    # Проверяем ноды
    diagnose_nodes(anim_bp)
    
    # Проверяем подключения
    diagnose_connections(anim_bp)
    
    # Проверяем AnimInstance класс
    diagnose_anim_instance_class(anim_bp)
    
    print("\n🎯 ДИАГНОСТИКА ЗАВЕРШЕНА")

def diagnose_variables(anim_bp):
    """Диагностирует переменные"""
    
    print("\n📝 ПРОВЕРКА ПЕРЕМЕННЫХ:")
    
    try:
        if hasattr(anim_bp, 'variables') and anim_bp.variables:
            existing_vars = [var.variable_name for var in anim_bp.variables]
            print(f"  Найдено переменных: {len(existing_vars)}")
            
            # Проверяем ИК переменные
            ik_vars = [
                "LeftFootEffectorLocation",
                "RightFootEffectorLocation",
                "LeftFootIKAlpha", 
                "RightFootIKAlpha"
            ]
            
            missing_ik_vars = []
            for var in ik_vars:
                if var in existing_vars:
                    print(f"  ✅ {var}")
                else:
                    print(f"  ❌ {var} - ОТСУТСТВУЕТ")
                    missing_ik_vars.append(var)
            
            if missing_ik_vars:
                print(f"\n⚠️ ОТСУТСТВУЮТ ИК ПЕРЕМЕННЫЕ: {missing_ik_vars}")
                print("   Это может приводить к нерабочей ИК!")
            
            # Проверяем дублирующиеся переменные
            duplicates = []
            for var in existing_vars:
                if existing_vars.count(var) > 1:
                    duplicates.append(var)
            
            if duplicates:
                print(f"\n⚠️ ДУБЛИРУЮЩИЕСЯ ПЕРЕМЕННЫЕ: {duplicates}")
                print("   Это может приводить к конфликтам!")
                
        else:
            print("  ❌ Нет доступа к переменным")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки переменных: {e}")

def diagnose_nodes(anim_bp):
    """Диагностирует ноды"""
    
    print("\n🔗 ПРОВЕРКА НОД:")
    
    try:
        if hasattr(anim_bp, 'anim_graph') and anim_bp.anim_graph:
            if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
                nodes = [node.node_name for node in anim_bp.anim_graph.nodes]
                print(f"  Найдено нод: {len(nodes)}")
                
                # Ищем ИК ноды
                ik_nodes = [node for node in nodes if "IK" in node or "TwoBone" in node]
                if ik_nodes:
                    print(f"  ✅ ИК ноды найдены: {ik_nodes}")
                    
                    # Проверяем каждый ИК нод
                    for node_name in ik_nodes:
                        diagnose_ik_node(anim_bp, node_name)
                else:
                    print("  ❌ ИК ноды не найдены!")
                    print("     Это основная причина нерабочей ИК!")
                
                # Ищем проблемные ноды
                problematic_nodes = [node for node in nodes if any(problem in node.lower() for problem in ["broken", "error", "invalid"])]
                if problematic_nodes:
                    print(f"  ⚠️ ПРОБЛЕМНЫЕ НОДЫ: {problematic_nodes}")
                
                # Ищем дублирующиеся ноды
                duplicates = []
                for node in nodes:
                    if nodes.count(node) > 1:
                        duplicates.append(node)
                
                if duplicates:
                    print(f"  ⚠️ ДУБЛИРУЮЩИЕСЯ НОДЫ: {duplicates}")
                    print("     Это может приводить к завязыванию ног в узел!")
                    
            else:
                print("  ❌ Нет доступа к нодам AnimGraph")
        else:
            print("  ❌ AnimGraph не найден")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки нод: {e}")

def diagnose_ik_node(anim_bp, node_name):
    """Диагностирует конкретный ИК нод"""
    
    print(f"\n  🔍 Анализ ИК нода: {node_name}")
    
    try:
        # Ищем нод по имени
        for node in anim_bp.anim_graph.nodes:
            if node.node_name == node_name:
                print(f"    Найден нод: {node_name}")
                
                # Проверяем тип нода
                if hasattr(node, 'node_class'):
                    print(f"    Тип: {node.node_class}")
                
                # Проверяем подключения
                if hasattr(node, 'pins'):
                    pins = [pin.pin_name for pin in node.pins]
                    print(f"    Пины: {pins}")
                    
                    # Проверяем подключенные пины
                    connected_pins = [pin for pin in node.pins if pin.linked_to]
                    if connected_pins:
                        print(f"    Подключенные пины: {[pin.pin_name for pin in connected_pins]}")
                    else:
                        print(f"    ⚠️ НЕТ ПОДКЛЮЧЕНИЙ!")
                        print(f"       Нод {node_name} не подключен - это причина нерабочей ИК!")
                
                break
        else:
            print(f"    ❌ Нод {node_name} не найден в графе")
            
    except Exception as e:
        print(f"    ❌ Ошибка анализа нода {node_name}: {e}")

def diagnose_connections(anim_bp):
    """Диагностирует подключения"""
    
    print("\n🔌 ПРОВЕРКА ПОДКЛЮЧЕНИЙ:")
    
    try:
        if hasattr(anim_bp, 'anim_graph') and anim_bp.anim_graph:
            if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
                total_connections = 0
                broken_connections = 0
                
                for node in anim_bp.anim_graph.nodes:
                    if hasattr(node, 'pins'):
                        for pin in node.pins:
                            if pin.linked_to:
                                total_connections += 1
                                # Проверяем целостность подключения
                                try:
                                    if not pin.linked_to:
                                        broken_connections += 1
                                except:
                                    broken_connections += 1
                
                print(f"  Всего подключений: {total_connections}")
                print(f"  Сломанных подключений: {broken_connections}")
                
                if broken_connections > 0:
                    print(f"  ⚠️ ОБНАРУЖЕНЫ СЛОМАННЫЕ ПОДКЛЮЧЕНИЯ!")
                    print(f"     Это может приводить к завязыванию ног в узел!")
                
                if total_connections == 0:
                    print(f"  ❌ НЕТ ПОДКЛЮЧЕНИЙ В ГРАФЕ!")
                    print(f"     Это основная причина нерабочей анимации!")
                    
    except Exception as e:
        print(f"  ❌ Ошибка проверки подключений: {e}")

def diagnose_anim_instance_class(anim_bp):
    """Диагностирует класс AnimInstance"""
    
    print("\n🎭 ПРОВЕРКА КЛАССА ANIMINSTANCE:")
    
    try:
        if hasattr(anim_bp, 'generated_class'):
            generated_class = anim_bp.generated_class
            if generated_class:
                class_name = generated_class.get_name()
                print(f"  Класс: {class_name}")
                
                # Проверяем, наследуется ли от нашего кастомного класса
                if "BTZBaseCharacterAnimInstance" in class_name:
                    print(f"  ✅ Использует кастомный AnimInstance")
                else:
                    print(f"  ⚠️ Использует стандартный AnimInstance")
                    print(f"     Рекомендуется использовать UBTZBaseCharacterAnimInstance")
            else:
                print(f"  ❌ Не удалось получить класс")
        else:
            print(f"  ❌ Нет доступа к generated_class")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки класса: {e}")

# Запускаем диагностику
diagnose_abp_player_issues()
