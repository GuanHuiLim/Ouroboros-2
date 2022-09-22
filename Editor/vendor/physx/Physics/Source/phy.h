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

//class RigidDynamic;
//class RigidStatic;
class PhysxWorld;
class PVD;
struct PhysxObject;
struct PhysicsObject;

enum class rigid { rstatic, rdynamic };
enum class shape { none, box, sphere, capsule, plane };

/*
enum class shape_type { shape_box, shape_sphere, shape_capsule, shape_plane };

class Shape {

private:

    int shapeID;

public:


};

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

/*
class RigidDynamic {

private:

    //int dynamicID;

    PxRigidDynamic* rigidDynamic = nullptr;

    PxReal angularDamping;
    PxVec3 angularVelocity;
    PxReal invMass;
    PxReal linearDamping;
    PxVec3 linearVelocit;
    PxReal mass;
    PxTransform centerOfMass;
    PxTransform globalPos;

public:

    int createRigidDynamic();

    //int createRigidDynamic(PxTransform transform);

    // GETTER FUNCTIONS
    PxRigidDynamic* getRigidDynamic() const;

    PxReal getAngularDamping() const;
    
    //PxVec3 getAngularVelocity(PxRigidDynamic* body);
    //PxReal getInvMass(PxRigidDynamic* body);
    //PxReal getLinearDamping(PxRigidDynamic* body);
    //PxVec3 getLinearVelocity(PxRigidDynamic* body);
    //PxReal getMass(PxRigidDynamic* body);
    //PxTransform getCenterOfMass(PxRigidDynamic* body);

    PxTransform getGlobalPos() const;

    // SETTER FUNCTIONS
    void setAngularDamping(PxReal angularDamping);
    //void setAngularVelocity(PxRigidDynamic* body, PxVec3 angularVelocity);
    //void setInvMass(PxRigidDynamic* body, PxReal invMass);
    //void setLinearDamping(PxRigidDynamic* body, PxReal linearDamping);
    //void setLinearVelocity(PxRigidDynamic* body, PxVec3 linearVelocity);
    //void setMass(PxRigidDynamic* body, PxReal setMass);
    //void setCenterOfMass(PxRigidDynamic* body, PxTransform centerOfMass);
    void setGlobalPos(PxRigidDynamic* body, PxTransform globalPos);
};
*/

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

    PxScene* scene;
    std::map<phy_uuid::UUID, PxMaterial*> mat; // later int change to UUID
    PxVec3 gravity;

    std::map<phy_uuid::UUID, PhysxObject*> all_objects; // store all the objects (lookups for keys / check if empty)

    std::vector<PhysxObject> m_objects; // to iterate through for setting the data

public:

    // SCENE
    PhysxWorld(PxVec3 gravity);
    ~PhysxWorld();
    void updateScene(float dt);

    // GRAVITY
    PxVec3 getGravity() const;
    void setGravity(PxVec3 gra);

    /*
    // SCENE
    int createScene(PxVec3 gravity);
    void releaseScene(int sceneID);
    */

    // MATERIAL
    phy_uuid::UUID createMat(PhysicsObject obj, Material material);
    //void updateMat(phy_uuid::UUID materialID, Material material);
    void destroyMat(phy_uuid::UUID materialID);

    // RIGIDBODY
    //PhysicsObject createRigidbody(rigid type);
    PhysicsObject createInstance();
    void removeRigidbody(PhysicsObject obj);

    // SHAPE
    void createShape(PhysicsObject obj, shape shape);

    /*
    shape_type createShape(shape_type type, Material material);

     GEOMETRY
    int createPlane(rigid type, PxPlane plane);
    int createPlane(rigid type, PxPlane plane, Material material);

    int createBox
    int createSphere

     CHECKING QUERY
    check
    */

};

// associated to each object in the physics world
struct PhysxObject {

    phy_uuid::UUID id = 0;
    phy_uuid::UUID matID = 0;

    // shape
    PxShape* m_shape; // prob no need this
    shape shape = shape::none;

    // ensure at least static or dynamic is init
    RigidStatic rs{};
    RigidDynamic rd{};

    rigid rigidID = rigid::rstatic; // do i need to set a default? (change it depends how it is created)

    bool gravity = true;
    bool kinematic = false;


    // SETTERS
    void enableGravity(bool gravity);
    void enableKinematic(bool kine);
};

//class PhysxObject { // me store
//
//private:
//    // state full
    // only be static or dynamic
    // do quite a few things
    // all sort of properties
//
//    int id;
//
//    Material mat;
//
//    RigidDynamic rd;
//    RigidStatic rs;
//
//    enum class rigid { rstatic, rdynamic };
//    rigid rigidID;
//
//    bool gravity;
//    bool kinematic;
//
//    //Collider collider;
//
//public:
//    /*
//    PhysxObject(int id);
//
//    Material getMaterial();
//
//    rigid getRigidType();
//
//    void enableGravity(bool gravity);
//
//    void enableKinematic(bool kine);
//    */
//};

struct PhysicsObject { // you store

    phy_uuid::UUID id;
    PhysxWorld* world;

    // functions...
    Material getMaterial() const;
    PxVec3 getposition() const;
    PxQuat getOrientation() const;
    // set orientation

    //getRotation() const;

    void setRigidType(rigid type);
    void setMaterial(Material material);
    void setposition(PxVec3 pos);

    // set default value for each type of shape & can change shape too
    void setShape(shape shape);

    // change each individual property based on its shape

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

/*
class Material {

public:

    PxReal staticFriction;
    PxReal dynamicFriction;
    PxReal restitution;

private:

    Material(PxReal sf = 0, PxReal df = 0, PxReal r = 0);

    PxReal const& getStaticFriction() const;
    PxReal const& getDynamicFriction() const;
    PxReal const& getRestitution() const;

    void setStaticFriction(PxMaterial* mat, PxReal sf);
    void setDynamicFriction(PxMaterial* mat, PxReal df);
    void setRestitution(PxMaterial* mat, PxReal r);
};
*/