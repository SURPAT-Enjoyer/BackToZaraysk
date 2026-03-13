import unreal

def create_strafe_input_actions():
    """Создает Input Actions для стрейфа"""
    
    # Создаем IA_StrafeRight
    strafe_right_action = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="IA_StrafeRight",
        package_path="/Game/BackToZaraysk/Core/Input/Actions/Strafe",
        asset_class=unreal.InputAction,
        factory=unreal.InputActionFactory()
    )
    
    if strafe_right_action:
        strafe_right_action.value_type = unreal.InputActionValueType.BOOLEAN
        unreal.EditorAssetLibrary.save_asset(strafe_right_action.get_path_name())
        print("Created IA_StrafeRight")
    
    # Создаем IA_StrafeLeft
    strafe_left_action = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="IA_StrafeLeft",
        package_path="/Game/BackToZaraysk/Core/Input/Actions/Strafe",
        asset_class=unreal.InputAction,
        factory=unreal.InputActionFactory()
    )
    
    if strafe_left_action:
        strafe_left_action.value_type = unreal.InputActionValueType.BOOLEAN
        unreal.EditorAssetLibrary.save_asset(strafe_left_action.get_path_name())
        print("Created IA_StrafeLeft")

def setup_strafe_input_mapping():
    """Настраивает Input Mapping Context для стрейфа"""
    
    # Загружаем существующий IMC_Default
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    
    if not imc_default:
        print("IMC_Default not found!")
        return
    
    # Загружаем созданные Input Actions
    strafe_right_action = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_StrafeRight.IA_StrafeRight")
    strafe_left_action = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_StrafeLeft.IA_StrafeLeft")
    
    if not strafe_right_action or not strafe_left_action:
        print("Strafe Input Actions not found!")
        return
    
    # Создаем мэппинги
    # D + Space (двойное нажатие Space) для стрейфа вправо
    right_mapping = unreal.EnhancedActionKeyMapping()
    right_mapping.action = strafe_right_action
    right_mapping.key = unreal.Key.D
    right_mapping.modifiers = []
    right_mapping.trigger = unreal.TriggerType.IMPULSE
    
    # A + Space (двойное нажатие Space) для стрейфа влево  
    left_mapping = unreal.EnhancedActionKeyMapping()
    left_mapping.action = strafe_left_action
    left_mapping.key = unreal.Key.A
    left_mapping.modifiers = []
    left_mapping.trigger = unreal.TriggerType.IMPULSE
    
    # Добавляем мэппинги в IMC
    imc_default.mappings.append(right_mapping)
    imc_default.mappings.append(left_mapping)
    
    # Сохраняем
    unreal.EditorAssetLibrary.save_asset(imc_default.get_path_name())
    print("Updated IMC_Default with strafe mappings")

if __name__ == "__main__":
    create_strafe_input_actions()
    setup_strafe_input_mapping()
    print("Strafe input actions setup complete!")

