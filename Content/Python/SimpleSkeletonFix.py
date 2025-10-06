# -*- coding: utf-8 -*-
import unreal

def simple_fix():
    """Simple fix for skeleton compatibility"""
    
    print("SIMPLE SKELETON COMPATIBILITY FIX")
    print("=" * 40)
    
    try:
        # Create simple climbing animations
        create_simple_animations()
        
    except Exception as e:
        print(f"ERROR: {e}")

def create_simple_animations():
    """Create simple climbing animations"""
    
    print("Creating simple climbing animations...")
    
    try:
        target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        target_skeleton = unreal.load_asset(target_skeleton_path)
        
        if not target_skeleton:
            print("ERROR: Could not load target skeleton")
            return
        
        print(f"SUCCESS: Target skeleton loaded: {target_skeleton.get_name()}")
        
        # Create animations folder
        anim_folder = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        if not unreal.EditorAssetLibrary.does_directory_exist(anim_folder):
            unreal.EditorAssetLibrary.make_directory(anim_folder)
            print("Created climbing animations folder")
        
        # Create simple animations
        animations = [
            {"name": "AS_Vault", "length": 1.5},
            {"name": "AS_Mantle", "length": 2.0},
            {"name": "AS_LedgeClimb", "length": 2.0}
        ]
        
        created_count = 0
        for anim_data in animations:
            anim_path = f"{anim_folder}/{anim_data['name']}.{anim_data['name']}"
            
            if not unreal.EditorAssetLibrary.does_asset_exist(anim_path):
                print(f"Creating {anim_data['name']}...")
                
                # Create animation sequence
                factory = unreal.AnimationSequenceFactoryNew()
                factory.target_skeleton = target_skeleton
                
                new_anim = factory.factory_create_new(anim_path)
                if new_anim:
                    # Set basic properties
                    new_anim.set_editor_property("sequence_length", anim_data['length'])
                    new_anim.set_editor_property("enable_root_motion", True)
                    
                    # Save the animation
                    unreal.EditorAssetLibrary.save_asset(anim_path)
                    print(f"SUCCESS: Created {anim_data['name']}")
                    created_count += 1
                else:
                    print(f"ERROR: Could not create {anim_data['name']}")
            else:
                print(f"WARNING: {anim_data['name']} already exists")
        
        print(f"\nTOTAL CREATED: {created_count} animations")
        
        if created_count > 0:
            print("\nANIMATIONS CREATED SUCCESSFULLY!")
            print("You can now use these animations in ABP_Player State Machine")
            print("\nNEXT STEPS:")
            print("1. Open ABP_Player")
            print("2. Create State Machine for climbing")
            print("3. Connect AS_Vault, AS_Mantle, AS_LedgeClimb animations")
            print("4. Set up transitions with bIsVaulting, bIsMantling, bIsLedgeClimbing")
        else:
            print("\nNO NEW ANIMATIONS CREATED")
            print("All animations already exist or there was an error")
        
    except Exception as e:
        print(f"ERROR creating animations: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    simple_fix()
