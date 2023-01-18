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
#include <unordered_map>

#include "uuid.h"

using namespace physx;

namespace phy
{
    class Collision;
    class PhysicsWorld;
    class PVD;

    struct PhysxObject;
    struct PhysicsObject;
    struct RaycastHit;

    enum class rigid { /*none, */ rstatic, rdynamic };
    enum class shape { none, box, sphere, capsule, plane };
    enum class force { force, acceleration, impulse, velocityChanged };
    enum class trigger { none, onTriggerEnter, onTriggerStay, onTriggerExit };
    enum class collision { none, onCollisionEnter, onCollisionStay, onCollisionExit };


    class EventCallBack : public PxSimulationEventCallback 
    {
    public:
        void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override;

        void onWake(PxActor** actors, PxU32 count) override;

        void onSleep(PxActor** actors, PxU32 count) override;

        void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 count) override;

        void onTrigger(PxTriggerPair* pairs, PxU32 count) override;

        void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override;
    };

    struct RaycastHit 
    {
        bool intersect = false;

        phy_uuid::UUID object_ID;

        PxVec3 position;
        PxVec3 normal;
        PxF32 distance;
    };

    struct LockingAxis 
    {
        bool x_axis = false;
        bool y_axis = false;
        bool z_axis = false;
    };

    struct ContactPoint 
    {
        PxVec3 normal;
        PxVec3 point;
        PxVec3 impulse;
    };

    struct ContactManifold 
    {
        phy_uuid::UUID shape1_ID;
        phy_uuid::UUID shape2_ID;

        collision status = collision::none;

        std::vector<ContactPoint> m_contactPoint;
        PxU8 contactCount;
    };

    struct TriggerManifold 
    {
        phy_uuid::UUID triggerID;
        phy_uuid::UUID otherID;

        trigger status = trigger::none;
        bool passingAway = false;

        //bool isStaying = false;
    };

    struct RigidBody 
    {
        PxRigidDynamic* rigidDynamic = nullptr;
        PxRigidStatic* rigidStatic = nullptr;
    };

    struct Material 
    {
        PxReal staticFriction;
        PxReal dynamicFriction;
        PxReal restitution;
    };

    // backend holds the overall info of the entire physics engine
    namespace physx_system 
    {
        static PhysicsWorld* currentWorld;

        void init();

        void shutdown();

        PxFoundation* createFoundation();

        PxPhysics* createPhysics();

        PxFoundation* getFoundation();

        PxPhysics* getPhysics();

        bool isTrigger(const PxFilterData& data);

        bool isTriggerShape(PxShape* shape);

        void setCurrentWorld(PhysicsWorld* world);

        PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
            PxFilterObjectAttributes attributes1, PxFilterData filterData1,
            PxPairFlags& pairFlags, const void* constantBlock,
            PxU32 constantBlockSize);

        void setupFiltering(PxShape* shape);
    };

    // This should be the interface object that others using the physics system will use.
    // associated to each object in the physics world (me store)
    struct PhysicsObject
    {
        phy_uuid::UUID id;
        Material material = Material{};

        PxVec3 position;
        PxQuat orientation;

        PxReal mass, invmass;
        PxReal linearDamping, angularDamping;

        PxVec3 linearVel, angularVel;

        shape shape_type = shape::none;
        
        PxBoxGeometry box;
        PxSphereGeometry sphere;
        PxPlaneGeometry plane;
        PxCapsuleGeometry capsule;
        
        rigid rigid_type = rigid::rstatic;

        LockingAxis lockPositionAxis{};
        LockingAxis lockRotationAxis{};

        bool is_trigger = false;
        bool gravity_enabled = true;
        bool is_kinematic = false;
        bool is_collider_enabled = true;
    };

    struct PhysicsCommand
    {
        phy_uuid::UUID Id;
        
        PxVec3 Force = PxVec3{};
        phy::force Type = phy::force::force;

        PxVec3 linearVel, angularVel;

        bool LinearVel = false;
        bool AngularVel = false;
        bool AddForce = false;
        bool AddTorque = false;
    };

    static constexpr std::size_t sizeofPhysicsObject = sizeof(PhysicsObject);

    // This object is the underlying physx object that should only be accessible by the underlying implementation.
    // associated to each object in the physics world. 
    struct PhysxObject
    {
        PhysxObject() = default; // default constructor
        PhysxObject(const PhysxObject& object); // copy constructor
        PhysxObject& operator=(const PhysxObject& object);
        PhysxObject(PhysxObject&& object) = default;
        PhysxObject& operator=(PhysxObject&& object) = default;

        std::unique_ptr<phy_uuid::UUID> id = nullptr;

        // material
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
        bool gravity_enabled = true; // static should be false
        bool is_kinematic = false;
        bool is_collider_enabled = true;
    };

    class PhysicsWorld
    {
    public:
        // SCENE
        PhysicsWorld(PxVec3 gravity = PxVec3(0.0f, -9.81f, 0.0f));
        ~PhysicsWorld();

        void updateWorld(float dt);

        // GRAVITY
        PxVec3 getWorldGravity() const;
        void setWorldGravity(PxVec3 gra);

        // RIGIDBODY
        PhysicsObject createInstance();
        void removeInstance(PhysicsObject obj);

        // DUPLICATE OBJECT
        PhysicsObject duplicateObject(phy_uuid::UUID id);

        // MAP OF OBJECTS
        std::map<phy_uuid::UUID, std::size_t> getLookupTable() const;
        bool hasObject(phy_uuid::UUID id) const;

        // RAYCAST
        RaycastHit raycast(PxVec3 origin, PxVec3 direction, PxReal distance);
        std::vector<RaycastHit> raycastAll(PxVec3 origin, PxVec3 direction, PxReal distance);

        // TRIGGER
        void updateTriggerState(phy_uuid::UUID id); // function to update objects for OnTriggerStay
        std::queue<TriggerManifold> getTriggerData() const; // function to retrieve the trigger queue data
        void clearTriggerData(); // function to reset the trigger queue data

        // COLLISION
        std::queue<ContactManifold> getCollisionData() const; // function to retrieve the collision queue data
        void clearCollisionData(); // function to reset the collision queue data


        // NEW KEY FUNCTIONS!
        std::unordered_map<phy_uuid::UUID, PhysicsObject> retrieveCurrentObjects() const;
        void submitUpdatedObjects(std::vector<PhysicsObject> updatedObjects);

        void submitPhysicsCommand(std::vector<PhysicsCommand> physicsCommand);

    private:
        // helper functions
        //void setAllOldData(PhysicsObject& physicsObj, PhysxObject& iniObj, size_t index);
        
        void retrieveOldData(PhysicsObject& physics_Obj, const PhysxObject& init_Obj) const;

        template<typename Type>
        void retrievePosOri(PhysicsObject& physics_Obj, Type data) const;

        void setAllData(PhysicsObject& updatedPhysicsObj, PhysxObject& underlying_Obj, bool duplicate);
        
        void setShape(PhysicsObject& updated_Obj, PhysxObject& underlying_Obj, bool duplicate);

        void setForce(PhysxObject& underlying_Obj, PhysicsCommand& command_Obj);
        
        void setTorque(PhysxObject& underlying_Obj, PhysicsCommand& command_Obj);


    private:
        PxScene* scene = nullptr;

        //std::map<phy_uuid::UUID, PxMaterial*> mat;  // map of all the physics materials in the scene.

        std::map<phy_uuid::UUID, std::size_t> m_objects_lookup; // store all the index of the objects (lookups for keys / check if empty)

        std::vector<PhysxObject> m_physx_objects; // to iterate through for setting the data

        std::queue<TriggerManifold> m_triggerCollisionPairs; // queue to store the trigger collision pairs

        std::queue<ContactManifold> m_collisionPairs; // queue to store the collision pairs

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
}
