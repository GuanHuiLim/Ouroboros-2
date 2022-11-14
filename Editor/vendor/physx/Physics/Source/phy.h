/************************************************************************************//*!
\file           phy.h
\project        Physics
\author         Ting Shu Mei, shumei.ting, 620003420 | code contribution (100%)
\par            email: shumei.ting\@digipen.edu
\date           Oct 02, 2022
\brief          Using of NVIDIA PhysX Library to build up the Physics System
                including Dynamics, Collision Detection, Reaction.
                Physx Github: https://github.com/NVIDIAGameWorks/PhysX
                Physx Documentation: https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Index.html

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

#pragma once

#pragma warning( push )
#pragma warning( disable : 26812 )
#pragma warning( disable : 26495 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 33010 )
#include <Physics/Physx/include/PxPhysicsAPI.h>
#pragma warning( pop )

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <deque>
#include <memory>
//#include <glm/glm.hpp>

#include "uuid.h"

//#define PVD_DEBUGGER false

using namespace physx;

namespace myPhysx {

    class Collision;

    class PhysxWorld;
    class PVD;
    struct PhysxObject;
    struct PhysicsObject;

    enum class rigid { none, rstatic, rdynamic };
    enum class shape { none, box, sphere, capsule, plane };
    enum class force { force, acceleration, impulse, velocityChanged };
    enum class trigger { none, onTriggerEnter, onTriggerStay, onTriggerExit};
    enum class collision { none, onCollisionEnter, onCollisionStay, onCollisionExit};
    
    
    class EventCallBack : public PxSimulationEventCallback {

    public:
        void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override;

        void onWake(PxActor** actors, PxU32 count) override;

        void onSleep(PxActor** actors, PxU32 count) override;

        void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 count) override;

        void onTrigger(PxTriggerPair* pairs, PxU32 count) override;

        void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override;
    };

    struct LockingAxis {

        bool x_axis;
        bool y_axis;
        bool z_axis;
    };

    struct ContactPoint {

        PxVec3 normal;
        PxVec3 point;
        PxVec3 impulse;
    };

    struct ContactManifold {

        phy_uuid::UUID shape1_ID;
        phy_uuid::UUID shape2_ID;

        collision status = collision::none;

        std::vector<ContactPoint> m_contactPoint;
        PxU8 contactCount;
    };

    struct TriggerManifold {

        phy_uuid::UUID triggerID;
        phy_uuid::UUID otherID;

        trigger status = trigger::none;
        bool passingAway = false;

        //bool isStaying = false;
    };

    struct RigidBody {

        PxRigidDynamic* rigidDynamic = nullptr;
        PxRigidStatic* rigidStatic = nullptr;
    };

    // unprotected class
    struct Material {

        PxReal staticFriction;
        PxReal dynamicFriction;
        PxReal restitution;
    };

    // backend holds the overall info of the entire physics engine
    namespace physx_system {

        static PhysxWorld* currentWorld;

        void init();

        void shutdown();

        PxFoundation* createFoundation();

        PxPhysics* createPhysics();

        PxFoundation* getFoundation();

        PxPhysics* getPhysics();

        bool isTrigger(const PxFilterData& data);

        bool isTriggerShape(PxShape* shape);

        void provideCurrentWorld(PhysxWorld* world);
        
        PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
                                                PxFilterObjectAttributes attributes1, PxFilterData filterData1,
                                                PxPairFlags& pairFlags, const void* constantBlock,
                                                PxU32 constantBlockSize);

        void setupFiltering(PxShape* shape);
    };

    // describes a physics scene
    class PhysxWorld {

    private:

        friend struct PhysicsObject;

        PxScene* scene = nullptr;
        std::map<phy_uuid::UUID, PxMaterial*> mat;
        PxVec3 gravity;

        std::map<phy_uuid::UUID, int> all_objects; // store all the index of the objects (lookups for keys / check if empty)

        std::vector<PhysxObject> m_objects; // to iterate through for setting the data
        
        std::queue<TriggerManifold> m_triggerCollisionPairs; // queue to store the trigger collision pairs

        std::queue<ContactManifold> m_collisionPairs; // queue to store the collision pairs

    public:

        // SCENE
        PhysxWorld(PxVec3 gravity);
        ~PhysxWorld();
        void updateScene(float dt);

        // GRAVITY
        PxVec3 getWorldGravity() const;
        void setWorldGravity(PxVec3 gra);

        // RIGIDBODY
        PhysicsObject createInstance();
        void removeInstance(PhysicsObject obj);

        // MAP OF OBJECTS
        std::map<phy_uuid::UUID, int>* getAllObject();

        // TRIGGER
        void updateTriggerState(phy_uuid::UUID id); // function to update objects for OnTriggerStay
        std::queue<TriggerManifold>* getTriggerData(); // function to retrieve the trigger queue data
        void clearTriggerData(); // function to reset the trigger queue data

        // COLLISION
        std::queue<ContactManifold>* getCollisionData(); // function to retrieve the collision queue data
        void clearCollisionData(); // function to reset the collision queue data

    };

    // associated to each object in the physics world (me store)
    struct PhysxObject {

        std::unique_ptr<phy_uuid::UUID> id = nullptr;
        phy_uuid::UUID matID = 0;

        // shape
        PxShape* m_shape = nullptr; // prob no need this
        shape shape = shape::none;

        // ensure at least static or dynamic is init
        RigidBody rb{};
        rigid rigidID = rigid::none;

        // lock and unlock pos/rot axis
        LockingAxis lockPositionAxis{ false };
        LockingAxis lockRotationAxis{ false };

        bool trigger = false;
        bool gravity = true; // static should be false
        bool kinematic = false;
        bool collider = true;
    };

    struct PhysicsObject { // you store

        phy_uuid::UUID id;
        PhysxWorld* world;

        // GETTERS
        LockingAxis getLockPositionAxis() const;
        LockingAxis getLockRotationAxis() const;
        Material getMaterial() const;
        PxVec3 getposition() const;
        PxQuat getOrientation() const;

        PxReal getMass() const;
        PxReal getInvMass() const;
        PxReal getAngularDamping() const;
        PxVec3 getAngularVelocity() const;
        PxReal getLinearDamping() const;
        PxVec3 getLinearVelocity() const;

        bool isTrigger() const;
        bool useGravity() const;
        bool isKinematic() const;
        bool isColliderEnabled() const;

        // SETTERS
        void setRigidType(rigid type);
        void setMaterial(Material material);
        void setPosOrientation(PxVec3 pos, PxQuat quat);

        void setMass(PxReal mass);
        void setMassSpaceInertia(PxVec3 mass);
        void setAngularDamping(PxReal angularDamping);
        void setAngularVelocity(PxVec3 angularVelocity);
        void setLinearDamping(PxReal linearDamping);
        void setLinearVelocity(PxVec3 linearVelocity);

        void disableGravity(bool gravity);
        void enableKinematic(bool kine);
        void enableCollider(bool collide);

        // AXIS LOCKING
        void lockPositionX(bool lock);
        void lockPositionY(bool lock);
        void lockPositionZ(bool lock);

        void lockRotationX(bool lock);
        void lockRotationY(bool lock);
        void lockRotationZ(bool lock);

        // TRIGGERS
        void setTriggerShape(bool trigger);
        
        // FORCE
        void addForce(PxVec3 f_amount, force f);
        void addTorque(PxVec3 f_amount, force f);

        // set default value for each type of shape & can change shape too
        template<typename Type>
        void reAttachShape(rigid rigidType, Type data);

        void setShape(shape shape);
        void removeShape();

        // change each individual property based on its shape
        void setBoxProperty(float halfextent_width, float halfextent_height, float halfextent_depth);
        void setSphereProperty(float radius);
        void setCapsuleProperty(float radius, float halfHeight);
        //void setPlaneProperty(float radius);

    };


    // Physx visual degguer
    class PVD {

    private:

        PxPvd* mPVD;

        PxPvdTransport* mTransport;

    public:

        PxPvd* createPvd(PxFoundation* foundation, const char* ip);

        void setupPvd(PxScene* scene);

        PxPvdTransport* getTransport();

        PxPvd*& pvd__();

        PxPvd* const& pvd__() const;
    };
};