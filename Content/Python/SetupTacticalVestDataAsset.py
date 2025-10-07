# -*- coding: utf-8 -*-

import unreal

def setup_tactical_vest_data_asset():
    """Настроить Data Asset для тактического жилета"""
    
    print("=== НАСТРОЙКА DATA ASSET ДЛЯ ТАКТИЧЕСКОГО ЖИЛЕТА ===")
    
    # Ищем Data Asset
    data_asset = find_tactical_vest_data_asset()
    
    if not data_asset:
        print("❌ Data Asset для жилета не найден!")
        return False
    
    print(f"✅ Найден Data Asset: {data_asset.get_name()}")
    
    # Настраиваем свойства
    setup_properties(data_asset)
    
    return True

def find_tactical_vest_data_asset():
    """Найти Data Asset для жилета"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    data_asset_assets = asset_registry.get_assets_by_class("DataAsset", True)
    
    for asset in data_asset_assets:
        asset_name = asset.get_asset().get_name()
        asset_class = asset.get_asset().get_class().get_name()
        
        # Ищем наш Data Asset
        if ("tactical" in asset_name.lower() or "vest" in asset_name.lower() or "DA_" in asset_name) and "TacticalVest" in asset_class:
            return asset.get_asset()
    
    return None

def setup_properties(data_asset):
    """Настроить свойства Data Asset"""
    
    print("🔧 Настройка свойств Data Asset...")
    
    try:
        # DisplayName
        unreal.EditorAssetLibrary.set_editor_property(data_asset, "DisplayName", "Тактический жилет")
        print("   ✅ DisplayName установлен")
        
        # Размер
        unreal.EditorAssetLibrary.set_editor_property(data_asset, "SizeInCellsX", 3)
        unreal.EditorAssetLibrary.set_editor_property(data_asset, "SizeInCellsY", 3)
        print("   ✅ Размер установлен: 3x3")
        
        # Слот экипировки (Vest = 1)
        unreal.EditorAssetLibrary.set_editor_property(data_asset, "EquipmentSlot", 1)
        print("   ✅ EquipmentSlot установлен: Vest")
        
        # Сокет прикрепления
        unreal.EditorAssetLibrary.set_editor_property(data_asset, "AttachSocketName", "spine_03")
        print("   ✅ AttachSocketName установлен: spine_03")
        
        # Нельзя вращать
        unreal.EditorAssetLibrary.set_editor_property(data_asset, "bRotatable", False)
        print("   ✅ bRotatable установлен: False")
        
        # EquippedMesh
        mesh_set = setup_equipped_mesh(data_asset)
        
        # Сохраняем Data Asset
        data_asset_path = data_asset.get_path_name()
        unreal.EditorAssetLibrary.save_asset(data_asset_path)
        print(f"   ✅ Data Asset сохранен: {data_asset_path}")
        
        if mesh_set:
            print("🎉 Data Asset полностью настроен!")
        else:
            print("⚠️ Data Asset настроен, но EquippedMesh не установлен")
        
        return True
        
    except Exception as e:
        print(f"❌ Ошибка при настройке: {e}")
        return False

def setup_equipped_mesh(data_asset):
    """Установить EquippedMesh"""
    
    print("   🔍 Поиск меша для жилета...")
    
    # Возможные пути к мешу
    possible_paths = [
        "/Game/insurgent_2/Characters/SK_ChestRigSmall",
        "/Game/insurgent_2/Characters/SK_ChestRigSmall.SK_ChestRigSmall",
        "/Game/insurgent_2/Characters/SK_ChestRigSmall.SK_ChestRigSmall_C",
    ]
    
    for mesh_path in possible_paths:
        mesh_asset = unreal.EditorAssetLibrary.load_asset(mesh_path)
        if mesh_asset and mesh_asset.get_class().get_name() == "SkeletalMesh":
            unreal.EditorAssetLibrary.set_editor_property(data_asset, "EquippedMesh", mesh_asset)
            print(f"   ✅ EquippedMesh установлен: {mesh_asset.get_name()}")
            return True
    
    print("   ❌ Меш для жилета не найден")
    return False

def main():
    setup_tactical_vest_data_asset()

if __name__ == "__main__":
    main()

