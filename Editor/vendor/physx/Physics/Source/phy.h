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
#include <cassert>
#include <unordered_map>
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
    struct RaycastHit;

    enum class rigid { none, rstatic, rdynamic };
    enum class shape { none, box, sphere, capsule, plane, convex };
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

    struct FilterGroup
    {
        enum Enum
        {
            Zero = (1 << 0),
            One = (1 << 1), 
            Two = (1 << 2),
            Three = (1 << 3),
            Four = (1 << 4),
            Fix = (1 << 5),
            Six = (1 << 6),
            Seven = (1 << 7),
            All = Zero | One | Two | Three | Four | Fix | Six | Seven // New value that combines all filter groups
        };
    };

    struct RaycastHit {

        bool intersect = false;
        
        phy_uuid::UUID object_ID;

        PxVec3 position;
        PxVec3 normal;
        PxF32 distance;
    };

    struct LockingAxis {

        bool x_axis = false;
        bool y_axis = false;
        bool z_axis = false;
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

    struct MeshVetices {

        PxReal x;
        PxReal y;
        PxReal z;
    };

    struct PhysicsCommand
    {
        phy_uuid::UUID Id;

        PxVec3 Force = PxVec3{};
        force Type = force::force;

        bool AddForce = false;
        bool AddTorque = false;
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

        void setCurrentWorld(PhysxWorld* world);
        
        PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
                                                PxFilterObjectAttributes attributes1, PxFilterData filterData1,
                                                PxPairFlags& pairFlags, const void* constantBlock,
                                                PxU32 constantBlockSize);

        void setupFiltering(PxShape* shape, PxU32 filterGroup, PxU32 filterMask);
    };

    // describes a physics scene
    class PhysxWorld {

    private:

        // do i still need this?
        friend struct PhysxObject;
        friend struct PhysicsObject;

        PxScene* scene = nullptr;
        std::map<phy_uuid::UUID, PxMaterial*> mat;
        PxVec3 gravity;

        std::map<phy_uuid::UUID, std::size_t> all_objects; // store all the index of the objects (lookups for keys / check if empty)

        std::vector<PhysxObject> m_objects; // to iterate through for setting the data
        
        std::queue<TriggerManifold> m_triggerCollisionPairs; // queue to store the trigger collision pairs

        std::queue<ContactManifold> m_collisionPairs; // queue to store the collision pairs
        
        std::vector<PxVec3> m_meshVertices{ PxVec3(0,0,0),PxVec3(0,0,0),PxVec3(0,0,0) }; // vector to store the mesh vertices


        // map of existing physics objects that are created
        std::unordered_map<phy_uuid::UUID, PhysicsObject> m_latestPhysicsObjects{};

    public:

        // SCENE
        PhysxWorld(PxVec3 gravity = PxVec3(0.0f, -9.81f, 0.0f));
        ~PhysxWorld();
        void updateScene(float dt);
        void updateInternal();

        // GRAVITY
        PxVec3 getWorldGravity() const;
        void setWorldGravity(PxVec3 gra);

        // RIGIDBODY
        PhysicsObject createInstance();
        void removeInstance(PhysicsObject obj);

        // DUPLICATE OBJECT
        PhysicsObject duplicateObject(phy_uuid::UUID id);
        //void setAllOldData(PhysicsObject& physicsObj, PhysxObject& iniObj, size_t index);

        // MAP OF OBJECTS
        std::map<phy_uuid::UUID, std::size_t>* getAllObject();
        bool hasObject(phy_uuid::UUID id) const;

        // SWEEP
        RaycastHit sweep(PxTransform initialPose, PxVec3 direction, PxReal distance);
        
        // RAYCAST
        RaycastHit raycast(PxVec3 origin, PxVec3 direction, PxReal distance);
        RaycastHit raycast(PxVec3 origin, PxVec3 direction, PxReal distance, std::uint32_t filter /*= FilterGroup::All*/);
        
        std::vector<RaycastHit> raycastAll(PxVec3 origin, PxVec3 direction, PxReal distance);
        std::vector<RaycastHit> raycastAll(PxVec3 origin, PxVec3 direction, PxReal distance, std::uint32_t filter /*= FilterGroup::All*/);

        // TRIGGER
        void updateTriggerState(phy_uuid::UUID id); // function to update objects for OnTriggerStay
        std::queue<TriggerManifold>* getTriggerData(); // function to retrieve the trigger queue data
        void clearTriggerData(); // function to reset the trigger queue data

        // COLLISION
        std::queue<ContactManifold>* getCollisionData(); // function to retrieve the collision queue data
        void clearCollisionData(); // function to reset the collision queue data

        // NEW KEY FUNCTIONS!
        std::unordered_map<phy_uuid::UUID, PhysicsObject> retrieveCurrentObjects() const;
        void submitUpdatedObjects(std::vector<PhysicsObject> updatedObjects);

        void submitPhysicsCommand(std::vector<PhysicsCommand> physicsCommand);

        // helper functions
        void retrieveOldData(PhysicsObject& physics_Obj, const PhysxObject& init_Obj) const;

        template<typename Type>
        void retrievePosOri(PhysicsObject& physics_Obj, Type data) const;

        void setAllData(PhysicsObject const& updatedPhysicsObj, PhysxObject& underlying_Obj, bool duplicate);

        void setShape(PhysicsObject const& updated_Obj, PhysxObject& underlying_Obj, PxRigidActor* underlying_rigidbody, bool duplicate);

        void setForce(PhysxObject& underlying_Obj, PhysicsCommand& command_Obj);

        void setTorque(PhysxObject& underlying_Obj, PhysicsCommand& command_Obj);
    
        PxConvexMesh* createConvexMesh(std::vector<PxVec3> vert);
    };

    // This should be the interface object that others using the physics system will use.
    // associated to each object in the physics world (me store)
    struct PhysxObject {

        PhysxObject() = default; // default constructor
        PhysxObject(const PhysxObject& object); // copy constructor
        PhysxObject& operator=(const PhysxObject& object);
        PhysxObject(PhysxObject&& object) noexcept = default;
        PhysxObject& operator=(PhysxObject&& object) = default;

        std::unique_ptr<phy_uuid::UUID> id = nullptr;

        PxMaterial* m_material = nullptr; 
        //phy_uuid::UUID matID = 0;

        // shape
        PxShape* m_shape = nullptr;
        shape shape_type = shape::none;

        // ensure at least static or dynamic is init
        RigidBody rb{};
        rigid rigid_type = rigid::rstatic;

        // lock and unlock pos/rot axis
        LockingAxis lockPositionAxis{};
        LockingAxis lockRotationAxis{};

        bool is_trigger = false;
        bool gravity_enabled = false; // static should be false
        bool is_kinematic = false;
        bool is_collider = true;

        // Filtering
        std::uint32_t filterIn = FilterGroup::Zero;
        std::uint32_t filterOut = FilterGroup::All;

        bool changeVertices = false;
        PxVec3 meshScale = PxVec3(1, 1, 1);
        std::vector<PxVec3> uploadVertices{};
        std::vector<PxVec3> meshVertices{ PxVec3(0,0,0), PxVec3(0,0,0), PxVec3(0,0,0) };
    };

    struct PhysicsObject { // you store

        phy_uuid::UUID id;

        // NEW ADDED
        Material material = Material{.4f,.2f,.0f};

        PxVec3 position = {0,0,0};
        PxQuat orientation = {0,0,0,1};

        PxReal mass = 0, invmass = 0;
        PxReal linearDamping = 0.00, angularDamping = 0.00;

        PxVec3 linearVel = { 0,0,0 }, angularVel = { 0,0,0 };

        shape shape_type = shape::none;

        PxBoxGeometry box;
        PxSphereGeometry sphere;
        PxPlaneGeometry plane;
        PxCapsuleGeometry capsule;
        PxConvexMeshGeometry convex;

        rigid rigid_type = rigid::rstatic;

        // lock and unlock pos/rot axis
        LockingAxis lockPositionAxis{};
        LockingAxis lockRotationAxis{};

        bool is_trigger = false;
        bool gravity_enabled = false; // static should be false
        bool is_kinematic = false;
        bool is_collider = true;

        // Filtering
        std::uint32_t filterIn = FilterGroup::Zero;
        std::uint32_t filterOut = FilterGroup::All;

        bool changeVertices = false;
        PxVec3 meshScale = PxVec3(1, 1, 1);
        std::vector<PxVec3> uploadVertices{};
        std::vector<PxVec3> meshVertices{ PxVec3(0,0,0),PxVec3(0,0,0),PxVec3(0,0,0) };
    };

    static constexpr std::size_t sizeofPhysicsObject = sizeof(PhysicsObject);

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