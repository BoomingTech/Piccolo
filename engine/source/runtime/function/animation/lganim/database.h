#pragma once
#include "array.h"
#include "common.h"
#include "runtime/core/math/vector3.h"
#include <cassert>
#include <vector>

namespace Piccolo
{
    enum
    {
        BOUND_SM_SIZE = 16,
        BOUND_LR_SIZE = 64,
    };
    struct CDataBase
    {
        Array2D<Vector3>    m_bone_positions;
        Array2D<Vector3>    m_bone_velocities;
        Array2D<Quaternion> m_bone_rotations;
        Array2D<Vector3>    m_bone_angular_velocities;
        Array1D<int>        m_bone_parents;

        Array1D<int>  m_range_starts;
        Array1D<int>  m_range_stops;
        Array2D<bool> m_contact_states;

        Array2D<float> m_features;
        Array1D<float> m_feature_offset;
        Array1D<float> m_feature_scale;

        uint32_t GetNumFrames() const { return m_bone_positions.m_rows; }
        uint32_t GetNumBones() const { return m_bone_positions.m_cols; }
        uint32_t GetNumRanges() const { return static_cast<uint32_t>(m_range_starts.size); }
        uint32_t GetNumFeatures() const { return m_features.m_cols; }
        uint32_t GetNumContacts() const { return m_contact_states.m_cols; }

        Array2D<float> bound_sm_min;
        Array2D<float> bound_sm_max;
        Array2D<float> bound_lr_min;
        Array2D<float> bound_lr_max;
    };

    inline void BuildDataBase() {}

    inline void LoadDataBase(CDataBase& db, const std::string& file_name)
    {
        FILE*   f;
        errno_t r = fopen_s(&f, file_name.c_str(), "rb");
        assert(f != NULL);

        array2d_read(db.m_bone_positions, f);
        array2d_read(db.m_bone_velocities, f);
        array2d_read(db.m_bone_rotations, f);
        array2d_read(db.m_bone_angular_velocities, f);
        array1d_read(db.m_bone_parents, f);

        array1d_read(db.m_range_starts, f);
        array1d_read(db.m_range_stops, f);

        array2d_read(db.m_contact_states, f);

        r = fclose(f);
    }

    inline void motion_matching_search(int&                 best_index,
                                       float&               best_cost,
                                       const Slice1D<int>   range_starts,
                                       const Slice1D<int>   range_stops,
                                       const Slice2D<float> features,
                                       const Slice1D<float> features_offset,
                                       const Slice1D<float> features_scale,
                                       const Slice2D<float> bound_sm_min,
                                       const Slice2D<float> bound_sm_max,
                                       const Slice2D<float> bound_lr_min,
                                       const Slice2D<float> bound_lr_max,
                                       const Slice1D<float> query_normalized,
                                       const float          transition_cost,
                                       const int            ignore_range_end,
                                       const int            ignore_surrounding)
    {
        int nfeatures = query_normalized.size;
        int nranges   = range_starts.size;

        int curr_index = best_index;

        // Find cost for current frame
        if (best_index != -1)
        {
            best_cost = 0.0;
            for (int i = 0; i < nfeatures; i++)
            {
                best_cost += squaref(query_normalized(i) - features(best_index, i));
            }
        }

        float curr_cost = 0.0f;

        // Search rest of database
        for (int r = 0; r < nranges; r++)
        {
            // Exclude end of ranges from search
            int i         = range_starts(r);
            int range_end = range_stops(r) - ignore_range_end;

            while (i < range_end)
            {
                // Find index of current and next large box
                int i_lr      = i / BOUND_LR_SIZE;
                int i_lr_next = (i_lr + 1) * BOUND_LR_SIZE;

                // Find distance to box
                curr_cost = transition_cost;
                for (int j = 0; j < nfeatures; j++)
                {
                    curr_cost += squaref(query_normalized(j) -
                                         clampf(query_normalized(j), bound_lr_min(i_lr, j), bound_lr_max(i_lr, j)));

                    if (curr_cost >= best_cost)
                    {
                        break;
                    }
                }

                // If distance is greater than current best jump to next box
                if (curr_cost >= best_cost)
                {
                    i = i_lr_next;
                    continue;
                }

                // Check against small box
                while (i < i_lr_next && i < range_end)
                {
                    // Find index of current and next small box
                    int i_sm      = i / BOUND_SM_SIZE;
                    int i_sm_next = (i_sm + 1) * BOUND_SM_SIZE;

                    // Find distance to box
                    curr_cost = transition_cost;
                    for (int j = 0; j < nfeatures; j++)
                    {
                        curr_cost += squaref(query_normalized(j) -
                                             clampf(query_normalized(j), bound_sm_min(i_sm, j), bound_sm_max(i_sm, j)));

                        if (curr_cost >= best_cost)
                        {
                            break;
                        }
                    }

                    // If distance is greater than current best jump to next box
                    if (curr_cost >= best_cost)
                    {
                        i = i_sm_next;
                        continue;
                    }

                    // Search inside small box
                    while (i < i_sm_next && i < range_end)
                    {
                        // Skip surrounding frames
                        if (curr_index != -1 && abs(i - curr_index) < ignore_surrounding)
                        {
                            i++;
                            continue;
                        }

                        // Check against each frame inside small box
                        curr_cost = transition_cost;
                        for (int j = 0; j < nfeatures; j++)
                        {
                            curr_cost += squaref(query_normalized(j) - features(i, j));
                            if (curr_cost >= best_cost)
                            {
                                break;
                            }
                        }

                        // If cost is lower than current best then update best
                        if (curr_cost < best_cost)
                        {
                            best_index = i;
                            best_cost  = curr_cost;
                        }

                        i++;
                    }
                }
            }
        }
    }

    // Search database
    inline void database_search(int&                 best_index,
                                float&               best_cost,
                                const CDataBase&     db,
                                const Slice1D<float> query,
                                const float          transition_cost    = 0.0f,
                                const int            ignore_range_end   = 20,
                                const int            ignore_surrounding = 20)
    {
        // Normalize Query
        Array1D<float> query_normalized(db.GetNumFeatures());
        for (int i = 0; i < db.GetNumFeatures(); i++)
        {
            query_normalized(i) = (query(i) - db.m_feature_offset(i)) / db.m_feature_scale(i);
        }

        // Search
        motion_matching_search(best_index,
                               best_cost,
                               db.m_range_starts,
                               db.m_range_stops,
                               db.m_features,
                               db.m_feature_offset,
                               db.m_feature_scale,
                               db.bound_sm_min,
                               db.bound_sm_max,
                               db.bound_lr_min,
                               db.bound_lr_max,
                               query_normalized,
                               transition_cost,
                               ignore_range_end,
                               ignore_surrounding);
    }
} // namespace piccolo
