#pragma once

#include "runtime/core/math/math.h"
#include "runtime/core/math/transform.h"

#include "runtime/resource/res_type/data/skeleton_data.h"

namespace Piccolo
{
    class Node
    {
    public:
        // Enumeration denoting the spaces which a transform can be relative to.
        enum class TransformSpace
        {
            /// Transform is relative to the local space
            LOCAL,
            /// Transform is relative to the space of the parent pNode
            AREN,
            /// Transform is relative to object space
            OBJECT
        };
#ifdef _DEBUG
    public:
#else
    protected:
#endif
        Node* m_parent {nullptr};

        std::string m_name;

        /// Stores the orientation/position/scale of the pNode relative to it's parent.
        Quaternion m_orientation {Quaternion::IDENTITY};
        Vector3    m_position {Vector3::ZERO};
        Vector3    m_scale {Vector3::UNIT_SCALE};

        // Cached combined orientation/position/scale.
        Quaternion m_derived_orientation {Quaternion::IDENTITY};
        Vector3    m_derived_position {Vector3::ZERO};
        Vector3    m_derived_scale {Vector3::UNIT_SCALE};

        /// The position/orientation/scale to use as a base for keyframe animation
        Vector3    m_initial_position {Vector3::ZERO};
        Quaternion m_initial_orientation {Quaternion::IDENTITY};
        Vector3    m_initial_scale {Vector3::UNIT_SCALE};

        Matrix4x4 m_inverse_Tpose;

        bool m_is_dirty {true};

    protected:
        /// Only available internally - notification of parent.
        virtual void setParent(Node* parent);

    public:
        Node(const std::string name);
        virtual ~Node();
        void               clear();
        const std::string& getName(void) const;
        virtual Node*      getParent(void) const;

        virtual const Quaternion& getOrientation() const;

        virtual void setOrientation(const Quaternion& q);
        virtual void resetOrientation(void);

        virtual void           setPosition(const Vector3& pos);
        virtual const Vector3& getPosition(void) const;

        virtual void           setScale(const Vector3& scale);
        virtual const Vector3& getScale(void) const;

        virtual void scale(const Vector3& scale);

        // Triggers the pNode to update it's combined transforms.
        virtual void updateDerivedTransform(void);

        virtual void translate(const Vector3& d, TransformSpace relativeTo = TransformSpace::AREN);

        // Rotate the pNode around an aritrary axis using a Quarternion.
        virtual void rotate(const Quaternion& q, TransformSpace relativeTo = TransformSpace::LOCAL);

        // Gets the orientation of the pNode as derived from all parents.
        virtual const Quaternion& _getDerivedOrientation(void) const;
        virtual const Vector3&    _getDerivedPosition(void) const;
        virtual const Vector3&    _getDerivedScale(void) const;
        virtual const Matrix4x4&  _getInverseTpose(void) const;

        // dirty and update
        virtual bool isDirty() const;
        virtual void setDirty();
        virtual void update();

        virtual void setAsInitialPose(void);
        virtual void resetToInitialPose(void);

        virtual const Vector3&    getInitialPosition(void) const;
        virtual const Quaternion& getInitialOrientation(void) const;
        virtual const Vector3&    getInitialScale(void) const;
    };

    class Bone : public Node
    {
        friend class LoDSkeleton;

    protected:
        std::shared_ptr<RawBone> m_definition {};
        // physics simulation and actor status

    public:
        Bone();
        void initialize(std::shared_ptr<RawBone> definition, Bone* parent_bone);

        size_t getID(void) const;
    };
} // namespace Piccolo
