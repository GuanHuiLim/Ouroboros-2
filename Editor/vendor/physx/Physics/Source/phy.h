#pragma once

#include <Physics/Physx/include/PxPhysicsAPI.h>
#include <iostream>
#include <vector>
#include <map>

#define PVD_DEBUGGER true

using namespace physx;

class Collision;

class RigidDynamic;
class RigidStatic;
class PVD;
class PhysxObject;
class PhysicsObject;


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

struct RigidDynamic {

    PxRigidDynamic* rigidDynamic = nullptr;

    PxReal angularDamping;
    PxVec3 angularVelocity;
    PxReal invMass;
    PxReal linearDamping;
    PxVec3 linearVelocit;
    PxReal mass;
    PxTransform centerOfMass;
    PxTransform globalPos;
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

class RigidStatic {

private:

    int rigidID;

    PxRigidStatic* rigidStatic;

public:

    //createRigidStatic

    //All the getter
    PxRigidStatic* getRigidStatic() const;
    //All the setter
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

    friend class PhysicsObject;

    PxScene* scene;
    std::map<int, PxMaterial*> mat; // later int change to UUID
    PxVec3 gravity;

    std::vector<PhysxObject> m_objects;

public:

    // SCENE
    PhysxWorld(PxVec3 gravity);
    ~PhysxWorld();

    /*
    // SCENE
    int createScene(PxVec3 gravity);
    void releaseScene(int sceneID);
    */

    // MATERIAL
    int createMat(Material material);
    void updateMat(int materialID, Material material);
    void destroyMat(int materialID);

    // RIGIDBODY
    PhysicsObject createRigidbody();
    void removeRigidbody(PhysicsObject obj);


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

    int id = -1;

    int matID = -1;
    //Material mat;

    // ensure at least static or dynamic is init
    RigidStatic rs{};
    RigidDynamic rd{};

    enum class rigid { rstatic, rdynamic };
    rigid rigidID = rigid::rstatic;

    bool gravity = true;
    bool kinematic = false;

    //~PhysxObject();

    //PhysxObject(int id, 
    //            int matID, 
    //            RigidDynamic rd, 
    //            RigidStatic rs, 
    //            rigid rigidID, 
    //            bool gravity, 
    //            bool kinematic);

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

    int id;
    PhysxWorld* world;

    // functions...
    Material getMaterial() const;

    PxVec3 getposition() const;

    void setposition(PxVec3 pos);


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