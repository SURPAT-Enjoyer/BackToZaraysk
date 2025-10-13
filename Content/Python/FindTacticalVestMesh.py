# -*- coding: utf-8 -*-

import unreal

def find_tactical_vest_mesh():
    """Найти меш тактического жилета в проекте"""
    
    print("=== ПОИСК МЕША ТАКТИЧЕСКОГО ЖИЛЕТА ===")
    
    # Возможные пути к мешу
    possible_paths = [
        "/Game/insurgent_2/Characters/SK_ChestRigSmall",
        "/Game/insurgent_2/Characters/SK_ChestRigSmall.SK_ChestRigSmall",
        "/Game/insurgent_2/Characters/SK_ChestRigSmall.SK_ChestRigSmall_C",
        "/Game/insurgent_2/Characters/SK_ChestRigSmall",
        "/Game/insurgent_2/Characters/",
        "/Game/BackToZaraysk/Characters/",
        "/Game/BackToZaraysk/Core/Characters/",
    ]
    
    found_meshes = []
    
    for path in possible_paths:
        print(f"Проверяем путь: {path}")
        
        # Пытаемся загрузить как USkeletalMesh
        mesh = unreal.EditorAssetLibrary.load_asset(path)
        if mesh:
            mesh_class = mesh.get_class().get_name()
            print(f"   ✅ Найден объект: {mesh.get_name()}, класс: {mesh_class}")
            
            if mesh_class == "SkeletalMesh":
                found_meshes.append((path, mesh))
                print(f"   🎯 Это SkeletalMesh!")
            else:
                print(f"   ⚠️ Это не SkeletalMesh")
        else:
            print(f"   ❌ Не найден")
    
    # Поиск всех SkeletalMesh в проекте
    print("\n=== ПОИСК ВСЕХ SKELETAL MESH В ПРОЕКТЕ ===")
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    skeletal_mesh_assets = asset_registry.get_assets_by_class("SkeletalMesh", True)
    
    print(f"Найдено {len(skeletal_mesh_assets)} SkeletalMesh в проекте:")
    
    for asset in skeletal_mesh_assets:
        asset_name = asset.get_asset().get_name()
        asset_path = asset.get_asset().get_path_name()
        
        # Ищем меши, содержащие "chest", "rig", "vest", "tactical"
        search_terms = ["chest", "rig", "vest", "tactical", "ChestRig"]
        if any(term.lower() in asset_name.lower() for term in search_terms):
            print(f"   🎯 {asset_name} -> {asset_path}")
            found_meshes.append((asset_path, asset.get_asset()))
    
    print(f"\n✅ Найдено {len(found_meshes)} подходящих мешей:")
    for path, mesh in found_meshes:
        print(f"   - {mesh.get_name()} -> {path}")
    
    return found_meshes

def main():
    find_tactical_vest_mesh()

if __name__ == "__main__":
    main()


