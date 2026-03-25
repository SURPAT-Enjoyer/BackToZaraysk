# -*- coding: utf-8 -*-
import unreal

def create_animations():
    """Create climbing animations only"""
    
    print("CREATING CLIMBING ANIMATIONS")
    print("=" * 30)
    
    try:
        # Load target skeleton
        skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        skeleton = unreal.load_asset(skeleton_path)
        
        if not skeleton:
            print(f"ERROR: Skeleton not found: {skeleton_path}")
            return
        
        print(f"SUCCESS: Skeleton loaded")
        
        # Create folder
        folder_path = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        if not unreal.EditorAssetLibrary.does_directory_exist(folder_path):
            unreal.EditorAssetLibrary.make_directory(folder_path)
            print("Created folder: Climbing")
        
        # Animation data
        anims = [
            {"name": "AS_Vault", "desc": "Vault animation"},
            {"name": "AS_Mantle", "desc": "Mantle animation"},
            {"name": "AS_LedgeClimb", "desc": "Ledge climb animation"}
        ]
        
        # Create animations
        for anim in anims:
            asset_path = f"{folder_path}/{anim['name']}.{anim['name']}"
            
            if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
                print(f"EXISTS: {anim['name']}")
                continue
            
            print(f"Creating: {anim['name']} - {anim['desc']}")
            
            # Create factory
            factory = unreal.AnimationSequenceFactoryNew()
            factory.target_skeleton = skeleton
            
            # Create animation
            new_anim = factory.factory_create_new(asset_path)
            
            if new_anim:
                # Set properties
                new_anim.set_editor_property("sequence_length", 2.0)
                new_anim.set_editor_property("enable_root_motion", True)
                
                # Save
                unreal.EditorAssetLibrary.save_asset(asset_path)
                print(f"SUCCESS: {anim['name']} created")
            else:
                print(f"ERROR: Failed to create {anim['name']}")
        
        print("\nDONE!")
        print("Animations created in:")
        print(folder_path)
        
    except Exception as e:
        print(f"ERROR: {e}")

if __name__ == "__main__":
    create_animations()
