#include "mm_instance.h"
#include "database.h"
#include "function/global/global_context.h"
#include "function/input/input_system.h"
#include "resource/asset_manager/asset_manager.h"
#include "spring.h"
#include "function/animation/animation_system.h"

using namespace Piccolo;

namespace Piccolo
{
	constexpr float g_times_per_tick = 1.0f / 60.f;

	struct character
	{
		Array1D<Vector3>        positions;
		Array1D<Vector3>        normals;
		Array1D<Vector2>        texcoords;
		Array1D<unsigned short> triangles;

		Array2D<float>          bone_weights;
		Array2D<unsigned short> bone_indices;

		Array1D<Vector3>    bone_rest_positions;
		Array1D<Quaternion> bone_rest_rotations;
	};

	void character_load(character& c, const std::string& filename)
	{
		FILE*   f;
		errno_t r = fopen_s(&f, filename.c_str(), "rb");
		assert(r == 0);

		array1d_read(c.positions, f);
		array1d_read(c.normals, f);
		array1d_read(c.texcoords, f);
		array1d_read(c.triangles, f);

		array2d_read(c.bone_weights, f);
		array2d_read(c.bone_indices, f);

		array1d_read(c.bone_rest_positions, f);
		array1d_read(c.bone_rest_rotations, f);

		r = fclose(f);
	}


	CAnimInstanceMotionMatching::CAnimInstanceMotionMatching()
	{
		const std::shared_ptr<AssetManager> asset_manager = g_runtime_global_context.m_asset_manager;
		assert(asset_manager);
		const std::string abs_path = asset_manager->getFullPath("asset/animation/database.bin").generic_string();
		LoadDataBase(m_data_base, abs_path);
		BuildMatchingFeature(m_data_base, m_feature_weight_foot_position, m_feature_weight_foot_velocity, m_feature_weight_hip_velocity, m_feature_weight_trajectory_positions, m_feature_weight_trajectory_directions);
		m_frame_index = m_data_base.m_range_starts(0);
		// init data
		m_curr_bone_positions          = m_data_base.m_bone_positions(m_frame_index);
		m_curr_bone_velocities         = m_data_base.m_bone_velocities(m_frame_index);
		m_curr_bone_rotations          = m_data_base.m_bone_rotations(m_frame_index);
		m_curr_bone_angular_velocities = m_data_base.m_bone_angular_velocities(m_frame_index);
		m_curr_bone_contacts           = m_data_base.m_contact_states(m_frame_index);

		m_trns_bone_positions          = m_data_base.m_bone_positions(m_frame_index);
		m_trns_bone_velocities         = m_data_base.m_bone_velocities(m_frame_index);
		m_trns_bone_rotations          = m_data_base.m_bone_rotations(m_frame_index);
		m_trns_bone_angular_velocities = m_data_base.m_bone_angular_velocities(m_frame_index);
		m_trns_bone_contacts           = m_data_base.m_contact_states(m_frame_index);

		m_bone_positions          = m_data_base.m_bone_positions(m_frame_index);
		m_bone_velocities         = m_data_base.m_bone_velocities(m_frame_index);
		m_bone_rotations          = m_data_base.m_bone_rotations(m_frame_index);
		m_bone_angular_velocities = m_data_base.m_bone_angular_velocities(m_frame_index);

		m_bone_offset_positions          = Array1D<Piccolo::Vector3>(m_data_base.GetNumBones());
		m_bone_offset_velocities         = Array1D<Piccolo::Vector3>(m_data_base.GetNumBones());
		m_bone_offset_rotations          = Array1D<Piccolo::Quaternion>(m_data_base.GetNumBones());
		m_bone_offset_angular_velocities = Array1D<Piccolo::Vector3>(m_data_base.GetNumBones());

		m_transition_src_position = Vector3::ZERO;
		m_transition_src_rotation = Quaternion::IDENTITY;
		m_transition_dst_position = Vector3::ZERO;
		m_transition_dst_rotation = Quaternion::IDENTITY;

		inertialize_pose_reset(m_bone_offset_positions, m_bone_offset_velocities, m_bone_offset_rotations, m_bone_offset_angular_velocities, m_transition_src_position, m_transition_src_rotation, m_transition_dst_position, m_transition_dst_rotation, m_bone_positions(0), m_bone_rotations(0));

		inertialize_pose_update(m_bone_positions, m_bone_velocities, m_bone_rotations, m_bone_angular_velocities, m_bone_offset_positions, m_bone_offset_velocities, m_bone_offset_rotations, m_bone_offset_angular_velocities, m_data_base.m_bone_positions(m_frame_index), m_data_base.m_bone_velocities(m_frame_index), m_data_base.m_bone_rotations(m_frame_index), m_data_base.m_bone_angular_velocities(m_frame_index), m_transition_src_position, m_transition_src_rotation, m_transition_dst_position, m_transition_dst_rotation, m_inertialize_blending_halflife, 0.0f);

		// Trajectory & Gameplay Data
		m_search_timer       = m_search_time;
		m_force_search_timer = m_search_time;

		m_desired_velocity             = Vector3::ZERO;
		m_desired_velocity_change_curr = Vector3::ZERO;
		m_desired_velocity_change_prev = Vector3::ZERO;

		m_desired_rotation             = Quaternion::IDENTITY;
		m_desired_rotation_change_curr = Vector3::ZERO;
		m_desired_rotation_change_prev = Vector3::ZERO;

		m_simulation_position         = Vector3::ZERO;
		m_simulation_velocity         = Vector3::ZERO;
		m_simulation_acceleration     = Vector3::ZERO;
		m_simulation_rotation         = Quaternion::IDENTITY;
		m_simulation_angular_velocity = Vector3::ZERO;

		m_trajectory_desired_velocities.resize(4);
		m_trajectory_desired_rotations.resize(4);
		m_trajectory_positions.resize(4);
		m_trajectory_velocities.resize(4);
		m_trajectory_accelerations.resize(4);
		m_trajectory_rotations.resize(4);
		m_trajectory_angular_velocities.resize(4);

		// for lmm
		const std::string decompressor_path = asset_manager->getFullPath("asset/animation/my_decompressor.bin").generic_string();
		LoadNetwork(m_decompressor, decompressor_path);

		const std::string latent_path = asset_manager->getFullPath("asset/animation/my_latent.bin").generic_string();
		LoadLatent(m_latent, latent_path);


		// for retargeting
		m_retarget_map.resize(58, (uint32_t)INDEX_NONE);
		m_retarget_map[0]  = Bone_Entity;
		m_retarget_map[1]  = Bone_Hips;
		m_retarget_map[54] = Bone_LeftUpLeg;
		m_retarget_map[55] = Bone_LeftLeg;
		m_retarget_map[56] = Bone_LeftFoot;
		m_retarget_map[57] = Bone_LeftToe;
		m_retarget_map[50] = Bone_RightUpLeg;
		m_retarget_map[51] = Bone_RightLeg;
		m_retarget_map[52] = Bone_RightFoot;
		m_retarget_map[53] = Bone_RightToe;
		m_retarget_map[3]  = Bone_Spine;
		m_retarget_map[4]  = Bone_Spine1;
		m_retarget_map[5]  = Bone_Spine2;
		m_retarget_map[6]  = Bone_Neck;
		m_retarget_map[7]  = Bone_Head;
		m_retarget_map[29] = Bone_LeftShoulder;
		m_retarget_map[30] = Bone_LeftArm;
		m_retarget_map[49] = Bone_LeftArm;
		m_retarget_map[31] = Bone_LeftForeArm;
		m_retarget_map[48] = Bone_LeftForeArm;
		m_retarget_map[32] = Bone_LeftHand;
		m_retarget_map[8]  = Bone_RightShoulder;
		m_retarget_map[9]  = Bone_RightArm;
		m_retarget_map[28]  = Bone_RightArm;
		m_retarget_map[10] = Bone_RightForeArm;
		m_retarget_map[27] = Bone_RightForeArm;
		m_retarget_map[11] = Bone_RightHand;

		const std::string character_path = asset_manager->getFullPath("asset/animation/character.bin").generic_string();
		character         character_data;
		character_load(character_data, character_path);

		auto size = character_data.bone_rest_positions.size;
		m_mm_bind_pose.reserve(size);
		m_mm_rest_pose_ls_rot.reserve(size);
		for (int i = 0; i < size; ++i)
		{
			m_mm_bind_pose.emplace_back(character_data.bone_rest_positions(i), character_data.bone_rest_rotations(i));
			m_mm_rest_pose_ls_rot.emplace_back(character_data.bone_rest_rotations(i));
		}
	}

	CAnimInstanceMotionMatching::CAnimInstanceMotionMatching(AnimationComponentRes* res) : CAnimInstanceMotionMatching()
	{
		std::shared_ptr<SkeletonData> skeleton_res = AnimationManager::tryLoadSkeleton(res->skeleton_file_path);
		m_skeleton.buildSkeleton(*skeleton_res);
	}

	Vector3 desired_velocity_update(const Vector3 input_direction, const float camera_yaw, const Quaternion simulation_rotation, const float fwrd_speed, const float side_speed, const float back_speed)
	{
		// Find stick position in world space by rotating using camera azimuth
		Vector3 global_stick_direction = Quaternion(Radian(camera_yaw), Vector3(0, 1, 0)) * input_direction;

		// Find stick position local to current facing direction
		Vector3 local_stick_direction = simulation_rotation.inverse() * global_stick_direction;

		// Scale stick by forward, sideways and backwards speeds
		Vector3 local_desired_velocity = local_stick_direction.z > 0.0f ? Vector3(side_speed, 0.0f, fwrd_speed) * local_stick_direction : Vector3(side_speed, 0.0f, back_speed) * local_stick_direction;

		// Re-orientate into the world space
		return simulation_rotation * local_desired_velocity;
	}

	Quaternion desired_rotation_update(const Quaternion desired_rotation, const Vector3 input_direction, const Vector3 camera_control, const float camera_yaw, const bool desired_strafe, const Vector3 desired_velocity)
	{
		Quaternion desired_rotation_curr = desired_rotation;

		// If strafe is active then desired direction is coming from right
		// stick as long as that stick is being used, otherwise we assume
		// forward facing
		if (desired_strafe)
		{
			Quaternion input_rotation    = Quaternion(Radian(camera_yaw), Vector3(0, 1, 0));
			Vector3    desired_direction = input_rotation * Vector3(0, 0, -1);

			if (camera_control.squaredLength() > 0.01f)
			{
				desired_direction = input_rotation * camera_control.normalisedCopy();
			}
			Radian angle = Radian(std::atan2f(desired_direction.x, desired_direction.z));
			return Quaternion(angle, Vector3(0, 1, 0));
		}

		// If strafe is not active the desired direction comes from the left
		// stick as long as that stick is being used
		else if (input_direction.squaredLength() > 0.01f)
		{
			Vector3 desired_direction = desired_velocity.normalisedCopy();
			Radian  angle             = Radian(std::atan2f(desired_direction.x, desired_direction.z));
			return Quaternion(angle, Vector3(0, 1, 0));
		}

		// Otherwise desired direction remains the same
		else
		{
			return desired_rotation_curr;
		}
	}

	void CAnimInstanceMotionMatching::TickAnimation(float delta_time)
	{
		m_crrent_time += delta_time;
		if (m_crrent_time > g_times_per_tick)
		{
			const int tick_count = static_cast<int>(m_crrent_time / g_times_per_tick);
			m_crrent_time        = m_crrent_time - g_times_per_tick * static_cast<float>(tick_count);
		}
		else
		{
			return;
		}
		delta_time = g_times_per_tick;

		// motion matching evaluate animation
		const unsigned command = g_runtime_global_context.m_input_system->getGameCommand();

		// cal current velocity
		Vector3 local_space_control_direction = Vector3::ZERO;
		if (command & static_cast<uint32_t>(GameCommand::forward))
		{
			local_space_control_direction -= Vector3::UNIT_Z;
		}
		if (command & static_cast<uint32_t>(GameCommand::backward))
		{
			local_space_control_direction += Vector3::UNIT_Z;
		}
		if (command & static_cast<uint32_t>(GameCommand::left))
		{
			local_space_control_direction -= Vector3::UNIT_X;
		}
		if (command & static_cast<uint32_t>(GameCommand::right))
		{
			local_space_control_direction += Vector3::UNIT_X;
		}

		Vector3 camera_control = Vector3::ZERO;

		float simulation_fwrd_speed = Math::lerp(m_simulation_run_fwrd_speed, m_simulation_walk_fwrd_speed, m_desired_gait);
		float simulation_back_speed = Math::lerp(m_simulation_run_side_speed, m_simulation_walk_side_speed, m_desired_gait);
		float simulation_side_speed = Math::lerp(m_simulation_run_back_speed, m_simulation_walk_back_speed, m_desired_gait);

		float   camera_yaw            = Math_PI;
		Vector3 desired_velocity_curr = desired_velocity_update(local_space_control_direction, camera_yaw, m_simulation_rotation, simulation_fwrd_speed, simulation_side_speed, simulation_back_speed);

		bool       desired_strafe        = false;
		Quaternion desired_rotation_curr = desired_rotation_update(m_desired_rotation, local_space_control_direction, camera_control, camera_yaw, desired_strafe, desired_velocity_curr);

		// Check if we should force a search because input changed quickly
		m_desired_velocity_change_prev = m_desired_velocity_change_curr;
		m_desired_velocity_change_curr = (desired_velocity_curr - m_desired_velocity) / delta_time;
		m_desired_velocity             = desired_velocity_curr;

		{
			m_desired_rotation_change_prev = m_desired_rotation_change_curr;
			Quaternion temp                = desired_rotation_curr.inverse() * m_desired_rotation;
			Radian     angle;
			Vector3    axis;
			temp.toAngleAxis(angle, axis);
			const Vector3 rot = axis * angle.valueRadians();

			m_desired_rotation_change_curr = rot / delta_time;
			m_desired_rotation             = desired_rotation_curr;
		}

		bool force_search = false;

		if (m_force_search_timer <= 0.0f && ((m_desired_velocity_change_prev.squaredLength() >= m_desired_velocity_change_threshold && m_desired_velocity_change_curr.squaredLength() < m_desired_velocity_change_threshold) || (m_desired_rotation_change_prev.squaredLength() >= m_desired_rotation_change_threshold && m_desired_rotation_change_curr.squaredLength() < m_desired_rotation_change_threshold)))
		{
			force_search         = true;
			m_force_search_timer = m_search_time;
		}
		else if (m_force_search_timer > 0)
		{
			m_force_search_timer -= delta_time;
		}

		// Predict Future Trajectory

		trajectory_desired_rotations_predict(m_trajectory_desired_rotations, m_trajectory_desired_velocities, m_desired_rotation, camera_yaw, local_space_control_direction, camera_control, desired_strafe, 20.0f * delta_time);

		trajectory_rotations_predict(m_trajectory_rotations, m_trajectory_angular_velocities, m_simulation_rotation, m_simulation_angular_velocity, m_trajectory_desired_rotations, m_simulation_rotation_halflife, 20.0f * delta_time);

		trajectory_desired_velocities_predict(m_trajectory_desired_velocities, m_trajectory_rotations, m_desired_velocity, camera_yaw, local_space_control_direction, camera_control, desired_strafe, simulation_fwrd_speed, simulation_side_speed, simulation_back_speed, 20.0f * delta_time);

		trajectory_positions_predict(m_trajectory_positions, m_trajectory_velocities, m_trajectory_accelerations, m_simulation_position, m_simulation_velocity, m_simulation_acceleration, m_trajectory_desired_velocities, m_simulation_velocity_halflife, 20.0f * delta_time);

		Array1D<float> query(m_data_base.GetNumFeatures());

		// Compute the features of the query vector
		Slice1D<float> query_features = m_data_base.m_features(m_frame_index);

		int offset = 0;
		query_copy_denormalized_feature(query, offset, 3, query_features, m_data_base.m_feature_offset, m_data_base.m_feature_scale); // Left Foot Position
		query_copy_denormalized_feature(query, offset, 3, query_features, m_data_base.m_feature_offset, m_data_base.m_feature_scale); // Right Foot Position
		query_copy_denormalized_feature(query, offset, 3, query_features, m_data_base.m_feature_offset, m_data_base.m_feature_scale); // Left Foot Velocity
		query_copy_denormalized_feature(query, offset, 3, query_features, m_data_base.m_feature_offset, m_data_base.m_feature_scale); // Right Foot Velocity
		query_copy_denormalized_feature(query, offset, 3, query_features, m_data_base.m_feature_offset, m_data_base.m_feature_scale); // Hip Velocity
		query_compute_trajectory_position_feature(query, offset, m_bone_positions(0), m_bone_rotations(0), m_trajectory_positions);
		query_compute_trajectory_direction_feature(query, offset, m_bone_rotations(0), m_trajectory_rotations);

		assert(offset == m_data_base.GetNumFeatures());

		bool end_of_anim = database_trajectory_index_clamp(m_data_base, m_frame_index, 1) == m_frame_index;

		if (force_search || m_search_timer <= 0.0f || end_of_anim)
		{
			// Search

			int   best_index = end_of_anim ? -1 : m_frame_index;
			float best_cost  = FLT_MAX;

			database_search(best_index, best_cost, m_data_base, query);

			// Transition if better frame found

			if (best_index != m_frame_index)
			{
				m_frame_index = best_index;
			}
			// Reset search timer
			m_search_timer = m_search_time;
		}

		// Tick down search timer
		m_search_timer -= delta_time;

		// Tick frame
		{
#define LMM 0
#if LMM
            int            input_size = static_cast<int>(m_data_base.GetNumFeatures()) + m_latent.m_cols;
            Array1D<float> input(input_size);
            uint32_t       num_feature = m_data_base.GetNumFeatures();

            for (uint32_t i = 0; i < num_feature; ++i)
            {
                input(static_cast<int>(i)) = m_data_base.m_features(m_frame_index, i);
            }
            for (int i = 0; i < m_latent.m_cols; ++i)
            {
                input(static_cast<int>(i + num_feature)) = m_latent(m_frame_index, i);
            }

            Array1D<float> output_layer;

            m_decompressor.EvaluateNn(input, output_layer);

            std::vector<float> debug_output_layer(output_layer.size);
            memcpy(debug_output_layer.data(), output_layer.data, output_layer.size * sizeof(float));

            Array1D<Vector3>    bone_positions(m_data_base.m_bone_positions.m_cols);
            Array1D<Quaternion> bone_rotations(m_data_base.m_bone_positions.m_cols);
            Array1D<Vector3>    bone_velocities(m_data_base.m_bone_positions.m_cols);
            Array1D<Vector3>    bone_angular_velocities(m_data_base.m_bone_positions.m_cols);
            // Extract bone positions
            int offset = 0;
            for (int i = 0; i < bone_positions.size - 1; i++)
            {
                bone_positions(i + 1) = Vector3(output_layer(offset + i * 3 + 0),
                                                output_layer(offset + i * 3 + 1),
                                                output_layer(offset + i * 3 + 2));
            }
            offset += (bone_positions.size - 1) * 3;

            // Extract bone rotations, convert from 2-axis representation
            for (int i = 0; i < bone_rotations.size - 1; i++)
            {
                bone_rotations(i + 1) = Quaternion::QuaternionFromXFormXY(Vector3(output_layer(offset + i * 6 + 0),
                                                                                  output_layer(offset + i * 6 + 2),
                                                                                  output_layer(offset + i * 6 + 4)),
                                                                          Vector3(output_layer(offset + i * 6 + 1),
                                                                                  output_layer(offset + i * 6 + 3),
                                                                                  output_layer(offset + i * 6 + 5)));
            }
            offset += (bone_rotations.size - 1) * 6;

            // Extract bone velocities
            for (int i = 0; i < bone_velocities.size - 1; i++)
            {
                bone_velocities(i + 1) = Vector3(output_layer(offset + i * 3 + 0),
                                                 output_layer(offset + i * 3 + 1),
                                                 output_layer(offset + i * 3 + 2));
            }
            offset += (bone_velocities.size - 1) * 3;

            // Extract bone angular velocities
            for (int i = 0; i < bone_angular_velocities.size - 1; i++)
            {
                bone_angular_velocities(i + 1) = Vector3(output_layer(offset + i * 3 + 0),
                                                         output_layer(offset + i * 3 + 1),
                                                         output_layer(offset + i * 3 + 2));
            }
            offset += (bone_angular_velocities.size - 1) * 3;

            // Extract root velocities and put in world space

            Vector3 root_velocity =
                m_curr_bone_rotations(0) *
                Vector3(output_layer(offset + 0), output_layer(offset + 1), output_layer(offset + 2));

            Vector3 root_angular_velocity =
                m_curr_bone_rotations(0) *
                Vector3(output_layer(offset + 3), output_layer(offset + 4), output_layer(offset + 5));

            offset += 6;

            // Find new root position/rotation/velocities etc.

            bone_positions(0)          = delta_time * root_velocity + m_curr_bone_positions(0);
            bone_rotations(0)          = Quaternion(root_angular_velocity * delta_time) * m_curr_bone_rotations(0);
            bone_velocities(0)         = root_velocity;
            bone_angular_velocities(0) = root_angular_velocity;

            // Extract bone contacts
            // if (bone_contacts.data != nullptr)
            // {
            //     bone_contacts(0) = output_layer(offset + 0) > 0.5f;
            //     bone_contacts(1) = output_layer(offset + 1) > 0.5f;
            // }

            offset += 2;

            // Check we got everything!
            assert(offset == output_layer.size);

            m_frame_index++;

            // 上一帧animation 中的root骨骼transform
            Vector3    last_frame_root_position = m_curr_bone_positions(0);
            Quaternion last_frame_root_rotation = m_curr_bone_rotations(0);

            // 这一阵animation bone
            Slice1D<Vector3>    curr_frame_bone_positions = bone_positions;
            Slice1D<Quaternion> curr_frame_bone_rotations = bone_rotations;

            m_curr_bone_positions = bone_positions;
            m_curr_bone_rotations = bone_rotations;
#else

			// 上一帧animation 中的root骨骼transform
			Vector3    last_frame_root_position = m_data_base.m_bone_positions(m_frame_index)(0);
			Quaternion last_frame_root_rotation = m_data_base.m_bone_rotations(m_frame_index)(0);

			m_frame_index++;
			// 这一阵animation bone
			Slice1D<Vector3>    curr_frame_bone_positions = m_data_base.m_bone_positions(m_frame_index);
			Slice1D<Quaternion> curr_frame_bone_rotations = m_data_base.m_bone_rotations(m_frame_index);
#endif

			// 当前模拟的root位置
			Vector3    curr_root_position = m_bone_positions(0);
			Quaternion curr_root_rotation = m_bone_rotations(0);

			// 做root motion
			Vector3 world_space_position = curr_root_rotation * (last_frame_root_rotation.inverse() * (curr_frame_bone_positions(0) - last_frame_root_position));
			// printf("world_space_delta_move:[%f, %f, %f]\n", world_space_position.x, world_space_position.y, world_space_position.z);
			// Normalize here because quat inv mul can sometimes produce
			// unstable returns when the two rotations are very close.
			Quaternion world_space_rotation = curr_root_rotation * (last_frame_root_rotation.inverse() * curr_frame_bone_rotations(0));
			world_space_rotation.normalise();
			// root bone 需要使用模拟位置
			m_bone_positions(0) = world_space_position + curr_root_position;
			m_bone_rotations(0) = world_space_rotation;

			// 除了root 其他直接copy到local space容器中
			for (int i = 1; i < m_bone_positions.size; i++)
			{
				m_bone_positions(i) = curr_frame_bone_positions(i);
				m_bone_rotations(i) = curr_frame_bone_rotations(i);
			}
		}

		// Update Simulation

		// Vector3 simulation_position_prev = m_simulation_position;

		simulation_positions_update(m_simulation_position, m_simulation_velocity, m_simulation_acceleration, m_desired_velocity, m_simulation_velocity_halflife, delta_time);

		simulation_rotations_update(m_simulation_rotation, m_simulation_angular_velocity, m_desired_rotation, m_simulation_rotation_halflife, delta_time);

		// final
		// printf("best index:%d\n", m_frame_index);
		// forward kinematic full
		forward_kinematic_full();
		m_root_motion   = m_bone_positions(0);
		m_root_rotation = m_bone_rotations(0);

		{
#if LMM
            Array1D<float> current_feature = m_data_base.m_features(m_frame_index);
#else
			Array1D<float> current_feature = m_data_base.m_features(m_frame_index);
#endif

			denormalize_features(current_feature, m_data_base.m_feature_offset, m_data_base.m_feature_scale);
			Quaternion rot = m_bone_rotations(0);
			Vector3    pos = m_bone_positions(0);

			Vector3 traj0_pos = rot * Vector3(current_feature(15), 0.0f, current_feature(16)) + pos;
			Vector3 traj1_pos = rot * Vector3(current_feature(17), 0.0f, current_feature(18)) + pos;
			Vector3 traj2_pos = rot * Vector3(current_feature(19), 0.0f, current_feature(20)) + pos;
			Vector3 traj0_dir = rot * Vector3(current_feature(21), 0.0f, current_feature(22));
			Vector3 traj1_dir = rot * Vector3(current_feature(23), 0.0f, current_feature(24));
			Vector3 traj2_dir = rot * Vector3(current_feature(25), 0.0f, current_feature(26));

			m_matched_position.resize(4);
			m_matched_position[0] = pos;
			m_matched_position[1] = traj0_pos;
			m_matched_position[2] = traj1_pos;
			m_matched_position[3] = traj2_pos;

			m_matched_direction.resize(4);
			m_matched_direction[0] = rot * Vector3(0, 0, 1);
			m_matched_direction[1] = traj0_dir;
			m_matched_direction[2] = traj1_dir;
			m_matched_direction[3] = traj2_dir;
		}
        DoRetargeting();
	}

	Transform CAnimInstanceMotionMatching::GetRootMotion()
	{
		return {m_root_motion, m_root_rotation};
	}

	bool CAnimInstanceMotionMatching::HasRootMotion()
	{
		return true;
	}

	void CAnimInstanceMotionMatching::BuildMatchingFeature(CDataBase& db, const float feature_weight_foot_position, const float feature_weight_foot_velocity, const float feature_weight_hip_velocity, const float feature_weight_trajectory_positions, const float feature_weight_trajectory_directions)
	{
		constexpr int num_features = 3 + // Left Foot Position
			3 +                          // Right Foot Position
			3 +                          // Left Foot Velocity
			3 +                          // Right Foot Velocity
			3 +                          // Hip Velocity
			6 +                          // Trajectory Positions 2D
			6;                           // Trajectory Directions 2D

		db.m_features.resize(db.GetNumFrames(), num_features);
		db.m_feature_offset.resize(num_features);
		db.m_feature_scale.resize(num_features);

		int offset = 0;
		ComputeBonePositionFeature(db, offset, Bone_LeftFoot, feature_weight_foot_position);
		ComputeBonePositionFeature(db, offset, Bone_RightFoot, feature_weight_foot_position);
		ComputeBoneVelocityFeature(db, offset, Bone_LeftFoot, feature_weight_foot_velocity);
		ComputeBoneVelocityFeature(db, offset, Bone_RightFoot, feature_weight_foot_velocity);
		ComputeBoneVelocityFeature(db, offset, Bone_Hips, feature_weight_hip_velocity);
		ComputeTrajectoryPositionFeature(db, offset, feature_weight_trajectory_positions);
		ComputeTrajectoryDirectionFeature(db, offset, feature_weight_trajectory_directions);

		assert(offset == num_features);

		DataBaseBuildBounds(db);
	}

	void CAnimInstanceMotionMatching::ComputeBonePositionFeature(CDataBase& db, int& offset, int bone, float weight)
	{
		for (int i = 0; i < db.GetNumFrames(); i++)
		{
			Vector3    bone_position;
			Quaternion bone_rotation;

			RecursiveForwardKinematics(bone_position, bone_rotation, db.m_bone_positions(i), db.m_bone_rotations(i), db.m_bone_parents, bone);

			bone_position = db.m_bone_rotations(i, 0).inverse() * (bone_position - db.m_bone_positions(i, 0));

			db.m_features(i, offset + 0) = bone_position.x;
			db.m_features(i, offset + 1) = bone_position.y;
			db.m_features(i, offset + 2) = bone_position.z;
		}

		NormalizeFeature(db.m_features, db.m_feature_offset, db.m_feature_scale, offset, 3, weight);

		offset += 3;
	}

	void CAnimInstanceMotionMatching::ComputeBoneVelocityFeature(CDataBase& db, int& offset, int bone, float weight)
	{
		for (int i = 0; i < db.GetNumFrames(); i++)
		{
			Vector3    bone_position;
			Vector3    bone_velocity;
			Quaternion bone_rotation;
			Vector3    bone_angular_velocity;

			RecursiveForwardKinematicsVelocity(bone_position, bone_velocity, bone_rotation, bone_angular_velocity, db.m_bone_positions(i), db.m_bone_velocities(i), db.m_bone_rotations(i), db.m_bone_angular_velocities(i), db.m_bone_parents, bone);

			bone_velocity = db.m_bone_rotations(i, 0).inverse() * bone_velocity;

			db.m_features(i, offset + 0) = bone_velocity.x;
			db.m_features(i, offset + 1) = bone_velocity.y;
			db.m_features(i, offset + 2) = bone_velocity.z;
		}

		NormalizeFeature(db.m_features, db.m_feature_offset, db.m_feature_scale, offset, 3, weight);

		offset += 3;
	}

	void CAnimInstanceMotionMatching::ComputeTrajectoryPositionFeature(CDataBase& db, int& offset, float weight)
	{
		for (int i = 0; i < static_cast<int>(db.GetNumFrames()); i++)
		{
			int t0 = DatabaseTrajectoryIndexClamp(db, i, 20);
			int t1 = DatabaseTrajectoryIndexClamp(db, i, 40);
			int t2 = DatabaseTrajectoryIndexClamp(db, i, 60);

			const Vector3 trajectory_pos0 = db.m_bone_rotations(i, 0).inverse() * (db.m_bone_positions(t0, 0) - db.m_bone_positions(i, 0));
			const Vector3 trajectory_pos1 = db.m_bone_rotations(i, 0).inverse() * (db.m_bone_positions(t1, 0) - db.m_bone_positions(i, 0));
			const Vector3 trajectory_pos2 = db.m_bone_rotations(i, 0).inverse() * (db.m_bone_positions(t2, 0) - db.m_bone_positions(i, 0));

			db.m_features(i, offset + 0) = trajectory_pos0.x;
			db.m_features(i, offset + 1) = trajectory_pos0.z;
			db.m_features(i, offset + 2) = trajectory_pos1.x;
			db.m_features(i, offset + 3) = trajectory_pos1.z;
			db.m_features(i, offset + 4) = trajectory_pos2.x;
			db.m_features(i, offset + 5) = trajectory_pos2.z;
		}

		NormalizeFeature(db.m_features, db.m_feature_offset, db.m_feature_scale, offset, 6, weight);

		offset += 6;
	}

	void CAnimInstanceMotionMatching::ComputeTrajectoryDirectionFeature(CDataBase& db, int& offset, float weight)
	{
		for (int i = 0; i < static_cast<int>(db.GetNumFrames()); i++)
		{
			int t0 = DatabaseTrajectoryIndexClamp(db, i, 20);
			int t1 = DatabaseTrajectoryIndexClamp(db, i, 40);
			int t2 = DatabaseTrajectoryIndexClamp(db, i, 60);

			const Vector3 trajectory_dir0 = db.m_bone_rotations(i, 0).inverse() * (db.m_bone_rotations(t0, 0) * Vector3(0, 0, 1));
			const Vector3 trajectory_dir1 = db.m_bone_rotations(i, 0).inverse() * (db.m_bone_rotations(t1, 0) * Vector3(0, 0, 1));
			const Vector3 trajectory_dir2 = db.m_bone_rotations(i, 0).inverse() * (db.m_bone_rotations(t2, 0) * Vector3(0, 0, 1));

			db.m_features(i, offset + 0) = trajectory_dir0.x;
			db.m_features(i, offset + 1) = trajectory_dir0.z;
			db.m_features(i, offset + 2) = trajectory_dir1.x;
			db.m_features(i, offset + 3) = trajectory_dir1.z;
			db.m_features(i, offset + 4) = trajectory_dir2.x;
			db.m_features(i, offset + 5) = trajectory_dir2.z;
		}

		NormalizeFeature(db.m_features, db.m_feature_offset, db.m_feature_scale, offset, 6, weight);

		offset += 6;
	}

	void CAnimInstanceMotionMatching::DataBaseBuildBounds(CDataBase& db)
	{
		int nbound_sm = ((db.GetNumFrames() + BOUND_SM_SIZE - 1) / BOUND_SM_SIZE);
		int nbound_lr = ((db.GetNumFrames() + BOUND_LR_SIZE - 1) / BOUND_LR_SIZE);

		db.bound_sm_min.resize(nbound_sm, db.GetNumFeatures());
		db.bound_sm_max.resize(nbound_sm, db.GetNumFeatures());
		db.bound_lr_min.resize(nbound_lr, db.GetNumFeatures());
		db.bound_lr_max.resize(nbound_lr, db.GetNumFeatures());

		db.bound_sm_min.set(FLT_MAX);
		db.bound_sm_max.set(FLT_MIN);
		db.bound_lr_min.set(FLT_MAX);
		db.bound_lr_max.set(FLT_MIN);

		for (int i = 0; i < db.GetNumFrames(); i++)
		{
			int i_sm = i / BOUND_SM_SIZE;
			int i_lr = i / BOUND_LR_SIZE;

			for (int j = 0; j < db.GetNumFeatures(); j++)
			{
				db.bound_sm_min(i_sm, j) = Math::min(db.bound_sm_min(i_sm, j), db.m_features(i, j));
				db.bound_sm_max(i_sm, j) = Math::max(db.bound_sm_max(i_sm, j), db.m_features(i, j));
				db.bound_lr_min(i_lr, j) = Math::min(db.bound_lr_min(i_lr, j), db.m_features(i, j));
				db.bound_lr_max(i_lr, j) = Math::max(db.bound_lr_max(i_lr, j), db.m_features(i, j));
			}
		}
	}

	void CAnimInstanceMotionMatching::RecursiveForwardKinematics(Vector3& bone_position, Quaternion& bone_rotation, const Slice1D<Vector3> bone_positions, const Slice1D<Quaternion> bone_rotations, const Slice1D<int> bone_parents, const int bone)
	{
		if (bone_parents(bone) != -1)
		{
			Vector3    parent_position;
			Quaternion parent_rotation;

			RecursiveForwardKinematics(parent_position, parent_rotation, bone_positions, bone_rotations, bone_parents, bone_parents(bone));

			bone_position = parent_rotation * bone_positions(bone) + parent_position;
			bone_rotation = parent_rotation * bone_rotations(bone);
		}
		else
		{
			bone_position = bone_positions(bone);
			bone_rotation = bone_rotations(bone);
		}
	}

	void CAnimInstanceMotionMatching::NormalizeFeature(Slice2D<float> features, Slice1D<float> features_offset, Slice1D<float> features_scale, const int offset, const int size, const float weight = 1.0f)
	{
		// First compute what is essentially the mean
		// value for each feature dimension
		for (int j = 0; j < size; j++)
		{
			features_offset(offset + j) = 0.0f;
		}

		for (int i = 0; i < features.rows; i++)
		{
			for (int j = 0; j < size; j++)
			{
				features_offset(offset + j) += features(i, offset + j) / static_cast<float>(features.rows);
			}
		}

		// Now compute the variance of each feature dimension
		Array1D<float> vars(size);
		vars.zero();

		for (int i = 0; i < features.rows; i++)
		{
			for (int j = 0; j < size; j++)
			{
				vars(j) += Math::sqr(features(i, offset + j) - features_offset(offset + j)) / static_cast<float>(features.rows);
			}
		}

		// We compute the overall std of the feature as the average
		// std across all dimensions
		float std = 0.0f;
		for (int j = 0; j < size; j++)
		{
			std += sqrtf(vars(j)) / static_cast<float>(size);
		}

		// Features with no variation can have zero std which is
		// almost always a bug.
		assert(std > 0.0f);

		// The scale of a feature is just the std divided by the weight
		for (int j = 0; j < size; j++)
		{
			features_scale(offset + j) = std / weight;
		}

		// Using the offset and scale we can then normalize the features
		for (int i = 0; i < features.rows; i++)
		{
			for (int j = 0; j < size; j++)
			{
				features(i, offset + j) = (features(i, offset + j) - features_offset(offset + j)) / features_scale(offset + j);
			}
		}
	}

	void CAnimInstanceMotionMatching::RecursiveForwardKinematicsVelocity(Vector3& bone_position, Vector3& bone_velocity, Quaternion& bone_rotation, Vector3& bone_angular_velocity, const Slice1D<Vector3> bone_positions, const Slice1D<Vector3> bone_velocities, const Slice1D<Quaternion> bone_rotations, const Slice1D<Vector3> bone_angular_velocities, const Slice1D<int> bone_parents, const int bone)
	{
		//
		if (bone_parents(bone) != -1)
		{
			Vector3    parent_position;
			Vector3    parent_velocity;
			Quaternion parent_rotation;
			Vector3    parent_angular_velocity;

			RecursiveForwardKinematicsVelocity(parent_position, parent_velocity, parent_rotation, parent_angular_velocity, bone_positions, bone_velocities, bone_rotations, bone_angular_velocities, bone_parents, bone_parents(bone));

			bone_position         = parent_rotation * bone_positions(bone) + parent_position;
			bone_velocity         = parent_velocity + parent_rotation * bone_velocities(bone) + parent_angular_velocity.crossProduct(parent_rotation * bone_positions(bone));
			bone_rotation         = parent_rotation * bone_rotations(bone);
			bone_angular_velocity = parent_rotation * (bone_angular_velocities(bone) + parent_angular_velocity);
		}
		else
		{
			bone_position         = bone_positions(bone);
			bone_velocity         = bone_velocities(bone);
			bone_rotation         = bone_rotations(bone);
			bone_angular_velocity = bone_angular_velocities(bone);
		}
	}

	int CAnimInstanceMotionMatching::DatabaseTrajectoryIndexClamp(CDataBase& db, int frame, int offset)
	{
		for (int i = 0; i < static_cast<int>(db.GetNumRanges()); i++)
		{
			if (frame >= db.m_range_starts(i) && frame < db.m_range_stops(i))
			{
				return std::clamp(frame + offset, db.m_range_starts(i), db.m_range_stops(i) - 1);
			}
		}

		assert(false);
		return -1;
	}

	void CAnimInstanceMotionMatching::trajectory_desired_rotations_predict(Slice1D<Quaternion> desired_rotations, const Slice1D<Vector3> desired_velocities, const Quaternion desired_rotation, const float camera_azimuth, const Vector3 gamepadstick_left, const Vector3 gamepadstick_right, const bool desired_strafe, const float dt)
	{
		desired_rotations(0) = desired_rotation;

		for (int i = 1; i < desired_rotations.size; i++)
		{
			desired_rotations(i) = desired_rotation_update(desired_rotations(i - 1), gamepadstick_left, gamepadstick_right, orbit_camera_update_azimuth(camera_azimuth, gamepadstick_right, desired_strafe, i * dt), desired_strafe, desired_velocities(i));
		}
	}

	float CAnimInstanceMotionMatching::orbit_camera_update_azimuth(const float azimuth, const Vector3 gamepadstick_right, const bool desired_strafe, const float dt)
	{
		Vector3 gamepadaxis = desired_strafe ? Vector3::ZERO : gamepadstick_right;
		return azimuth + 2.0f * dt * -gamepadaxis.x;
	}

	void CAnimInstanceMotionMatching::trajectory_rotations_predict(Slice1D<Quaternion> rotations, Slice1D<Vector3> angular_velocities, const Quaternion rotation, const Vector3 angular_velocity, const Slice1D<Quaternion> desired_rotations, const float halflife, const float dt)
	{
		rotations.set(rotation);
		angular_velocities.set(angular_velocity);

		for (int i = 1; i < rotations.size; i++)
		{
			simulation_rotations_update(rotations(i), angular_velocities(i), desired_rotations(i), halflife, i * dt);
		}
	}

	void CAnimInstanceMotionMatching::simulation_rotations_update(Quaternion& rotation, Vector3& angular_velocity, const Quaternion desired_rotation, const float halflife, const float dt)
	{
		simple_spring_damper_exact(rotation, angular_velocity, desired_rotation, halflife, dt);
	}

	void CAnimInstanceMotionMatching::trajectory_desired_velocities_predict(Slice1D<Vector3> desired_velocities, const Slice1D<Quaternion> trajectory_rotations, const Vector3 desired_velocity, const float camera_azimuth, const Vector3 gamepadstick_left, const Vector3 gamepadstick_right, const bool desired_strafe, const float fwrd_speed, const float side_speed, const float back_speed, const float dt)
	{
		desired_velocities(0) = desired_velocity;

		for (int i = 1; i < desired_velocities.size; i++)
		{
			desired_velocities(i) = desired_velocity_update(gamepadstick_left, orbit_camera_update_azimuth(camera_azimuth, gamepadstick_right, desired_strafe, i * dt), trajectory_rotations(i), fwrd_speed, side_speed, back_speed);
		}
	}

	void CAnimInstanceMotionMatching::trajectory_positions_predict(Slice1D<Vector3> positions, Slice1D<Vector3> velocities, Slice1D<Vector3> accelerations, const Vector3 position, const Vector3 velocity, const Vector3 acceleration, const Slice1D<Vector3> desired_velocities, const float halflife, const float dt)
	{
		positions(0)     = position;
		velocities(0)    = velocity;
		accelerations(0) = acceleration;

		for (int i = 1; i < positions.size; i++)
		{
			positions(i)     = positions(i - 1);
			velocities(i)    = velocities(i - 1);
			accelerations(i) = accelerations(i - 1);

			simulation_positions_update(positions(i), velocities(i), accelerations(i), desired_velocities(i), halflife, dt);
		}
	}

	void CAnimInstanceMotionMatching::simulation_positions_update(Vector3& position, Vector3& velocity, Vector3& acceleration, const Vector3 desired_velocity, const float halflife, const float dt)
	{
		float   y    = halflife_to_damping(halflife) / 2.0f;
		Vector3 j0   = velocity - desired_velocity;
		Vector3 j1   = acceleration + j0 * y;
		float   eydt = fast_negexpf(y * dt);

		Vector3 position_prev = position;

		position     = eydt * (((-j1) / (y * y)) + ((-j0 - j1 * dt) / y)) + (j1 / (y * y)) + j0 / y + desired_velocity * dt + position_prev;
		velocity     = eydt * (j0 + j1 * dt) + desired_velocity;
		acceleration = eydt * (acceleration - j1 * y * dt);

		// position = simulation_collide_obstacles(position_prev, position, obstacles_positions, obstacles_scales);
	}

	void CAnimInstanceMotionMatching::query_copy_denormalized_feature(Slice1D<float> query, int& offset, const int size, const Slice1D<float> features, const Slice1D<float> features_offset, const Slice1D<float> features_scale)
	{
		for (int i = 0; i < size; i++)
		{
			query(offset + i) = features(offset + i) * features_scale(offset + i) + features_offset(offset + i);
		}

		offset += size;
	}

	void CAnimInstanceMotionMatching::query_compute_trajectory_position_feature(Slice1D<float> query, int& offset, const Vector3 root_position, const Quaternion root_rotation, const Slice1D<Vector3> trajectory_positions)
	{
		Quaternion inv_rot = root_rotation.inverse();
		Vector3    traj0   = inv_rot * (trajectory_positions(1) - root_position);
		Vector3    traj1   = inv_rot * (trajectory_positions(2) - root_position);
		Vector3    traj2   = inv_rot * (trajectory_positions(3) - root_position);

		query(offset + 0) = traj0.x;
		query(offset + 1) = traj0.z;
		query(offset + 2) = traj1.x;
		query(offset + 3) = traj1.z;
		query(offset + 4) = traj2.x;
		query(offset + 5) = traj2.z;

		offset += 6;

		m_trajectory_position.resize(4);
        m_trajectory_position[0] = trajectory_positions(0);
        m_trajectory_position[1] = trajectory_positions(1);
        m_trajectory_position[2] = trajectory_positions(2);
        m_trajectory_position[3] = trajectory_positions(3);
	}

	void CAnimInstanceMotionMatching::query_compute_trajectory_direction_feature(Slice1D<float> query, int& offset, const Quaternion root_rotation, const Slice1D<Quaternion> trajectory_rotations)
	{
		Quaternion inv_rot = root_rotation.inverse();
		Vector3    traj0   = inv_rot * (trajectory_rotations(1) * Vector3::UNIT_Z);
		Vector3    traj1   = inv_rot * (trajectory_rotations(2) * Vector3::UNIT_Z);
		Vector3    traj2   = inv_rot * (trajectory_rotations(3) * Vector3::UNIT_Z);

		query(offset + 0) = traj0.x;
		query(offset + 1) = traj0.z;
		query(offset + 2) = traj1.x;
		query(offset + 3) = traj1.z;
		query(offset + 4) = traj2.x;
		query(offset + 5) = traj2.z;

		offset += 6;

		m_trajectory_direction.resize(4);
        m_trajectory_direction[0] = trajectory_rotations(0) * Vector3::UNIT_Z;
        m_trajectory_direction[1] = trajectory_rotations(1) * Vector3::UNIT_Z;
        m_trajectory_direction[2] = trajectory_rotations(2) * Vector3::UNIT_Z;
        m_trajectory_direction[3] = trajectory_rotations(3) * Vector3::UNIT_Z;
	}

	int CAnimInstanceMotionMatching::database_trajectory_index_clamp(CDataBase& db, int frame, int offset)
	{
		for (int i = 0; i < db.GetNumRanges(); i++)
		{
			if (frame >= db.m_range_starts(i) && frame < db.m_range_stops(i))
			{
#if LMM
                return clamp(frame + offset, db.m_range_starts(i), db.m_range_stops(i) - 2);
#else
				return clamp(frame + offset, db.m_range_starts(i), db.m_range_stops(i) - 1);
#endif
			}
		}
		return -1;
	}

	void CAnimInstanceMotionMatching::forward_kinematic_full()
	{
		m_result_frame.m_bone_names.resize(m_data_base.GetNumBones());
        m_result_frame.m_parents.resize(m_data_base.GetNumBones());
        m_result_frame.m_position.resize(m_data_base.GetNumBones());
        m_result_frame.m_rotation.resize(m_data_base.GetNumBones());
        m_result_frame.m_scaling.resize(m_data_base.GetNumBones());

		for (int i = 0; i < static_cast<int>(m_data_base.GetNumBones()); ++i)
		{
            m_result_frame.m_parents[i] = m_data_base.m_bone_parents(i);
            m_result_frame.m_position[i] = m_bone_positions(i);
            m_result_frame.m_rotation[i] = m_bone_rotations(i);
		}
	}

	void CAnimInstanceMotionMatching::inertialize_pose_reset(Slice1D<Vector3> bone_offset_positions, Slice1D<Vector3> bone_offset_velocities, Slice1D<Quaternion> bone_offset_rotations, Slice1D<Vector3> bone_offset_angular_velocities, Vector3& transition_src_position, Quaternion& transition_src_rotation, Vector3& transition_dst_position, Quaternion& transition_dst_rotation, const Vector3 root_position, const Quaternion root_rotation)
	{
		bone_offset_positions.zero();
		bone_offset_velocities.zero();
		bone_offset_rotations.set(Quaternion());
		bone_offset_angular_velocities.zero();

		transition_src_position = root_position;
		transition_src_rotation = root_rotation;
		transition_dst_position = Vector3();
		transition_dst_rotation = Quaternion();
	}

	// This function transitions the inertializer for
	// the full character. It takes as input the current
	// offsets, as well as the root transition locations,
	// current root state, and the full pose information
	// for the pose being transitioned from (src) as well
	// as the pose being transitioned to (dst) in their
	// own animation spaces.
	void CAnimInstanceMotionMatching::inertialize_pose_transition(Slice1D<Vector3> bone_offset_positions, Slice1D<Vector3> bone_offset_velocities, Slice1D<Quaternion> bone_offset_rotations, Slice1D<Vector3> bone_offset_angular_velocities, Vector3& transition_src_position, Quaternion& transition_src_rotation, Vector3& transition_dst_position, Quaternion& transition_dst_rotation, const Vector3 root_position, const Vector3 root_velocity, const Quaternion root_rotation, const Vector3 root_angular_velocity, const Slice1D<Vector3> bone_src_positions, const Slice1D<Vector3> bone_src_velocities, const Slice1D<Quaternion> bone_src_rotations, const Slice1D<Vector3> bone_src_angular_velocities, const Slice1D<Vector3> bone_dst_positions, const Slice1D<Vector3> bone_dst_velocities, const Slice1D<Quaternion> bone_dst_rotations, const Slice1D<Vector3> bone_dst_angular_velocities)
	{
		// First we record the root position and rotation
		// in the animation data for the source and destination
		// animation
		transition_dst_position = root_position;
		transition_dst_rotation = root_rotation;
		transition_src_position = bone_dst_positions(0);
		transition_src_rotation = bone_dst_rotations(0);

		// We then find the velocities so we can transition the
		// root inertiaizers
		Vector3 world_space_dst_velocity = transition_dst_rotation * (transition_src_rotation.inverse() * bone_dst_velocities(0));

		Vector3 world_space_dst_angular_velocity = transition_dst_rotation * (transition_src_rotation.inverse() * bone_dst_angular_velocities(0));

		// Transition inertializers recording the offsets for
		// the root joint
		inertialize_transition(bone_offset_positions(0), bone_offset_velocities(0), root_position, root_velocity, root_position, world_space_dst_velocity);

		inertialize_transition(bone_offset_rotations(0), bone_offset_angular_velocities(0), root_rotation, root_angular_velocity, root_rotation, world_space_dst_angular_velocity);

		// Transition all the inertializers for each other bone
		for (int i = 1; i < bone_offset_positions.size; i++)
		{
			inertialize_transition(bone_offset_positions(i), bone_offset_velocities(i), bone_src_positions(i), bone_src_velocities(i), bone_dst_positions(i), bone_dst_velocities(i));

			inertialize_transition(bone_offset_rotations(i), bone_offset_angular_velocities(i), bone_src_rotations(i), bone_src_angular_velocities(i), bone_dst_rotations(i), bone_dst_angular_velocities(i));
		}
	}

	// This function updates the inertializer states. Here
	// it outputs the smoothed animation (input plus offset)
	// as well as updating the offsets themselves. It takes
	// as input the current playing animation as well as the
	// root transition locations, a halflife, and a dt
	void CAnimInstanceMotionMatching::inertialize_pose_update(Slice1D<Vector3> bone_positions, Slice1D<Vector3> bone_velocities, Slice1D<Quaternion> bone_rotations, Slice1D<Vector3> bone_angular_velocities, Slice1D<Vector3> bone_offset_positions, Slice1D<Vector3> bone_offset_velocities, Slice1D<Quaternion> bone_offset_rotations, Slice1D<Vector3> bone_offset_angular_velocities, const Slice1D<Vector3> bone_input_positions, const Slice1D<Vector3> bone_input_velocities, const Slice1D<Quaternion> bone_input_rotations, const Slice1D<Vector3> bone_input_angular_velocities, const Vector3 transition_src_position, const Quaternion transition_src_rotation, const Vector3 transition_dst_position, const Quaternion transition_dst_rotation, const float halflife, const float dt)
	{
		// First we find the next root position, velocity, rotation
		// and rotational velocity in the world space by transforming
		// the input animation from it's animation space into the
		// space of the currently playing animation.
		Vector3 world_space_position = transition_dst_rotation * (transition_src_rotation.inverse() * (bone_input_positions(0) - transition_src_position)) + transition_dst_position;
		Vector3 world_space_velocity = transition_dst_rotation * (transition_src_rotation.inverse() * bone_input_velocities(0));

		// Normalize here because quat inv mul can sometimes produce
		// unstable returns when the two rotations are very close.
		Quaternion world_space_rotation = transition_dst_rotation * (transition_src_rotation.inverse() * bone_input_rotations(0));
		world_space_rotation.normalise();

		Vector3 world_space_angular_velocity = transition_dst_rotation * (transition_src_rotation.inverse() * bone_input_angular_velocities(0));

		// Then we update these two inertializers with these new world space inputs
		inertialize_update(bone_positions(0), bone_velocities(0), bone_offset_positions(0), bone_offset_velocities(0), world_space_position, world_space_velocity, halflife, dt);

		inertialize_update(bone_rotations(0), bone_angular_velocities(0), bone_offset_rotations(0), bone_offset_angular_velocities(0), world_space_rotation, world_space_angular_velocity, halflife, dt);

		// Then we update the inertializers for the rest of the bones
		for (int i = 1; i < bone_positions.size; i++)
		{
			// bone_positions(i)          = bone_input_positions(i);
			// bone_velocities(i)         = bone_input_velocities(i);
			// bone_rotations(i)          = bone_input_rotations(i);
			// bone_angular_velocities(i) = bone_input_angular_velocities(i);

			inertialize_update(bone_positions(i), bone_velocities(i), bone_offset_positions(i), bone_offset_velocities(i), bone_input_positions(i), bone_input_velocities(i), halflife, dt);

			inertialize_update(bone_rotations(i), bone_angular_velocities(i), bone_offset_rotations(i), bone_offset_angular_velocities(i), bone_input_rotations(i), bone_input_angular_velocities(i), halflife, dt);
		}
	}

	void CAnimInstanceMotionMatching::denormalize_features(Slice1D<float> features, const Slice1D<float> features_offset, const Slice1D<float> features_scale)
	{
		for (int i = 0; i < features.size; i++)
		{
			features(i) = (features(i) * features_scale(i)) + features_offset(i);
		}
	}

	void CAnimInstanceMotionMatching::DoRetargeting()
	{
		SAnimationFrame& mm_result = m_result_frame;

		const Skeleton& dest_skeleton = m_skeleton;

		// 先赋初值
		AnimationResult retargeted_result;
		const auto      bone_num = dest_skeleton.getBonesCount();

		retargeted_result.node.resize(bone_num);
		std::vector<std::string> names;
		std::vector<Vector3>     translate;
		std::vector<Vector3>     scaling;
		std::vector<Quaternion>  rotation;
		std::vector<Matrix4x4>   inv_t_pose;
		std::vector<Transform>   target_rest_ls_transforms;
		std::vector<int32_t>     parent_indices;

		names.resize(bone_num);
		translate.resize(bone_num);
		scaling.resize(bone_num);
		rotation.resize(bone_num);
		inv_t_pose.resize(bone_num);
		target_rest_ls_transforms.resize(bone_num);
		parent_indices.resize(bone_num, INDEX_NONE);
		//先使用t pose填充
		for (int i = 0; i < bone_num; ++i)
		{
			auto&      bone                     = dest_skeleton.getBones()[i];
			const auto index                    = static_cast<int>(bone.getID());
			retargeted_result.node[index].index = index + 1;
			target_rest_ls_transforms[index]         = Transform(bone.m_position, bone.m_orientation, bone.m_scale);
			translate[index]                    = bone.m_position;
			scaling[index]                      = bone.m_scale;
			rotation[index]                     = bone.m_orientation;
			names[index]                        = bone.m_name;
			inv_t_pose[index]                   = bone._getInverseTpose();

			const auto parent_node = bone.getParent();
			const auto parent_bone = dynamic_cast<Piccolo::Bone*>(parent_node);
			parent_indices[index]  = parent_bone ? static_cast<int>(parent_bone->getID()) : INDEX_NONE;
		}
        if (first_update)
		{
            first_update = false;
            std::vector<int32_t> bone_layer;
            bone_layer.resize(parent_indices.size());
            for (size_t i = 0; i < parent_indices.size(); ++i)
            {
                auto p = parent_indices[i];
                if (p == INDEX_NONE)
                {
                    bone_layer[i] = 0;
                }
                else
                {
                    bone_layer[i] = bone_layer[p] + 1;
                }
                for (int j = 0; j < bone_layer[i]; ++j)
                {
                    printf("\t");
                }
                printf("%llu-%s\n", i, names[i].c_str());
            }
			
		}

		// re-targeting
		std::vector<Transform> mm_anim_cs_trans;
        std::vector<Transform> mm_rest_cs_trans;
        mm_anim_cs_trans.reserve(m_mm_bind_pose.size());
        mm_rest_cs_trans.reserve(m_mm_bind_pose.size());

        for (size_t i = 0; i < m_mm_bind_pose.size(); ++i)
		{
            auto parent = mm_result.m_parents[i];
            if (parent == (uint32_t)INDEX_NONE)
            {
                mm_anim_cs_trans.emplace_back(Vector3::ZERO, mm_result.m_rotation[i]);
                mm_rest_cs_trans.emplace_back(m_mm_bind_pose[i]);
            }
            else
            {
	            auto& anim_parent_cs_tran = mm_anim_cs_trans[parent];
	            mm_anim_cs_trans.emplace_back(anim_parent_cs_tran * Transform(mm_result.m_position[i], mm_result.m_rotation[i]));
                mm_rest_cs_trans.emplace_back(m_mm_bind_pose[i]);
            }
		}



		std::vector<Transform> target_rest_pose_cs_transforms;
        target_rest_pose_cs_transforms.resize(bone_num);
        for (int i = 0; i < bone_num; ++i)
        {
            const int32_t parent_index     = parent_indices[i];
            if (parent_index != INDEX_NONE)
            {
                Transform parent_transform    = target_rest_pose_cs_transforms[parent_index];
                target_rest_pose_cs_transforms[i] = parent_transform * target_rest_ls_transforms[i];
            }
            else
            {
                target_rest_pose_cs_transforms[i] = target_rest_ls_transforms[i];
            }
        }

		/*
		 *
		 */
        std::vector<Transform> target_anim_pose_cs_transforms;
        target_anim_pose_cs_transforms.resize(bone_num);
        for (int target_idx = 0; target_idx < bone_num; ++target_idx)
        {
            const uint32_t src_idx = m_retarget_map[target_idx];
            const int32_t  parent_index = parent_indices[target_idx];
            if (src_idx == (uint32_t)INDEX_NONE)
            {
				//没有对应的重定向骨骼，使用本身的
                if (parent_index != INDEX_NONE)
                {
                    Transform parent_transform = target_anim_pose_cs_transforms[parent_index];
                    target_anim_pose_cs_transforms[target_idx] = parent_transform * target_rest_ls_transforms[target_idx];
                }
                else
                {
                    target_anim_pose_cs_transforms[target_idx] = target_rest_ls_transforms[target_idx];
                }
            }
            else
            {
                auto source_anim_cs = Math::RightHandYUpToZUp(mm_anim_cs_trans[src_idx]);
                auto source_anim_ls = Math::RightHandYUpToZUp(mm_result.m_rotation[src_idx]);
                auto source_rest_cs = Math::RightHandYUpToZUp(mm_rest_cs_trans[src_idx]);
                auto source_rest_ls = Math::RightHandYUpToZUp(m_mm_rest_pose_ls_rot[src_idx]);
                
				if (parent_index != INDEX_NONE)
                {
                    Transform& parent_transform             = target_anim_pose_cs_transforms[parent_index];
                    Transform& target_rest_pose_ls_transform = target_rest_ls_transforms[target_idx];
                    Transform& target_rest_pose_cs_transform = target_rest_pose_cs_transforms[target_idx];

                    target_anim_pose_cs_transforms[target_idx].m_rotation = source_anim_cs.m_rotation *
                                                                            source_rest_cs.m_rotation.inverse() *
                                                                            target_rest_pose_cs_transform.m_rotation;

                    target_anim_pose_cs_transforms[target_idx].m_position =
						parent_transform.m_rotation * target_rest_pose_ls_transform.m_position + parent_transform.m_position;
                }
                else
                {
                    target_anim_pose_cs_transforms[target_idx].m_rotation = source_anim_cs.m_rotation *
                                                                         source_rest_cs.m_rotation.inverse() *
                                                                         target_rest_ls_transforms[target_idx].m_rotation;
                    
                    target_anim_pose_cs_transforms[target_idx].m_position = target_rest_ls_transforms[target_idx].m_position;
                }
            }
        }

		m_component_space_transform.resize(target_anim_pose_cs_transforms.size());
        m_parent_info.resize(target_anim_pose_cs_transforms.size());
        for (size_t i = 0; i < target_anim_pose_cs_transforms.size(); ++i)
		{
            auto source_anim_cs            = target_anim_pose_cs_transforms[i];
            m_component_space_transform[i] = source_anim_cs.getMatrix();
            auto parent                    = parent_indices[i]; //mm_result.m_parents[i];
            m_parent_info[i]               = parent;
		}
		for (int i = 0; i < bone_num; ++i)
		{
            Matrix4x4 skinning_matrix           = target_anim_pose_cs_transforms[i].getMatrix() * inv_t_pose[i];
			retargeted_result.node[i].transform = skinning_matrix.toMatrix4x4_();
		}

		m_result = retargeted_result;
	}
}
