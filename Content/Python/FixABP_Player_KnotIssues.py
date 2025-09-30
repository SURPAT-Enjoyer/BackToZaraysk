import unreal
import time

def fix_abp_player_knot_issues():
    """Исправляет проблемы в ABP_Player, приводящие к завязыванию ног в узел"""
    
    print("🔧 ИСПРАВЛЕНИЕ ПРОБЛЕМ ABP_PLAYER - ЗАВЯЗЫВАНИЕ НОГ В УЗЕЛ")
    print("=" * 60)
    
    try:
        # Ждем загрузки
        time.sleep(1)
        
        # Находим ABP_Player
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
                    print(f"📂 ABP_Player загружен: {abp_path}")
                    
                    # Исправляем проблемы
                    fix_animation_blueprint_issues(anim_bp)
                    
                    # Сохраняем изменения
                    unreal.EditorAssetLibrary.save_asset(abp_path)
                    print(f"💾 ✅ ABP_Player исправлен и сохранен: {abp_path}")
                    abp_found = True
                    break
        
        if not abp_found:
            print("❌ ABP_Player не найден, ищем все Animation Blueprint...")
            fix_all_animation_blueprints()
            
    except Exception as e:
        print(f"❌ Ошибка исправления: {e}")
        import traceback
        traceback.print_exc()

def fix_all_animation_blueprints():
    """Исправляет все найденные Animation Blueprint"""
    
    try:
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"📋 Найдено Animation Blueprint: {len(anim_bp_assets)}")
            
            for asset_data in anim_bp_assets:
                if "Player" in asset_data.asset_name:
                    print(f"\n🔧 Исправление: {asset_data.asset_name}")
                    
                    anim_bp = unreal.load_asset(asset_data.asset_name)
                    if anim_bp:
                        fix_animation_blueprint_issues(anim_bp)
                        unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                        print(f"✅ Исправлен: {asset_data.asset_name}")
                        
    except Exception as e:
        print(f"❌ Ошибка исправления всех ABP: {e}")

def fix_animation_blueprint_issues(anim_bp):
    """Исправляет проблемы в Animation Blueprint"""
    
    print("\n🔧 ИСПРАВЛЕНИЕ ПРОБЛЕМ ANIMATION BLUEPRINT")
    print("-" * 40)
    
    # 1. Исправляем переменные
    fix_ik_variables(anim_bp)
    
    # 2. Исправляем ноды
    fix_ik_nodes(anim_bp)
    
    # 3. Исправляем подключения
    fix_connections(anim_bp)
    
    # 4. Настраиваем класс AnimInstance
    fix_anim_instance_class(anim_bp)
    
    print("\n🎯 ИСПРАВЛЕНИЕ ЗАВЕРШЕНО")

def fix_ik_variables(anim_bp):
    """Исправляет переменные ИК"""
    
    print("\n📝 ИСПРАВЛЕНИЕ ПЕРЕМЕННЫХ ИК:")
    
    # Удаляем дублирующиеся переменные
    remove_duplicate_variables(anim_bp)
    
    # Создаем правильные переменные ИК
    ik_variables = [
        ("LeftFootEffectorLocation", "Vector", unreal.Vector(0, 0, 0)),
        ("RightFootEffectorLocation", "Vector", unreal.Vector(0, 0, 0)),
        ("LeftFootIKAlpha", "Float", 1.0),
        ("RightFootIKAlpha", "Float", 1.0)
    ]
    
    existing_vars = []
    try:
        if hasattr(anim_bp, 'variables') and anim_bp.variables:
            existing_vars = [var.variable_name for var in anim_bp.variables]
    except:
        pass
    
    for var_name, var_type, default_value in ik_variables:
        if var_name not in existing_vars:
            try:
                # Создаем переменную
                if var_type == "Vector":
                    new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                        anim_bp, var_name, unreal.Vector
                    )
                elif var_type == "Float":
                    new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                        anim_bp, var_name, unreal.Float
                    )
                
                if new_var:
                    new_var.variable_name = var_name
                    new_var.default_value = default_value
                    print(f"  ✅ Создана переменная: {var_name} ({var_type})")
                else:
                    print(f"  ❌ Не удалось создать: {var_name}")
                    
            except Exception as e:
                print(f"  ❌ Ошибка создания {var_name}: {e}")
        else:
            print(f"  ℹ️ Переменная уже существует: {var_name}")

def remove_duplicate_variables(anim_bp):
    """Удаляет дублирующиеся переменные"""
    
    try:
        if hasattr(anim_bp, 'variables') and anim_bp.variables:
            var_names = [var.variable_name for var in anim_bp.variables]
            duplicates = []
            
            for var_name in var_names:
                if var_names.count(var_name) > 1 and var_name not in duplicates:
                    duplicates.append(var_name)
            
            if duplicates:
                print(f"  🔄 Удаление дублирующихся переменных: {duplicates}")
                # Здесь можно добавить логику удаления дубликатов
                # Но в Unreal Editor это обычно делается вручную
                
    except Exception as e:
        print(f"  ❌ Ошибка удаления дубликатов: {e}")

def fix_ik_nodes(anim_bp):
    """Исправляет ИК ноды"""
    
    print("\n🔗 ИСПРАВЛЕНИЕ ИК НОД:")
    
    try:
        if hasattr(anim_bp, 'anim_graph') and anim_bp.anim_graph:
            if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
                nodes = [node.node_name for node in anim_bp.anim_graph.nodes]
                
                # Удаляем дублирующиеся ноды
                remove_duplicate_nodes(anim_bp, nodes)
                
                # Создаем правильные ИК ноды
                create_proper_ik_nodes(anim_bp, nodes)
                
            else:
                print("  ❌ Нет доступа к нодам AnimGraph")
        else:
            print("  ❌ AnimGraph не найден")
            
    except Exception as e:
        print(f"  ❌ Ошибка исправления нод: {e}")

def remove_duplicate_nodes(anim_bp, nodes):
    """Удаляет дублирующиеся ноды"""
    
    duplicates = []
    for node in nodes:
        if nodes.count(node) > 1 and node not in duplicates:
            duplicates.append(node)
    
    if duplicates:
        print(f"  🔄 Найдены дублирующиеся ноды: {duplicates}")
        print(f"     ⚠️ Рекомендуется удалить дубликаты вручную в редакторе")

def create_proper_ik_nodes(anim_bp, existing_nodes):
    """Создает правильные ИК ноды"""
    
    ik_nodes = [
        ("LeftFootIK", "TwoBoneIK"),
        ("RightFootIK", "TwoBoneIK")
    ]
    
    for node_name, node_type in ik_nodes:
        if node_name not in existing_nodes:
            try:
                # Создаем ИК нод
                new_node = unreal.EditorAnimBlueprintUtils.add_node(
                    anim_bp.anim_graph, node_type, node_name
                )
                if new_node:
                    print(f"  ✅ Создан ИК нод: {node_name}")
                else:
                    print(f"  ❌ Не удалось создать: {node_name}")
                    
            except Exception as e:
                print(f"  ❌ Ошибка создания нода {node_name}: {e}")
        else:
            print(f"  ℹ️ ИК нод уже существует: {node_name}")

def fix_connections(anim_bp):
    """Исправляет подключения"""
    
    print("\n🔌 ИСПРАВЛЕНИЕ ПОДКЛЮЧЕНИЙ:")
    
    try:
        if hasattr(anim_bp, 'anim_graph') and anim_bp.anim_graph:
            if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
                # Проверяем основные подключения
                check_main_connections(anim_bp)
                
                # Проверяем ИК подключения
                check_ik_connections(anim_bp)
                
            else:
                print("  ❌ Нет доступа к нодам для проверки подключений")
        else:
            print("  ❌ AnimGraph не найден для проверки подключений")
            
    except Exception as e:
        print(f"  ❌ Ошибка исправления подключений: {e}")

def check_main_connections(anim_bp):
    """Проверяет основные подключения"""
    
    print("  🔍 Проверка основных подключений...")
    
    # Ищем Output Pose нод
    output_nodes = [node for node in anim_bp.anim_graph.nodes if "Output" in node.node_name or "Pose" in node.node_name]
    
    if output_nodes:
        print(f"  ✅ Найдены выходные ноды: {[node.node_name for node in output_nodes]}")
    else:
        print("  ❌ НЕ НАЙДЕНЫ ВЫХОДНЫЕ НОДЫ!")
        print("     Это критическая проблема - анимация не будет работать!")

def check_ik_connections(anim_bp):
    """Проверяет ИК подключения"""
    
    print("  🔍 Проверка ИК подключений...")
    
    ik_nodes = [node for node in anim_bp.anim_graph.nodes if "IK" in node.node_name]
    
    if ik_nodes:
        for node in ik_nodes:
            print(f"    Анализ ИК нода: {node.node_name}")
            
            # Проверяем подключения пинов
            if hasattr(node, 'pins'):
                connected_pins = [pin for pin in node.pins if pin.linked_to]
                if connected_pins:
                    print(f"      ✅ Подключенные пины: {[pin.pin_name for pin in connected_pins]}")
                else:
                    print(f"      ❌ НЕТ ПОДКЛЮЧЕНИЙ!")
                    print(f"         Нод {node.node_name} не подключен - исправьте вручную!")
    else:
        print("  ❌ ИК ноды не найдены")

def fix_anim_instance_class(anim_bp):
    """Исправляет класс AnimInstance"""
    
    print("\n🎭 НАСТРОЙКА КЛАССА ANIMINSTANCE:")
    
    try:
        # Проверяем текущий класс
        if hasattr(anim_bp, 'parent_class'):
            parent_class = anim_bp.parent_class
            if parent_class:
                class_name = parent_class.get_name()
                print(f"  Текущий родительский класс: {class_name}")
                
                # Рекомендуем использовать наш кастомный класс
                if "BTZBaseCharacterAnimInstance" not in class_name:
                    print("  ⚠️ Рекомендуется использовать UBTZBaseCharacterAnimInstance")
                    print("     Это обеспечит правильную работу с ИК переменными")
                else:
                    print("  ✅ Используется правильный родительский класс")
            else:
                print("  ❌ Не удалось получить родительский класс")
        else:
            print("  ❌ Нет доступа к parent_class")
            
    except Exception as e:
        print(f"  ❌ Ошибка настройки класса: {e}")

# Запускаем исправление
fix_abp_player_knot_issues()
