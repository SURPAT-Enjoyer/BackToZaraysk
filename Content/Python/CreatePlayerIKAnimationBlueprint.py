import unreal

def create_player_ik_animation_blueprint():
    # Создать новый Animation Blueprint для игрока с IK ног
    factory = unreal.AnimationBlueprintFactory()
    # Используем тот же скелет, что и в ABP_Player (предположительно Manny)
    factory.target_skeleton = unreal.load_asset("/Game/Characters/Manuequins/Meshes/SK_Manny.SK_Manny")

    # Создаем ABP в той же папке, что и ABP_Player
    abp = factory.factory_create_new("/Game/BackToZaraysk/Core/Characters/Player/ABP_Player_IK")

    # Получить AnimGraph
    anim_graph = abp.get_anim_graph()

    # Добавить Sequence Player
    sequence_player = unreal.AnimGraphNode_SequencePlayer()
    sequence_player.node.title = "Idle Sequence"
    # Попробуем найти подходящую idle анимацию
    try:
        idle_anim = unreal.load_asset("/Game/Characters/Manuequins/Animations/Manny/MM_Idle.MM_Idle")
        if idle_anim:
            sequence_player.sequence = idle_anim
    except:
        pass
    anim_graph.add_node(sequence_player)

    # Добавить переменные для IK effector locations
    # LeftFootEffectorLocation
    left_var = unreal.AnimBlueprintVariable()
    left_var.variable_name = "LeftFootEffectorLocation"
    left_var.variable_type = unreal.AnimBlueprintVariableType.VECTOR
    left_var.default_value = unreal.Vector(0.0, 0.0, 0.0)
    left_var.bIsPublic = True
    abp.variable_set.add(left_var)

    # RightFootEffectorLocation
    right_var = unreal.AnimBlueprintVariable()
    right_var.variable_name = "RightFootEffectorLocation"
    right_var.variable_type = unreal.AnimBlueprintVariableType.VECTOR
    right_var.default_value = unreal.Vector(0.0, 0.0, 0.0)
    right_var.bIsPublic = True
    abp.variable_set.add(right_var)

    # Добавить Two Bone IK для левой ноги
    left_foot_ik = unreal.AnimGraphNode_TwoBoneIK()
    left_foot_ik.node.title = "Left Foot IK"
    left_foot_ik.i_bone_a_bone_name = "thigh_l"
    left_foot_ik.i_bone_b_bone_name = "calf_l"
    left_foot_ik.i_effector_bone_name = "foot_l"
    anim_graph.add_node(left_foot_ik)

    # Добавить Two Bone IK для правой ноги
    right_foot_ik = unreal.AnimGraphNode_TwoBoneIK()
    right_foot_ik.node.title = "Right Foot IK"
    right_foot_ik.i_bone_a_bone_name = "thigh_r"
    right_foot_ik.i_bone_b_bone_name = "calf_r"
    right_foot_ik.i_effector_b_bone_name = "foot_r"
    anim_graph.add_node(right_foot_ik)

    # Добавить Output Pose
    output_pose = unreal.AnimGraphNode_Root()
    anim_graph.add_node(output_pose)

    # Подключить ноды
    try:
        # Sequence Player -> Left IK
        anim_graph.connect_nodes(sequence_player, 'Pose', left_foot_ik, 'Pose')
        # Left IK -> Right IK
        anim_graph.connect_nodes(left_foot_ik, 'Pose', right_foot_ik, 'Pose')
        # Right IK -> Output
        anim_graph.connect_nodes(right_foot_ik, 'Pose', output_pose, 'Result')

        # Подключить переменные к EffectorLocation
        # Создать getter ноды для переменных
        left_var_getter = unreal.AnimGraphNode_Variable()
        left_var_getter.node.title = "LeftFootEffectorLocation Getter"
        left_var_getter.variable_name = "LeftFootEffectorLocation"
        left_var_getter.bIsGet = True
        anim_graph.add_node(left_var_getter)

        right_var_getter = unreal.AnimGraphNode_Variable()
        right_var_getter.node.title = "RightFootEffectorLocation Getter"
        right_var_getter.variable_name = "RightFootEffectorLocation"
        right_var_getter.bIsGet = True
        anim_graph.add_node(right_var_getter)

        # Подключить getters к IK нодам
        anim_graph.connect_nodes(left_var_getter, 'Value', left_foot_ik, 'EffectorLocation')
        anim_graph.connect_nodes(right_var_getter, 'Value', right_foot_ik, 'EffectorLocation')

        print("Successfully connected IK nodes and variables")

    except Exception as e:
        print(f"Error connecting nodes: {e}")

    # Сохранить и скомпилировать
    try:
        unreal.EditorAssetLibrary.save_asset(abp.get_path_name())
        print("Successfully saved ABP_Player_IK")

        # Компилировать
        unreal.AnimBlueprintLibrary.compile_anim_blueprint(abp)
        print("Successfully compiled ABP_Player_IK")

    except Exception as e:
        print(f"Error saving/compiling ABP: {e}")

    print(f"Created Player IK Animation Blueprint at {abp.get_path_name()}")

    return abp

if __name__ == "__main__":
    create_player_ik_animation_blueprint()
