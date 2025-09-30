import unreal
import time

def fix_animinstance_ik():
    """Исправляет проблемы с ИК в AnimInstance"""
    
    print("🔧 ИСПРАВЛЕНИЕ ПРОБЛЕМ ИК В ANIMINSTANCE")
    print("=" * 50)
    
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
                    print(f"📂 ABP_Player загружен")
                    
                    # Проверяем и исправляем Animation Blueprint
                    check_and_fix_animation_blueprint(anim_bp)
                    
                    # Сохраняем изменения
                    unreal.EditorAssetLibrary.save_asset(abp_path)
                    print(f"💾 ✅ ABP_Player исправлен и сохранен")
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
                        check_and_fix_animation_blueprint(anim_bp)
                        unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                        print(f"✅ Исправлен: {asset_data.asset_name}")
                        
    except Exception as e:
        print(f"❌ Ошибка исправления всех ABP: {e}")

def check_and_fix_animation_blueprint(anim_bp):
    """Проверяет и исправляет Animation Blueprint"""
    
    print("\n🔧 ПРОВЕРКА И ИСПРАВЛЕНИЕ ANIMATION BLUEPRINT")
    print("-" * 40)
    
    # 1. Проверяем переменные ИК
    check_ik_variables(anim_bp)
    
    # 2. Проверяем ИК ноды
    check_ik_nodes(anim_bp)
    
    # 3. Проверяем подключения
    check_ik_connections(anim_bp)
    
    # 4. Проверяем класс AnimInstance
    check_anim_instance_class(anim_bp)
    
    print("\n🎯 ПРОВЕРКА ЗАВЕРШЕНА")

def check_ik_variables(anim_bp):
    """Проверяет переменные ИК"""
    
    print("\n📝 ПРОВЕРКА ПЕРЕМЕННЫХ ИК:")
    
    try:
        if hasattr(anim_bp, 'variables') and anim_bp.variables:
            existing_vars = [var.variable_name for var in anim_bp.variables]
            print(f"  Найдено переменных: {len(existing_vars)}")
            
            # Критические переменные ИК
            critical_vars = [
                "LeftFootEffectorLocation",
                "RightFootEffectorLocation",
                "LeftFootIKAlpha",
                "RightFootIKAlpha"
            ]
            
            missing_vars = []
            for var in critical_vars:
                if var in existing_vars:
                    print(f"  ✅ {var}")
                else:
                    print(f"  ❌ {var} - ОТСУТСТВУЕТ!")
                    missing_vars.append(var)
            
            if missing_vars:
                print(f"\n🚨 КРИТИЧЕСКИЕ ПРОБЛЕМЫ:")
                print(f"   Отсутствуют переменные ИК: {missing_vars}")
                print(f"   Это основная причина нерабочей ИК!")
                print(f"\n💡 РЕШЕНИЕ:")
                print(f"   1. Откройте Animation Blueprint")
                print(f"   2. В My Blueprint → Variables добавьте:")
                for var in missing_vars:
                    if "EffectorLocation" in var:
                        print(f"      - {var} (Vector, Default: 0,0,0)")
                    elif "IKAlpha" in var:
                        print(f"      - {var} (Float, Default: 1.0)")
                print(f"   3. Подключите переменные к соответствующим пинам ИК нодов")
                
        else:
            print("  ❌ Нет доступа к переменным")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки переменных: {e}")

def check_ik_nodes(anim_bp):
    """Проверяет ИК ноды"""
    
    print("\n🔗 ПРОВЕРКА ИК НОД:")
    
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
                        check_ik_node_details(anim_bp, node_name)
                else:
                    print(f"  ❌ ИК НОДЫ НЕ НАЙДЕНЫ!")
                    print(f"     Это критическая проблема - ИК не будет работать!")
                    print(f"\n💡 РЕШЕНИЕ:")
                    print(f"   1. Откройте Animation Blueprint")
                    print(f"   2. В AnimGraph добавьте TwoBoneIK ноды:")
                    print(f"      - LeftFootIK")
                    print(f"      - RightFootIK")
                    print(f"   3. Настройте кости:")
                    print(f"      LeftFootIK: IKBone=foot_l, Joint Target=calf_l, Effector Target=thigh_l")
                    print(f"      RightFootIK: IKBone=foot_r, Joint Target=calf_r, Effector Target=thigh_r")
                    
            else:
                print("  ❌ Нет доступа к нодам")
        else:
            print("  ❌ AnimGraph не найден")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки нод: {e}")

def check_ik_node_details(anim_bp, node_name):
    """Проверяет детали ИК нода"""
    
    print(f"\n  🔍 Анализ ИК нода: {node_name}")
    
    try:
        # Ищем нод по имени
        for node in anim_bp.anim_graph.nodes:
            if node.node_name == node_name:
                print(f"    Найден нод: {node_name}")
                
                # Проверяем подключения
                if hasattr(node, 'pins'):
                    connected_pins = [pin for pin in node.pins if pin.linked_to]
                    if connected_pins:
                        print(f"      ✅ Подключенные пины: {[pin.pin_name for pin in connected_pins]}")
                        
                        # Проверяем критические подключения
                        pin_names = [pin.pin_name for pin in connected_pins]
                        if "EffectorLocation" not in pin_names:
                            print(f"      ❌ EffectorLocation НЕ ПОДКЛЮЧЕН!")
                            print(f"         Подключите переменную EffectorLocation к этому пину")
                        if "Alpha" not in pin_names:
                            print(f"      ❌ Alpha НЕ ПОДКЛЮЧЕН!")
                            print(f"         Подключите переменную IKAlpha к этому пину")
                    else:
                        print(f"      ❌ НЕТ ПОДКЛЮЧЕНИЙ!")
                        print(f"         Нод {node_name} не подключен - исправьте вручную!")
                
                break
        else:
            print(f"    ❌ Нод {node_name} не найден в графе")
            
    except Exception as e:
        print(f"    ❌ Ошибка анализа нода {node_name}: {e}")

def check_ik_connections(anim_bp):
    """Проверяет подключения ИК"""
    
    print("\n🔌 ПРОВЕРКА ПОДКЛЮЧЕНИЙ ИК:")
    
    try:
        if hasattr(anim_bp, 'anim_graph') and anim_bp.anim_graph:
            if hasattr(anim_bp.anim_graph, 'nodes') and anim_bp.anim_graph.nodes:
                total_connections = 0
                
                for node in anim_bp.anim_graph.nodes:
                    if hasattr(node, 'pins'):
                        for pin in node.pins:
                            if pin.linked_to:
                                total_connections += 1
                
                print(f"  Всего подключений: {total_connections}")
                
                if total_connections == 0:
                    print(f"  ❌ НЕТ ПОДКЛЮЧЕНИЙ В ГРАФЕ!")
                    print(f"     Это критическая проблема - анимация не работает!")
                    print(f"\n💡 РЕШЕНИЕ:")
                    print(f"   1. Подключите все ноды в правильном порядке")
                    print(f"   2. Убедитесь, что есть выходной нод (Output Pose)")
                    print(f"   3. Проверьте, что ИК ноды подключены к основному графу")
                    
            else:
                print("  ❌ Нет доступа к нодам для проверки подключений")
        else:
            print("  ❌ AnimGraph не найден")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки подключений: {e}")

def check_anim_instance_class(anim_bp):
    """Проверяет класс AnimInstance"""
    
    print("\n🎭 ПРОВЕРКА КЛАССА ANIMINSTANCE:")
    
    try:
        if hasattr(anim_bp, 'parent_class'):
            parent_class = anim_bp.parent_class
            if parent_class:
                class_name = parent_class.get_name()
                print(f"  Родительский класс: {class_name}")
                
                if "BTZBaseCharacterAnimInstance" in class_name:
                    print(f"  ✅ Используется правильный кастомный AnimInstance")
                else:
                    print(f"  ⚠️ Используется стандартный AnimInstance")
                    print(f"     Рекомендуется использовать UBTZBaseCharacterAnimInstance")
                    print(f"\n💡 РЕШЕНИЕ:")
                    print(f"   1. В Class Settings → Parent Class")
                    print(f"   2. Установите UBTZBaseCharacterAnimInstance")
                    
            else:
                print("  ❌ Не удалось получить родительский класс")
        else:
            print("  ❌ Нет доступа к parent_class")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки класса: {e}")

# Запускаем исправление
fix_animinstance_ik()
