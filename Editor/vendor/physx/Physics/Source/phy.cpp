#include <iostream>
#include "phy.h"

using namespace physx;

static PVD myPVD;

static constexpr bool use_debugger = false;

// actor might / not release
// scene (might release all the actor)
// mDispatcher
// mPhysics

// check if got pvd then release the pvd then transport
// mFoundation

PxDefaultAllocator      mDefaultAllocatorCallback;
PxDefaultErrorCallback  mDefaultErrorCallback;
PxDefaultCpuDispatcher* mDispatcher; // release

PxTolerancesScale       mToleranceScale;

PxFoundation* mFoundation; 
PxPhysics* mPhysics; // release

/*-----------------------------------------------------------------------------*/
/*                               Physx                                         */
/*-----------------------------------------------------------------------------*/
namespace physx_system {

    PxFoundation* getFoundation() {

        return mFoundation;
    }


    PxPhysics* getPhysics() {

        return mPhysics;
    }

    PxFoundation* createFoundation() {

        mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
        if (!mFoundation) throw("PxCreateFoundation failed!");

        return mFoundation;
    }

    PxPhysics* createPhysics() {

        mToleranceScale.length = 100;        // typical length of an object
        mToleranceScale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice

        //if (PVD_DEBUGGER)
        if constexpr (use_debugger)
            mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, myPVD.pvd__());
        else
            mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale);

        return mPhysics;
    }

    void init() {

        createFoundation();

        if constexpr (use_debugger) {
            printf("DEBUGGER ON\n");
            myPVD.createPvd(getFoundation(), "192.168.157.213");
        }
        else {
            printf("DEBUGGER OFF\n");
        }

        createPhysics();
    }

    void shutdown() {

        mDispatcher->release();

        getPhysics()->release();

        // pvd release here

        getFoundation()->release();

    }

}

/*-----------------------------------------------------------------------------*/
/*                               PhysxWorld                                    */
/*-----------------------------------------------------------------------------*/
PhysxWorld::PhysxWorld(PxVec3 grav)
{
    // check where leaking
    //m_objects.reserve(1000);

    // Setup scene description
    PxSceneDesc sceneDesc(physx_system::getPhysics()->getTolerancesScale());
    sceneDesc.gravity = grav; // PxVec3(0.0f, -9.81f, 0.0f);
    gravity = sceneDesc.gravity;

    mDispatcher = PxDefaultCpuDispatcherCreate(2);

    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    // report all the kin-kin contacts
    sceneDesc.kineKineFilteringMode = PxPairFilteringMode::eKEEP;
    // report all the static-kin contacts
    sceneDesc.staticKineFilteringMode = PxPairFilteringMode::eKEEP;

    //sceneDesc.simulationEventCallback = &event_callback;
    //sceneDesc.filterShader = contactReportFilterShader;

    scene = physx_system::getPhysics()->createScene(sceneDesc);

}

PhysxWorld::~PhysxWorld()
{
    // maybe here never shutdown prop
    scene->release();
}

void PhysxWorld::updateScene(float dt) {

    scene->simulate(dt); // 1.f / 60.f
    //scene->collide(dt);
    scene->fetchResults(true);
}

PxVec3 PhysxWorld::getGravity() const {

    return gravity;
}

void PhysxWorld::setGravity(PxVec3 gra) {

    scene->setGravity(gra);
    gravity = gra;
}


phy_uuid::UUID PhysxWorld::createMat(Material material) {

    PxMaterial* newMat = physx_system::getPhysics()->createMaterial(material.staticFriction,
        material.dynamicFriction,
        material.restitution);


    phy_uuid::UUID UUID = phy_uuid::UUID{}; // later change to UUID

    mat.emplace(UUID, newMat);

    return UUID;
}

void PhysxWorld::updateMat(phy_uuid::UUID materialID, Material material) {

    // SEARCH FOR THAT KEY UUID IN THE MAP (if have then set the new material in)

    for (auto const& x : mat) {

        if (x.first == materialID) {

            x.second->setStaticFriction(material.staticFriction);
            x.second->setDynamicFriction(material.dynamicFriction);
            x.second->setRestitution(material.restitution);
        }
    }
}

void PhysxWorld::destroyMat(phy_uuid::UUID materialID) {

    for (auto const& x : mat) {

        if (x.first == materialID) {
            x.second->release();
        }
    }
}

PhysicsObject PhysxWorld::createRigidbody()
{
    // create instance of the object (on the stack)
    //phy_uuid::UUID UUID = phy_uuid::UUID{};

    // here should create 1 of them 
    PhysxObject obj;   // assume assign UUID
    obj.id = 0;// UUID;
    obj.rd.rigidDynamic = physx_system::getPhysics()->createRigidDynamic(PxTransform(PxVec3(0)));
    scene->addActor(*obj.rd.rigidDynamic);

    // assign UUID 
    // store the object
    m_objects.emplace_back(obj); 
    //all_objects.insert({ obj.id, &obj });

    // return the object i created
    return PhysicsObject{ /*UUID*/ obj.id, this }; // a copy
}

void PhysxWorld::removeRigidbody(PhysicsObject obj)
{
    // check what need to release 
    
    // check/find the id from the obj vector then if match 
    // remove from that vector then release

    auto begin = std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& elem) { return elem.id == obj.id; });
    //begin->destroy();
    m_objects.erase(begin);
}

/*
void PhysxObject::destroy() {

    if (rs != nullptr) {
        rs->getRigidStatic()->release();
    }

    if (rd != nullptr) {
        rd->getRigidDynamic()->release();
    }
}
*/

//PhysxWorld world;
//
//PhysicsObject obj = world.createPhysicsObject();
//obj.getMaterial()
//
//world.removeObject(obj);


/*-----------------------------------------------------------------------------*/
/*                               PhysxObject                                   */
/*-----------------------------------------------------------------------------*/
void PhysxObject::setMass(PxReal mass) {

    if (rigidID == rigid::rdynamic)
        rd.rigidDynamic->setMass(mass);
}

void PhysxObject::enableKinematic(bool kine) {

    if (rigidID == rigid::rdynamic)
        rd.rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kine);
}

void PhysxObject::enableGravity(bool gravity) {

    rd.rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, gravity);
}

/*-----------------------------------------------------------------------------*/
/*                               PhysicsObject                                 */
/*-----------------------------------------------------------------------------*/

Material PhysicsObject::getMaterial() const {

    // prob need change

    Material mat{};

    for (auto const& x : world->mat) {
        if (x.first == world->m_objects[id].matID) {

            mat.staticFriction = x.second->getStaticFriction();
            mat.dynamicFriction = x.second->getDynamicFriction();
            mat.restitution = x.second->getRestitution();
        }
    }

    return mat; // world->m_objects[id].matID;
}

PxVec3 PhysicsObject::getposition() const {

    //for (auto const& x : world->all_objects) {
    //    if (x.first == id) {
    //
    //        return PxVec3{ x.second->rd.rigidDynamic->getGlobalPose().p.x,
    //                       x.second->rd.rigidDynamic->getGlobalPose().p.y,
    //                       x.second->rd.rigidDynamic->getGlobalPose().p.z };
    //    }
    //}

    // no have then return a default?

    return PxVec3{ world->m_objects[id].rd.rigidDynamic->getGlobalPose().p.x,
                   world->m_objects[id].rd.rigidDynamic->getGlobalPose().p.y,
                   world->m_objects[id].rd.rigidDynamic->getGlobalPose().p.z };
}

void PhysicsObject::setposition(PxVec3 pos) {

    //for (auto const& x : world->all_objects) {
    //    if (x.first == id) {
    //        x.second->rd.rigidDynamic->setGlobalPose(PxTransform(PxVec3(pos.x, pos.y, pos.z)));
    //    }
    //}

    world->m_objects[id].rd.rigidDynamic->setGlobalPose(PxTransform(PxVec3(pos.x, pos.y, pos.z)));
}

/*-----------------------------------------------------------------------------*/
/*                               RigidDynamic                                  */
/*-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------*/
/*                               RigidStatic                                   */
/*-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------*/
/*                               PVD                                           */
/*-----------------------------------------------------------------------------*/
PxPvd* PVD::createPvd(PxFoundation* foundation, const char* ip) {

    mPVD = PxCreatePvd(*foundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(ip, 5425, 10);
    mPVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

    return mPVD;

    //mPVD->getTransport()->release();
}

void PVD::setupPvd(PxScene* scene) {

    // Setup PVD
    PxPvdSceneClient* pvdClient = scene->getScenePvdClient();
    //PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();

    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
}

PxPvd*& PVD::pvd__() {

    return mPVD;
}

PxPvd* const& PVD::pvd__() const {

    return mPVD;
}