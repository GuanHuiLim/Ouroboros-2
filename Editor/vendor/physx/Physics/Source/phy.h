#pragma once

#include <Physics/Physx/include/PxPhysicsAPI.h>
#include <iostream>
#include <vector>
#include <map>
//#include <glm/glm.hpp>

#include "uuid.h"

#define PVD_DEBUGGER false

using namespace physx;

class Collision;

class PhysxWorld;
class PVD;
struct PhysxObject;
struct PhysicsObject;

enum class rigid { none, rstatic, rdynamic };
enum class shape { none, box, sphere, capsule, plane };

/*
class Collision {

private:

public:


};


class EventCallBack : public PxSimulationEventCallback {

private:

public:
    void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override {
        printf("CALLBACK: onConstraintBreak\n");
    }
    void onWake(PxActor** actors, PxU32 count) override {
        printf("CALLBACK: onWake\n");
    }
    void onSleep(PxActor** actors, PxU32 count) override {
        printf("CALLBACK: onSleep\n");
    }
    void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override {
        printf("CALLBACK: onContact\n");
    }
    void onTrigger(PxTriggerPair* pairs, PxU32 count) override {
        printf("CALLBACK: onTrigger\n");
    }
    void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override {
        printf("CALLBACK: onAdvance\n");
    }
};
*/

struct RigidDynamic {

    PxRigidDynamic* rigidDynamic = nullptr;

    //PxReal angularDamping;
    //PxVec3 angularVelocity;
    ////PxReal invMass;
    //PxReal linearDamping;
    //PxVec3 linearVelocity;
    //PxReal mass;
    //PxTransform centerOfMass;
    //PxTransform globalPos;
};

struct RigidStatic {

    PxRigidStatic* rigidStatic = nullptr;

    /// other variables if needed
};

// unprotected class
struct Material {

    PxReal staticFriction;
    PxReal dynamicFriction;
    PxReal restitution;
};

// backend holds the overall info of the entire physics engine
namespace physx_system {

    void init();

    void shutdown();

    PxFoundation* createFoundation();

    PxPhysics* createPhysics();

    PxFoundation* getFoundation();

    PxPhysics* getPhysics();
};

// describes a physics scene
class PhysxWorld {

private:

    friend struct PhysicsObject;

    PxScene* scene = nullptr;
    std::map<phy_uuid::UUID, PxMaterial*> mat; // later int change to UUID
    PxVec3 gravity;

    //std::map<phy_uuid::UUID, PhysxObject*> all_objects; // store all the objects (lookups for keys / check if empty)
    std::map<phy_uuid::UUID, int> all_objects; // store all the index of the objects (lookups for keys / check if empty)

    std::vector<PhysxObject> m_objects; // to iterate through for setting the data

public:

    // SCENE
    PhysxWorld(PxVec3 gravity);
    ~PhysxWorld();
    void updateScene(float dt);

    // GRAVITY
    PxVec3 getGravity() const;
    void setGravity(PxVec3 gra);

    // MATERIAL
    //void destroyMat(phy_uuid::UUID materialID);

    // RIGIDBODY
    PhysicsObject createInstance();
    void removeInstance(PhysicsObject obj);

    //CHECKING QUERY
};

// associated to each object in the physics world (me store)
struct PhysxObject {

    phy_uuid::UUID id = 0;
    phy_uuid::UUID matID = 0;

    // shape
    PxShape* m_shape = nullptr; // prob no need this
    shape shape = shape::none;

    // ensure at least static or dynamic is init
    RigidStatic rs{};
    RigidDynamic rd{};

    rigid rigidID = rigid::none;

    bool gravity = true;
    bool kinematic = false;
};

struct PhysicsObject { // you store

    phy_uuid::UUID id;
    PhysxWorld* world;

    // GETTERS
    Material getMaterial() const;
    PxVec3 getposition() const;
    PxQuat getOrientation() const;

    // SETTERS
    void setRigidType(rigid type);
    void setMaterial(Material material);
    //void setposition(PxVec3 pos);
    //void setOrientation(PxQuat quat);
    void setPosOrientation(PxVec3 pos, PxQuat quat);

    void setGravity(bool gravity);
    void setKinematic(bool kine);

    // set default value for each type of shape & can change shape too
    void setShape(shape shape);

    // change each individual property based on its shape
    void setBoxProperty(float halfextent_width, float halfextent_height, float halfextent_depth);
    void setSphereProperty(float radius);
    //void setPlaneProperty(float radius);
    void setCapsuleProperty(float radius, float halfHeight);

    // prob functions that dont really need
    void setMass(PxReal mass);
    void setAngularDamping(PxReal angularDamping);
    void setAngularVelocity(PxVec3 angularVelocity);
    void setLinearDamping(PxReal linearDamping);
    void setLinearVelocity(PxVec3 linearVelocity);
};


// Physx visual degguer
class PVD {

private:

    PxPvd* mPVD;

public:

    PxPvd* createPvd(PxFoundation* foundation, const char* ip);

    void setupPvd(PxScene* scene);

    PxPvd*& pvd__();

    PxPvd* const& pvd__() const;
};


//mATERIAL MAT;
//PHYSICSWORLD.UPDATESHAPE(OBJID, MAT);
//MAT = PHYSICSWORLD.GETMATERIAL(OBJID);
//MAT.SET
//PHYSICSWORLD.SETMAT(OBJID, MAT);