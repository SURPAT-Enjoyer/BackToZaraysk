# -*- coding: utf-8 -*-

import unreal

def check_all_data_assets():
    """Проверить все Data Asset в проекте"""
    
    print("=== ПОИСК ВСЕХ DATA ASSET В ПРОЕКТЕ ===")
    
    # Получаем Asset Registry
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    
    # Ищем все Data Asset
    data_asset_assets = asset_registry.get_assets_by_class("DataAsset", True)
    
    print(f"Найдено {len(data_asset_assets)} Data Asset в проекте:")
    
    for asset in data_asset_assets:
        asset_name = asset.get_asset().get_name()
        asset_path = asset.get_asset().get_path_name()
        asset_class = asset.get_asset().get_class().get_name()
        
        print(f"   - {asset_name}")
        print(f"     Путь: {asset_path}")
        print(f"     Класс: {asset_class}")
        
        # Если это наш Data Asset
        if "tactical" in asset_name.lower() or "vest" in asset_name.lower():
            print(f"     🎯 НАЙДЕН DATA ASSET ДЛЯ ЖИЛЕТА!")
            check_tactical_vest_data_asset(asset.get_asset())
        
        print()

def check_tactical_vest_data_asset(data_asset):
    """Проверить конкретный Data Asset для жилета"""
    
    print(f"=== ПРОВЕРКА DATA ASSET: {data_asset.get_name()} ===")
    
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
            
            # Попробуем найти и установить меш
            setup_equipped_mesh(data_asset)
        
        # RelativeTransform
        relative_transform = unreal.EditorAssetLibrary.get_editor_property(data_asset, "RelativeTransform")
        print(f"   RelativeTransform: {relative_transform}")
        
        print("✅ Проверка завершена!")
        
    except Exception as e:
        print(f"❌ Ошибка при проверке свойств: {e}")

def setup_equipped_mesh(data_asset):
    """Установить EquippedMesh для Data Asset"""
    
    print("   🔧 Попытка установить EquippedMesh...")
    
    # Возможные пути к мешу
    possible_paths = [
        "/Game/insurgent_2/Characters/SK_ChestRigSmall",
        "/Game/insurgent_2/Characters/SK_ChestRigSmall.SK_ChestRigSmall",
        "/Game/insurgent_2/Characters/SK_ChestRigSmall.SK_ChestRigSmall_C",
    ]
    
    for mesh_path in possible_paths:
        print(f"   Проверяем путь: {mesh_path}")
        
        mesh_asset = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if mesh_asset:
            mesh_class = mesh_asset.get_class().get_name()
            print(f"   ✅ Найден объект: {mesh_asset.get_name()}, класс: {mesh_class}")
            
            if mesh_class == "SkeletalMesh":
                # Устанавливаем меш
                unreal.EditorAssetLibrary.set_editor_property(data_asset, "EquippedMesh", mesh_asset)
                
                # Сохраняем Data Asset
                data_asset_path = data_asset.get_path_name()
                unreal.EditorAssetLibrary.save_asset(data_asset_path)
                
                print(f"   ✅ EquippedMesh установлен и сохранен!")
                return True
            else:
                print(f"   ⚠️ Это не SkeletalMesh")
        else:
            print(f"   ❌ Не найден")
    
    print("   ❌ Не удалось найти подходящий меш")
    return False

def main():
    check_all_data_assets()

if __name__ == "__main__":
    main()
