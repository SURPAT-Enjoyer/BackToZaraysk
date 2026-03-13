import unreal

def fix_tactical_vest_physics():
    print("=== FIXING BP_TacticalVest PHYSICS ===")
    
    possible_paths = [
        "/Game/BackToZaraysk/Core/Items/Equipment/BP_TacticalVest",
        "/Game/BackToZaraysk/Items/BP_TacticalVest",
        "/Game/BackToZaraysk/BP_TacticalVest",
        "/Game/BP_TacticalVest",
        "/Game/Items/BP_TacticalVest"
    ]
    
    blueprint_path = None
    for path in possible_paths:
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            blueprint_path = path
            print("Found Blueprint at: " + path)
            break
    
    if not blueprint_path:
        print("ERROR: BP_TacticalVest not found in any expected location")
        print("Available assets in /Game/:")
        all_assets = unreal.EditorAssetLibrary.list_assets("/Game/", recursive=True)
        for asset in all_assets:
            if "TacticalVest" in asset or "tactical" in asset.lower():
                print("  " + asset)
        return False
    
    try:
        blueprint = unreal.load_asset(blueprint_path)
        if not blueprint:
            print("ERROR: Could not load Blueprint")
            return False
            
        print("SUCCESS: Blueprint loaded")
        
        # Исправляем настройки через EditorAssetLibrary
        print("Fixing collision and physics settings...")
        
        # Устанавливаем настройки коллизии
        unreal.EditorAssetLibrary.set_editor_property(blueprint, "CollisionEnabled", unreal.CollisionEnabled.QUERY_AND_PHYSICS, "Mesh")
        unreal.EditorAssetLibrary.set_editor_property(blueprint, "CollisionObjectType", unreal.CollisionChannel.WORLD_DYNAMIC, "Mesh")
        
        # Устанавливаем настройки физики
        unreal.EditorAssetLibrary.set_editor_property(blueprint, "SimulatePhysics", True, "Mesh")
        unreal.EditorAssetLibrary.set_editor_property(blueprint, "EnableGravity", True, "Mesh")
        
        # Устанавливаем ответы коллизии
        unreal.EditorAssetLibrary.set_editor_property(blueprint, "CollisionResponseToAllChannels", unreal.CollisionResponse.BLOCK, "Mesh")
        unreal.EditorAssetLibrary.set_editor_property(blueprint, "CollisionResponseToPawn", unreal.CollisionResponse.OVERLAP, "Mesh")
        
        print("Saving Blueprint...")
        unreal.EditorAssetLibrary.save_asset(blueprint_path)
        
        print("SUCCESS: Physics settings fixed!")
        return True
        
    except Exception as e:
        print("ERROR: " + str(e))
        return False

def main():
    print("Starting BP_TacticalVest physics fix...")
    success = fix_tactical_vest_physics()
    
    if success:
        print("FIX COMPLETED SUCCESSFULLY!")
    else:
        print("FIX FAILED!")

if __name__ == "__main__":
    main()
