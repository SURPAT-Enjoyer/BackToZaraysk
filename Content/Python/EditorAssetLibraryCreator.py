# -*- coding: utf-8 -*-
import unreal

def create_animations_with_editor_library():
    """Create animations using only EditorAssetLibrary"""
    
    print("EDITOR ASSET LIBRARY ANIMATION CREATOR")
    print("=" * 40)
    
    try:
        # Create folder
        folder_path = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        
        if not unreal.EditorAssetLibrary.does_directory_exist(folder_path):
            unreal.EditorAssetLibrary.make_directory(folder_path)
            print("Created folder: Climbing")
        
        # Load skeleton
        skeleton = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin")
        
        if not skeleton:
            print("ERROR: Could not load skeleton")
            return
        
        print(f"SUCCESS: Skeleton loaded: {skeleton.get_name()}")
        
        # Animation names
        animations = ["AS_Vault", "AS_Mantle", "AS_LedgeClimb"]
        created_count = 0
        
        for anim_name in animations:
            asset_path = f"{folder_path}/{anim_name}.{anim_name}"
            
            if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
                print(f"EXISTS: {anim_name}")
                continue
            
            print(f"Creating: {anim_name}")
            
            try:
                # Create animation using EditorAssetLibrary
                new_anim = unreal.EditorAssetLibrary.create_asset(
                    asset_name=anim_name,
                    package_path=folder_path,
                    asset_class=unreal.AnimSequence,
                    factory=unreal.AnimationSequenceFactory()
                )
                
                if new_anim:
                    # Set properties
                    new_anim.set_editor_property("target_skeleton", skeleton)
                    new_anim.set_editor_property("sequence_length", 2.0)
                    new_anim.set_editor_property("enable_root_motion", True)
                    
                    # Save
                    unreal.EditorAssetLibrary.save_asset(asset_path)
                    print(f"SUCCESS: {anim_name} created")
                    created_count += 1
                else:
                    print(f"ERROR: Could not create {anim_name}")
                    
            except Exception as e:
                print(f"ERROR creating {anim_name}: {e}")
        
        print(f"\nTOTAL CREATED: {created_count} animations")
        
        if created_count > 0:
            print("\nSUCCESS!")
            print("Animations created successfully")
            print("You can now use them in ABP_Player State Machine")
        else:
            print("\nNo new animations created")
        
    except Exception as e:
        print(f"ERROR: {e}")

if __name__ == "__main__":
    create_animations_with_editor_library()





