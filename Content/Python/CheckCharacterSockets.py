# -*- coding: utf-8 -*-

import unreal

def check_character_sockets():
    """Проверить сокеты персонажа"""
    
    print("=== ПРОВЕРКА СОКЕТОВ ПЕРСОНАЖА ===")
    
    # Получаем все персонажи в мире
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        print("❌ Не удалось получить мир")
        return
    
    # Ищем персонажа
    character = None
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_class().get_name() == "PlayerCharacter":
            character = actor
            break
    
    if not character:
        print("❌ Персонаж не найден в мире")
        return
    
    print(f"✅ Найден персонаж: {character.get_name()}")
    
    # Получаем меш персонажа
    mesh_comp = None
    for comp in character.get_components_by_class(unreal.SkeletalMeshComponent):
        mesh_comp = comp
        break
    
    if not mesh_comp:
        print("❌ SkeletalMeshComponent не найден")
        return
    
    print(f"✅ Найден SkeletalMeshComponent: {mesh_comp.get_name()}")
    
    # Получаем скелет
    skeletal_mesh = mesh_comp.get_skeletal_mesh()
    if not skeletal_mesh:
        print("❌ SkeletalMesh не найден")
        return
    
    print(f"✅ SkeletalMesh: {skeletal_mesh.get_name()}")
    
    # Получаем все сокеты
    sockets = skeletal_mesh.get_sockets()
    print(f"📋 Найдено {len(sockets)} сокетов:")
    
    spine_sockets = []
    for socket in sockets:
        socket_name = socket.get_name()
        print(f"   - {socket_name}")
        
        # Ищем сокеты связанные с позвоночником
        if "spine" in socket_name.lower() or "chest" in socket_name.lower():
            spine_sockets.append(socket_name)
            print(f"     🎯 Подходящий сокет для жилета!")
    
    print(f"\n🎯 Найдено {len(spine_sockets)} подходящих сокетов для жилета:")
    for socket in spine_sockets:
        print(f"   - {socket}")
    
    # Проверяем конкретно spine_03
    spine_03_exists = any("spine_03" in socket.get_name() for socket in sockets)
    if spine_03_exists:
        print("✅ Сокет 'spine_03' найден!")
    else:
        print("❌ Сокет 'spine_03' НЕ найден!")
        
        # Предлагаем альтернативы
        if spine_sockets:
            print(f"💡 Рекомендуем использовать один из этих сокетов:")
            for socket in spine_sockets:
                print(f"   - {socket}")

def main():
    check_character_sockets()

if __name__ == "__main__":
    main()

