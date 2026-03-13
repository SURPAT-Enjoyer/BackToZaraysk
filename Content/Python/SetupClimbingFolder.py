# -*- coding: utf-8 -*-
import unreal

def setup_climbing_folder():
    """Setup climbing animations folder and provide instructions"""
    
    print("SETUP CLIMBING FOLDER")
    print("=" * 25)
    
    try:
        # Create folder
        folder_path = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        
        if not unreal.EditorAssetLibrary.does_directory_exist(folder_path):
            unreal.EditorAssetLibrary.make_directory(folder_path)
            print("SUCCESS: Created folder: Climbing")
        else:
            print("EXISTS: Climbing folder already exists")
        
        # Load skeleton
        skeleton = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin")
        
        if skeleton:
            print(f"SUCCESS: Skeleton loaded: {skeleton.get_name()}")
        else:
            print("ERROR: Could not load skeleton")
        
        print("\nMANUAL CREATION INSTRUCTIONS:")
        print("=" * 35)
        print("1. Open Content Browser")
        print("2. Navigate to: BackToZaraysk/Core/Characters/Player/Animations/Climbing")
        print("3. Right Click → Animation → Animation Sequence")
        print("4. Set Target Skeleton: SK_Mannequin")
        print("5. Create these animations:")
        print("   - AS_Vault")
        print("   - AS_Mantle")
        print("   - AS_LedgeClimb")
        print("6. For each animation, set properties:")
        print("   - Sequence Length: 2.0")
        print("   - Enable Root Motion: True")
        print("   - Root Motion Root Lock: Unlocked")
        
        print("\nALTERNATIVE: Use existing animations")
        print("=" * 40)
        print("1. Copy existing animations from:")
        print("   - Content/BackToZaraysk/Core/Characters/Player/Animations/")
        print("2. Rename them to:")
        print("   - AS_Vault")
        print("   - AS_Mantle")
        print("   - AS_LedgeClimb")
        print("3. Place in Climbing folder")
        
        print("\nFOLDER READY!")
        print("You can now create animations manually")
        
    except Exception as e:
        print(f"ERROR: {e}")

if __name__ == "__main__":
    setup_climbing_folder()





