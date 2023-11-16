#pragma once
#include <vector>
#include "database.h"
#include "anim_instance.h"
#include "runtime/core/math/vector3.h"
#include "neural_network.h"

namespace Piccolo{
	enum Bones
	{
		Bone_Entity = 0,
		Bone_Hips = 1,
		Bone_LeftUpLeg = 2,
		Bone_LeftLeg = 3,
		Bone_LeftFoot = 4,
		Bone_LeftToe = 5,
		Bone_RightUpLeg = 6,
		Bone_RightLeg = 7,
		Bone_RightFoot = 8,
		Bone_RightToe = 9,
		Bone_Spine = 10,
		Bone_Spine1 = 11,
		Bone_Spine2 = 12,
		Bone_Neck = 13,
		Bone_Head = 14,
		Bone_LeftShoulder = 15,
		Bone_LeftArm = 16,
		Bone_LeftForeArm = 17,
		Bone_LeftHand = 18,
		Bone_RightShoulder = 19,
		Bone_RightArm = 20,
		Bone_RightForeArm = 21,
		Bone_RightHand = 22,
		Bone_Count,
	};

	struct MapPair
    {
        MapPair(std::string src, std::string dest) : src_name(std::move(src)), dest_name(std::move(dest)) {}
        std::string src_name;
        std::string dest_name;
    };

    struct MMPair
    {
        MMPair(Bones src, std::string dst) : src_idx(src), dest_name(std::move(dst)) {}
        Bones       src_idx;
        std::string dest_name;
    };

    
	struct SAnimationFrame
    {
        std::vector<std::string>         m_bone_names;
        std::vector<Vector3>    m_position;
        std::vector<Quaternion> m_rotation;
        std::vector<Vector3>    m_scaling;
        std::vector<uint32_t>            m_parents;
    };

	REFLECTION_TYPE(CAnimInstanceMotionMatching)

    CLASS(CAnimInstanceMotionMatching : public CAnimInstanceBase, WhiteListFields)
    {
        REFLECTION_BODY(CAnimInstanceMotionMatching)
	
	public:
        CAnimInstanceMotionMatching();
		CAnimInstanceMotionMatching(AnimationComponentRes* res);
		~CAnimInstanceMotionMatching() override = default;
		void TickAnimation(float delta_time) override;
        Skeleton* GetSkeleton() override { return &m_skeleton; }

		Transform GetRootMotion() override;
		bool HasRootMotion() override;



		// debug info
        std::vector<Vector3> m_trajectory_position;
        std::vector<Vector3> m_trajectory_direction;

        std::vector<Vector3> m_matched_position;
        std::vector<Vector3> m_matched_direction;
	private:
        std::vector<Transform> m_mm_bind_pose;
        std::vector<Quaternion> m_mm_rest_pose_ls_rot;



        Skeleton m_skeleton;
        // dst_idx->src_idx 选小的当做映射表 找个地方存一下这个rig映射
        std::vector<uint32_t>  m_retarget_map;
		Vector3 m_root_motion;
        Quaternion m_root_rotation;
        
        SAnimationFrame m_result_frame;

		float m_crrent_time{0.0f};
		CDataBase m_data_base;

		float m_feature_weight_foot_position{0.75f};
		float m_feature_weight_foot_velocity{1.0f};
		float m_feature_weight_hip_velocity{1.0f};
		float m_feature_weight_trajectory_positions{1.0f};
		float m_feature_weight_trajectory_directions{1.5f};

		// Pose & Inertializer Data
		int m_frame_index{0};
		float m_inertialize_blending_halflife{0.1f};
		Array1D<Vector3> m_curr_bone_positions;
		Array1D<Vector3> m_curr_bone_velocities;
		Array1D<Quaternion> m_curr_bone_rotations;
		Array1D<Vector3> m_curr_bone_angular_velocities;
		Array1D<bool> m_curr_bone_contacts;

		Array1D<Vector3> m_trns_bone_positions;
		Array1D<Vector3> m_trns_bone_velocities;
		Array1D<Quaternion> m_trns_bone_rotations;
		Array1D<Vector3> m_trns_bone_angular_velocities;
		Array1D<bool> m_trns_bone_contacts;

		Array1D<Vector3> m_bone_positions;
		Array1D<Vector3> m_bone_velocities;
		Array1D<Quaternion> m_bone_rotations;
		Array1D<Vector3> m_bone_angular_velocities;

		Array1D<Vector3> m_bone_offset_positions;
		Array1D<Vector3> m_bone_offset_velocities;
		Array1D<Quaternion> m_bone_offset_rotations;
		Array1D<Vector3> m_bone_offset_angular_velocities;


        Vector3 m_transition_src_position;
        Quaternion m_transition_src_rotation;
        Vector3 m_transition_dst_position;
        Quaternion m_transition_dst_rotation;

        // Trajectory & Gameplay Data
        float m_search_time{0.1f};
        float m_search_timer{0};
        float m_force_search_timer{0};

        Vector3 m_desired_velocity;
        Vector3 m_desired_velocity_change_curr;
        Vector3 m_desired_velocity_change_prev;
		float m_desired_velocity_change_threshold{50.0};

        Quaternion m_desired_rotation;
        Vector3 m_desired_rotation_change_curr;
        Vector3 m_desired_rotation_change_prev;
		float m_desired_rotation_change_threshold{50.0};

		float m_desired_gait{0.0f};
		float m_desired_gait_velocity{0.0f};

        Vector3 m_simulation_position;
        Vector3 m_simulation_velocity;
        Vector3 m_simulation_acceleration;
        Quaternion m_simulation_rotation;
        Vector3 m_simulation_angular_velocity;

		float m_simulation_velocity_halflife{0.27f};
		float m_simulation_rotation_halflife{0.27f};

		// All speeds in m/s
		float m_simulation_run_fwrd_speed{4.0f};
		float m_simulation_run_side_speed{3.0f};
		float m_simulation_run_back_speed{2.5f};

		float m_simulation_walk_fwrd_speed{1.75f};
		float m_simulation_walk_side_speed{1.5f};
		float m_simulation_walk_back_speed{1.25f};

		Array1D<Vector3> m_trajectory_desired_velocities;
		Array1D<Quaternion> m_trajectory_desired_rotations;
		Array1D<Vector3> m_trajectory_positions;
		Array1D<Vector3> m_trajectory_velocities;
		Array1D<Vector3> m_trajectory_accelerations;
		Array1D<Quaternion> m_trajectory_rotations;
		Array1D<Vector3> m_trajectory_angular_velocities;

		//lmm
        CNeuralNetwork m_decompressor;
        Array2D<float> m_latent;
        bool           m_lmm_enable {true};


		void BuildMatchingFeature(CDataBase& db,
		                          const float feature_weight_foot_position,
		                          const float feature_weight_foot_velocity,
		                          const float feature_weight_hip_velocity,
		                          const float feature_weight_trajectory_positions,
		                          const float feature_weight_trajectory_directions);
		void ComputeBonePositionFeature(CDataBase& db, int& offset, int bone, float weight = 1.0f);
		void ComputeBoneVelocityFeature(CDataBase& db, int& offset, int bone, float weight = 1.0f);
		void ComputeTrajectoryPositionFeature(CDataBase& db, int& offset, float weight = 1.0f);
		void ComputeTrajectoryDirectionFeature(CDataBase& db, int& offset, float weight = 1.0f);

		void DataBaseBuildBounds(CDataBase& db);
		void RecursiveForwardKinematics(Vector3& bone_position,
		                                Quaternion& bone_rotation,
		                                const Slice1D<Vector3> bone_positions,
		                                const Slice1D<Quaternion> bone_rotations,
		                                const Slice1D<int> bone_parents,
		                                const int bone);
        void NormalizeFeature(Slice2D<float> features,
                              Slice1D<float> features_offset,
                              Slice1D<float> features_scale,
		                      const int offset,
		                      const int size,
		                      const float weight);


		void RecursiveForwardKinematicsVelocity(Vector3& bone_position,
		                                        Vector3& bone_velocity,
		                                        Quaternion& bone_rotation,
		                                        Vector3& bone_angular_velocity,
		                                        const Slice1D<Vector3> bone_positions,
		                                        const Slice1D<Vector3> bone_velocities,
		                                        const Slice1D<Quaternion> bone_rotations,
		                                        const Slice1D<Vector3> bone_angular_velocities,
		                                        const Slice1D<int> bone_parents,
		                                        const int bone);

		int DatabaseTrajectoryIndexClamp(CDataBase& db, int frame, int offset);
		void trajectory_desired_rotations_predict(Slice1D<Quaternion> desired_rotations,
		                                          const Slice1D<Vector3> desired_velocities,
		                                          const Quaternion desired_rotation,
		                                          const float camera_azimuth,
		                                          const Vector3 gamepadstick_left,
		                                          const Vector3 gamepadstick_right,
		                                          const bool desired_strafe,
		                                          const float dt);
		float orbit_camera_update_azimuth(float azimuth, Vector3 gamepadstick_right, bool desired_strafe, float dt);
		void trajectory_rotations_predict(Slice1D<Quaternion> rotations,
		                                  Slice1D<Vector3> angular_velocities,
		                                  const Quaternion rotation,
		                                  const Vector3 angular_velocity,
		                                  const Slice1D<Quaternion> desired_rotations,
		                                  const float halflife,
		                                  const float dt);
		void simulation_rotations_update(Quaternion& rotation,
		                                 Vector3& angular_velocity,
		                                 const Quaternion desired_rotation,
		                                 const float halflife,
		                                 const float dt);
		void trajectory_desired_velocities_predict(Slice1D<Vector3> desired_velocities,
		                                           const Slice1D<Quaternion> trajectory_rotations,
		                                           const Vector3 desired_velocity,
		                                           const float camera_azimuth,
		                                           const Vector3 gamepadstick_left,
		                                           const Vector3 gamepadstick_right,
		                                           const bool desired_strafe,
		                                           const float fwrd_speed,
		                                           const float side_speed,
		                                           const float back_speed,
		                                           const float dt);

		void trajectory_positions_predict(Slice1D<Vector3> positions,
		                                  Slice1D<Vector3> velocities,
		                                  Slice1D<Vector3> accelerations,
		                                  const Vector3 position,
		                                  const Vector3 velocity,
		                                  const Vector3 acceleration,
		                                  const Slice1D<Vector3> desired_velocities,
		                                  const float halflife,
		                                  const float dt);

		void simulation_positions_update(Vector3& position,
		                                 Vector3& velocity,
		                                 Vector3& acceleration,
		                                 const Vector3 desired_velocity,
		                                 const float halflife,
		                                 const float dt);
		void query_copy_denormalized_feature(Slice1D<float> query,
		                                     int& offset,
		                                     const int size,
		                                     const Slice1D<float> features,
		                                     const Slice1D<float> features_offset,
		                                     const Slice1D<float> features_scale);

		void query_compute_trajectory_position_feature(Slice1D<float> query,
		                                               int& offset,
		                                               const Vector3 root_position,
		                                               const Quaternion root_rotation,
		                                               const Slice1D<Vector3> trajectory_positions);

		void query_compute_trajectory_direction_feature(Slice1D<float> query,
		                                                int& offset,
		                                                const Quaternion root_rotation,
		                                                const Slice1D<Quaternion> trajectory_rotations);
		int database_trajectory_index_clamp(CDataBase& db, int frame, int offset);
		void forward_kinematic_full();
		void inertialize_pose_reset(Slice1D<Vector3> bone_offset_positions,
		                            Slice1D<Vector3> bone_offset_velocities,
		                            Slice1D<Quaternion> bone_offset_rotations,
		                            Slice1D<Vector3> bone_offset_angular_velocities,
		                            Vector3& transition_src_position,
		                            Quaternion& transition_src_rotation,
		                            Vector3& transition_dst_position,
		                            Quaternion& transition_dst_rotation,
		                            const Vector3 root_position,
		                            const Quaternion root_rotation);
		void inertialize_pose_transition(Slice1D<Vector3> bone_offset_positions,
		                                 Slice1D<Vector3> bone_offset_velocities,
		                                 Slice1D<Quaternion> bone_offset_rotations,
		                                 Slice1D<Vector3> bone_offset_angular_velocities,
		                                 Vector3& transition_src_position,
		                                 Quaternion& transition_src_rotation,
		                                 Vector3& transition_dst_position,
		                                 Quaternion& transition_dst_rotation,
		                                 const Vector3 root_position,
		                                 const Vector3 root_velocity,
		                                 const Quaternion root_rotation,
		                                 const Vector3 root_angular_velocity,
		                                 const Slice1D<Vector3> bone_src_positions,
		                                 const Slice1D<Vector3> bone_src_velocities,
		                                 const Slice1D<Quaternion> bone_src_rotations,
		                                 const Slice1D<Vector3> bone_src_angular_velocities,
		                                 const Slice1D<Vector3> bone_dst_positions,
		                                 const Slice1D<Vector3> bone_dst_velocities,
		                                 const Slice1D<Quaternion> bone_dst_rotations,
		                                 const Slice1D<Vector3> bone_dst_angular_velocities);

		void inertialize_pose_update(Slice1D<Vector3> bone_positions,
		                             Slice1D<Vector3> bone_velocities,
		                             Slice1D<Quaternion> bone_rotations,
		                             Slice1D<Vector3> bone_angular_velocities,
		                             Slice1D<Vector3> bone_offset_positions,
		                             Slice1D<Vector3> bone_offset_velocities,
		                             Slice1D<Quaternion> bone_offset_rotations,
		                             Slice1D<Vector3> bone_offset_angular_velocities,
		                             const Slice1D<Vector3> bone_input_positions,
		                             const Slice1D<Vector3> bone_input_velocities,
		                             const Slice1D<Quaternion> bone_input_rotations,
		                             const Slice1D<Vector3> bone_input_angular_velocities,
		                             const Vector3 transition_src_position,
		                             const Quaternion transition_src_rotation,
		                             const Vector3 transition_dst_position,
		                             const Quaternion transition_dst_rotation,
		                             const float halflife,
		                             const float dt);
        void denormalize_features(Slice1D<float>       features,
                                  const Slice1D<float> features_offset,
                                  const Slice1D<float> features_scale);
        void DoRetargeting();
	public:
// #if _DEBUG
		// std::vector<Piccolo::Vector3> m_current_feature_positions;
		// std::vector<Piccolo::Quaternion> m_current_feature_rotations;
		// Piccolo::Quaternion m_current_root_rotation{Piccolo::Quaternion::IDENTITY};
		//
		// std::vector<Piccolo::Vector3> m_matched_feature_positions;
		// std::vector<Piccolo::Vector3> m_matched_feature_direction;
// #endif
	};
}
