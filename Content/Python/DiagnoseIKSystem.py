import unreal
import time

def diagnose_ik_system():
    """Диагностирует всю систему ИК для поиска проблем"""
    
    print("🔍 ДИАГНОСТИКА СИСТЕМЫ ИК")
    print("=" * 40)
    
    try:
        # Ждем загрузки
        time.sleep(1)
        
        # 1. Проверяем класс AnimInstance
        check_animinstance_class()
        
        # 2. Проверяем переменные в Animation Blueprint
        check_animation_blueprint_variables()
        
        # 3. Проверяем ИК ноды в Animation Blueprint
        check_animation_blueprint_nodes()
        
        # 4. Проверяем подключения
        check_animation_blueprint_connections()
        
        # 5. Проверяем сокеты персонажа
        check_character_sockets()
        
        print("\n🎯 ДИАГНОСТИКА ЗАВЕРШЕНА")
        
    except Exception as e:
        print(f"❌ Ошибка диагностики: {e}")
        import traceback
        traceback.print_exc()

def check_animinstance_class():
    """Проверяет класс AnimInstance"""
    
    print("\n🎭 ПРОВЕРКА КЛАССА ANIMINSTANCE:")
    
    try:
        abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
        
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            anim_bp = unreal.load_asset(abp_path)
            if anim_bp:
                if hasattr(anim_bp, 'parent_class') and anim_bp.parent_class:
                    class_name = anim_bp.parent_class.get_name()
                    print(f"  ✅ Класс: {class_name}")
                    
                    if "BTZBaseCharacterAnimInstance" in class_name:
                        print(f"  ✅ Используется правильный класс!")
                        return True
                    else:
                        print(f"  ❌ Неправильный класс!")
                        return False
                else:
                    print(f"  ❌ Не удалось получить класс")
                    return False
            else:
                print(f"  ❌ Не удалось загрузить Animation Blueprint")
                return False
        else:
            print(f"  ❌ ABP_Player не найден")
            return False
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки класса: {e}")
        return False

def check_animation_blueprint_variables():
    """Проверяет переменные в Animation Blueprint"""
    
    print("\n📝 ПРОВЕРКА ПЕРЕМЕННЫХ ANIMATION BLUEPRINT:")
    
    try:
        abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
        
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            anim_bp = unreal.load_asset(abp_path)
            if anim_bp:
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
                        print(f"\n🚨 ПРОБЛЕМА: Отсутствуют переменные ИК!")
                        print(f"   Это основная причина нерабочей ИК!")
                        print(f"\n💡 РЕШЕНИЕ:")
                        print(f"   1. Откройте Animation Blueprint")
                        print(f"   2. В My Blueprint → Variables добавьте:")
                        for var in missing_vars:
                            if "EffectorLocation" in var:
                                print(f"      - {var} (Vector, Default: 0,0,0)")
                            elif "IKAlpha" in var:
                                print(f"      - {var} (Float, Default: 1.0)")
                        print(f"   3. Сохраните файл")
                        return False
                    else:
                        print(f"  ✅ Все переменные ИК присутствуют!")
                        return True
                else:
                    print(f"  ❌ Нет доступа к переменным")
                    return False
            else:
                print(f"  ❌ Не удалось загрузить Animation Blueprint")
                return False
        else:
            print(f"  ❌ ABP_Player не найден")
            return False
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки переменных: {e}")
        return False

def check_animation_blueprint_nodes():
    """Проверяет ИК ноды в Animation Blueprint"""
    
    print("\n🔗 ПРОВЕРКА ИК НОД:")
    
    try:
        abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
        
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            anim_bp = unreal.load_asset(abp_path)
            if anim_bp:
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
                            return True
                        else:
                            print(f"  ❌ ИК НОДЫ НЕ НАЙДЕНЫ!")
                            print(f"     Это критическая проблема!")
                            print(f"\n💡 РЕШЕНИЕ:")
                            print(f"   1. Откройте Animation Blueprint")
                            print(f"   2. В AnimGraph добавьте TwoBoneIK ноды:")
                            print(f"      - LeftFootIK")
                            print(f"      - RightFootIK")
                            print(f"   3. Настройте кости и подключения")
                            return False
                    else:
                        print(f"  ❌ Нет доступа к нодам")
                        return False
                else:
                    print(f"  ❌ AnimGraph не найден")
                    return False
            else:
                print(f"  ❌ Не удалось загрузить Animation Blueprint")
                return False
        else:
            print(f"  ❌ ABP_Player не найден")
            return False
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки нод: {e}")
        return False

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
                            print(f"         Подключите переменную EffectorLocation")
                        if "Alpha" not in pin_names:
                            print(f"      ❌ Alpha НЕ ПОДКЛЮЧЕН!")
                            print(f"         Подключите переменную IKAlpha")
                    else:
                        print(f"      ❌ НЕТ ПОДКЛЮЧЕНИЙ!")
                        print(f"         Нод {node_name} не подключен!")
                
                break
        else:
            print(f"    ❌ Нод {node_name} не найден в графе")
            
    except Exception as e:
        print(f"    ❌ Ошибка анализа нода {node_name}: {e}")

def check_animation_blueprint_connections():
    """Проверяет подключения в Animation Blueprint"""
    
    print("\n🔌 ПРОВЕРКА ПОДКЛЮЧЕНИЙ:")
    
    try:
        abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
        
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            anim_bp = unreal.load_asset(abp_path)
            if anim_bp:
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
                            print(f"     Это критическая проблема!")
                            print(f"\n💡 РЕШЕНИЕ:")
                            print(f"   1. Подключите все ноды в правильном порядке")
                            print(f"   2. Убедитесь, что есть выходной нод (Output Pose)")
                            return False
                        else:
                            print(f"  ✅ Есть подключения в графе")
                            return True
                    else:
                        print(f"  ❌ Нет доступа к нодам")
                        return False
                else:
                    print(f"  ❌ AnimGraph не найден")
                    return False
            else:
                print(f"  ❌ Не удалось загрузить Animation Blueprint")
                return False
        else:
            print(f"  ❌ ABP_Player не найден")
            return False
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки подключений: {e}")
        return False

def check_character_sockets():
    """Проверяет сокеты персонажа"""
    
    print("\n🔌 ПРОВЕРКА СОКЕТОВ ПЕРСОНАЖА:")
    
    try:
        # Ищем скелет персонажа
        skeleton_paths = [
            "/Game/Characters/Mannequin_UE4/Meshes/SK_Mannequin",
            "/Game/Characters/Mannequins/Meshes/SK_Mannequin",
            "/Game/BackToZaraysk/Characters/Meshes/SK_Mannequin"
        ]
        
        skeleton_found = False
        for skeleton_path in skeleton_paths:
            if unreal.EditorAssetLibrary.does_asset_exist(skeleton_path):
                print(f"  ✅ Найден скелет: {skeleton_path}")
                skeleton_found = True
                break
        
        if not skeleton_found:
            print(f"  ⚠️ Скелет персонажа не найден по стандартным путям")
            print(f"     Проверьте, что сокеты foot_l и foot_r существуют")
        
        print(f"  💡 Убедитесь, что в скелете есть сокеты:")
        print(f"     - foot_l (левая нога)")
        print(f"     - foot_r (правая нога)")
        
        return True
        
    except Exception as e:
        print(f"  ❌ Ошибка проверки сокетов: {e}")
        return False

# Запускаем диагностику
diagnose_ik_system()
