#!/usr/bin/env python3
"""
Скрипт для исправления пути к мешу жилета в Data Asset
"""

import unreal

def fix_vest_mesh_path():
    """Исправляет путь к мешу жилета в Data Asset"""
    
    # Путь к Data Asset жилета
    vest_data_asset_path = "/Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest"
    
    # Правильный путь к мешу
    correct_mesh_path = "/Game/Insurgent_2/Mesh/Separate_Parts/SK_ChestRigSmall"
    
    try:
        # Загружаем Data Asset
        vest_data_asset = unreal.load_asset(vest_data_asset_path)
        
        if vest_data_asset is None:
            print(f"❌ Не удалось загрузить Data Asset: {vest_data_asset_path}")
            return False
        
        print(f"✅ Загружен Data Asset: {vest_data_asset.get_name()}")
        
        # Загружаем правильный меш
        correct_mesh = unreal.load_asset(correct_mesh_path)
        
        if correct_mesh is None:
            print(f"❌ Не удалось загрузить меш: {correct_mesh_path}")
            return False
        
        print(f"✅ Загружен меш: {correct_mesh.get_name()}")
        
        # Устанавливаем правильный меш
        vest_data_asset.equipped_mesh = correct_mesh
        
        # Сохраняем изменения
        unreal.EditorAssetLibrary.save_asset(vest_data_asset_path)
        
        print(f"✅ Путь к мешу исправлен и сохранен!")
        print(f"   Старый путь: /Game/insurgent_2/Characters/SK_ChestRigSmall")
        print(f"   Новый путь: {correct_mesh_path}")
        
        return True
        
    except Exception as e:
        print(f"❌ Ошибка при исправлении пути: {str(e)}")
        return False

def check_vest_data_asset():
    """Проверяет текущее состояние Data Asset жилета"""
    
    vest_data_asset_path = "/Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest"
    
    try:
        vest_data_asset = unreal.load_asset(vest_data_asset_path)
        
        if vest_data_asset is None:
            print(f"❌ Data Asset не найден: {vest_data_asset_path}")
            return
        
        print(f"📋 Информация о Data Asset жилета:")
        print(f"   Название: {vest_data_asset.get_name()}")
        print(f"   Display Name: {vest_data_asset.display_name}")
        print(f"   Equipment Slot: {vest_data_asset.equipment_slot}")
        print(f"   Has Additional Storage: {vest_data_asset.b_has_additional_storage}")
        print(f"   Additional Grid Size: {vest_data_asset.additional_grid_size}")
        
        if vest_data_asset.equipped_mesh:
            mesh_path = vest_data_asset.equipped_mesh.get_path_name()
            print(f"   Equipped Mesh: {mesh_path}")
        else:
            print(f"   Equipped Mesh: НЕ УСТАНОВЛЕН")
            
    except Exception as e:
        print(f"❌ Ошибка при проверке Data Asset: {str(e)}")

if __name__ == "__main__":
    print("🔧 Исправление пути к мешу жилета")
    print("=" * 50)
    
    # Сначала проверяем текущее состояние
    print("📋 Текущее состояние Data Asset:")
    check_vest_data_asset()
    print()
    
    # Исправляем путь
    print("🔧 Исправление пути к мешу:")
    success = fix_vest_mesh_path()
    print()
    
    # Проверяем результат
    if success:
        print("📋 Состояние после исправления:")
        check_vest_data_asset()
        print()
        print("✅ Исправление завершено успешно!")
    else:
        print("❌ Исправление не удалось!")


