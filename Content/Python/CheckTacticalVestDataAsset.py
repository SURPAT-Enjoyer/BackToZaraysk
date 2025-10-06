# -*- coding: utf-8 -*-

import unreal

def check_tactical_vest_data_asset():
    """Проверить настройки Data Asset для тактического жилета"""
    
    print("=== ПРОВЕРКА DATA ASSET ТАКТИЧЕСКОГО ЖИЛЕТА ===")
    
    # Путь к Data Asset
    data_asset_path = "/Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest"
    
    # Загружаем Data Asset
    data_asset = unreal.EditorAssetLibrary.load_asset(data_asset_path)
    
    if not data_asset:
        print(f"❌ ОШИБКА: Data Asset не найден по пути: {data_asset_path}")
        return False
    
    print(f"✅ Data Asset найден: {data_asset.get_name()}")
    print(f"   Класс: {data_asset.get_class().get_name()}")
    
    # Проверяем свойства
    try:
        # DisplayName
        display_name = unreal.EditorAssetLibrary.get_editor_property(data_asset, "DisplayName")
        print(f"   DisplayName: {display_name}")
        
        # Размер в клетках
        size_x = unreal.EditorAssetLibrary.get_editor_property(data_asset, "SizeInCellsX")
        size_y = unreal.EditorAssetLibrary.get_editor_property(data_asset, "SizeInCellsY")
        print(f"   Размер: {size_x}x{size_y}")
        
        # Слот экипировки
        equipment_slot = unreal.EditorAssetLibrary.get_editor_property(data_asset, "EquipmentSlot")
        print(f"   EquipmentSlot: {equipment_slot}")
        
        # Сокет прикрепления
        attach_socket = unreal.EditorAssetLibrary.get_editor_property(data_asset, "AttachSocketName")
        print(f"   AttachSocketName: {attach_socket}")
        
        # EquippedMesh - самое важное!
        equipped_mesh = unreal.EditorAssetLibrary.get_editor_property(data_asset, "EquippedMesh")
        if equipped_mesh:
            print(f"   EquippedMesh: {equipped_mesh.get_name()} ✅")
        else:
            print(f"   EquippedMesh: NULL ❌")
            
            # Попробуем найти меш
            mesh_path = "/Game/insurgent_2/Characters/SK_ChestRigSmall"
            mesh_asset = unreal.EditorAssetLibrary.load_asset(mesh_path)
            if mesh_asset:
                print(f"   Найден меш по пути: {mesh_path}")
                print(f"   Имя меша: {mesh_asset.get_name()}")
                
                # Устанавливаем меш
                unreal.EditorAssetLibrary.set_editor_property(data_asset, "EquippedMesh", mesh_asset)
                unreal.EditorAssetLibrary.save_asset(data_asset_path)
                print(f"   ✅ EquippedMesh установлен и сохранен!")
            else:
                print(f"   ❌ Меш не найден по пути: {mesh_path}")
        
        # RelativeTransform
        relative_transform = unreal.EditorAssetLibrary.get_editor_property(data_asset, "RelativeTransform")
        print(f"   RelativeTransform: {relative_transform}")
        
        print("\n✅ Проверка завершена успешно!")
        return True
        
    except Exception as e:
        print(f"❌ Ошибка при проверке свойств: {e}")
        return False

def main():
    check_tactical_vest_data_asset()

if __name__ == "__main__":
    main()
