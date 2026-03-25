import unreal

def add_ik_variables_to_player_abp():
    # Загрузить существующий ABP_Player
    abp = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/ABP_Player.ABP_Player")
    if not abp:
        print("ABP_Player not found at /Game/BackToZaraysk/Core/Characters/Player/ABP_Player")
        return

    print(f"Loaded ABP: {abp}")

    # Получить AnimBlueprintGeneratedClass
    anim_class = abp.generated_class
    if not anim_class:
        print("No generated class found")
        return

    print(f"Anim class: {anim_class}")

    # Создать переменные FVector для IK effector locations
    left_var_name = "LeftFootEffectorLocation"
    right_var_name = "RightFootEffectorLocation"

    # Проверить, существуют ли уже переменные
    if hasattr(anim_class, left_var_name):
        print(f"Variable {left_var_name} already exists")
    else:
        # Добавить переменную LeftFootEffectorLocation
        left_var = unreal.AnimBlueprintVariable()
        left_var.variable_name = left_var_name
        left_var.variable_type = unreal.AnimBlueprintVariableType.VECTOR
        left_var.default_value = unreal.Vector(0.0, 0.0, 0.0)
        left_var.bIsPublic = True

        # Добавить переменную в ABP
        abp.variable_set.add(left_var)
        print(f"Added variable {left_var_name}")

    if hasattr(anim_class, right_var_name):
        print(f"Variable {right_var_name} already exists")
    else:
        # Добавить переменную RightFootEffectorLocation
        right_var = unreal.AnimBlueprintVariable()
        right_var.variable_name = right_var_name
        right_var.variable_type = unreal.AnimBlueprintVariableType.VECTOR
        right_var.default_value = unreal.Vector(0.0, 0.0, 0.0)
        right_var.bIsPublic = True

        # Добавить переменную в ABP
        abp.variable_set.add(right_var)
        print(f"Added variable {right_var_name}")

    # Сохранить изменения
    try:
        unreal.EditorAssetLibrary.save_asset(abp.get_path_name())
        print("Successfully saved ABP_Player with IK variables")
    except Exception as e:
        print(f"Error saving ABP: {e}")

    # Попробовать скомпилировать ABP
    try:
        unreal.AnimBlueprintLibrary.compile_anim_blueprint(abp)
        print("Successfully compiled ABP_Player")
    except Exception as e:
        print(f"Error compiling ABP: {e}")

if __name__ == "__main__":
    add_ik_variables_to_player_abp()
