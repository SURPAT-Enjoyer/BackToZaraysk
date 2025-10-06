# -*- coding: utf-8 -*-
import unreal

def create_basic_animations():
    """Create basic climbing animations using simple method"""
    
    print("BASIC ANIMATION CREATOR")
    print("=" * 30)
    
    try:
        # Get asset tools
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        
        # Create folder
        folder_path = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        
        if not unreal.EditorAssetLibrary.does_directory_exist(folder_path):
            unreal.EditorAssetLibrary.make_directory(folder_path)
            print("Created folder: Climbing")
        
        # Load skeleton
        skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        skeleton = unreal.load_asset(skeleton_path)
        
        if not skeleton:
            print("ERROR: Could not load skeleton")
            return
        
        print(f"SUCCESS: Skeleton loaded: {skeleton.get_name()}")
        
        # Animation names
        anim_names = ["AS_Vault", "AS_Mantle", "AS_LedgeClimb"]
        
        created_count = 0
        
        for anim_name in anim_names:
            full_path = f"{folder_path}/{anim_name}"
            
            # Check if already exists
            if unreal.EditorAssetLibrary.does_asset_exist(f"{full_path}.{anim_name}"):
                print(f"EXISTS: {anim_name}")
                continue
            
            print(f"Creating: {anim_name}")
            
            try:
                # Create animation sequence
                animation = asset_tools.create_asset(
                    asset_name=anim_name,
                    package_path=folder_path,
                    asset_class=unreal.AnimSequence,
                    factory=unreal.AnimationSequenceFactoryNew()
                )
                
                if animation:
                    # Set properties
                    animation.set_editor_property("target_skeleton", skeleton)
                    animation.set_editor_property("sequence_length", 2.0)
                    animation.set_editor_property("enable_root_motion", True)
                    
                    # Save
                    unreal.EditorAssetLibrary.save_asset(f"{full_path}.{anim_name}")
                    print(f"SUCCESS: {anim_name} created")
                    created_count += 1
                else:
                    print(f"ERROR: Could not create {anim_name}")
                    
            except Exception as e:
                print(f"ERROR creating {anim_name}: {e}")
        
        print(f"\nTOTAL CREATED: {created_count} animations")
        
        if created_count > 0:
            print("\nSUCCESS!")
            print("Animations created in:")
            print(folder_path)
            print("\nYou can now use these animations in ABP_Player")
        else:
            print("\nNo new animations created")
        
    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    create_basic_animations()
