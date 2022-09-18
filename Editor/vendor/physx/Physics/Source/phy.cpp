#include <iostream>
#include "phy.h"

using namespace physx;

//static Physx myPhysx;
//static PhysxWorld myWorld;
static PVD myPVD;

static constexpr bool use_debugger = true;

PxDefaultAllocator      mDefaultAllocatorCallback;
PxDefaultErrorCallback  mDefaultErrorCallback;
PxDefaultCpuDispatcher* mDispatcher;

PxTolerancesScale       mToleranceScale;

PxFoundation* mFoundation;
PxPhysics* mPhysics;

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

        getFoundation()->release();

        getPhysics()->release();
    }





}

/*-----------------------------------------------------------------------------*/
/*                               PhysxWorld                                    */
/*-----------------------------------------------------------------------------*/
PhysxWorld::PhysxWorld(PxVec3 grav)
{
    // check where leaking
    
    // Setup scene description
    PxSceneDesc sceneDesc(physx_system::getPhysics()->getTolerancesScale());
    sceneDesc.gravity = grav; // PxVec3(0.0f, -9.81f, 0.0f);

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

int PhysxWorld::createMat(Material material) {

    PxMaterial* newMat = physx_system::getPhysics()->createMaterial(material.staticFriction,
        material.dynamicFriction,
        material.restitution);


    int UUID = 0; // later change to UUID

    mat.emplace(UUID, newMat);

    return UUID;

    /*
    PxMaterial* mat = myMaster.getPhysics()->createMaterial(material.getStaticFriction(),
        material.getDynamicFriction(),
        material.getRestitution());

    materialList.emplace_back(mat);

    return materialList.size() - 1; // return created material id
    */
}

void PhysxWorld::updateMat(int materialID, Material material) {

    // SEARCH FOR THAT KEY UUID IN THE MAP (if have then set the new material in)

    for (auto const& x : mat) {

        if (x.first == materialID) {

            x.second->setStaticFriction(material.staticFriction);
            x.second->setDynamicFriction(material.dynamicFriction);
            x.second->setRestitution(material.restitution);
        }
    }

    /*
    materialList[materialID]->setStaticFriction(material.getStaticFriction());
    materialList[materialID]->setDynamicFriction(material.getDynamicFriction());
    materialList[materialID]->setRestitution(material.getRestitution());
    */
}

void PhysxWorld::destroyMat(int materialID) {

    for (auto const& x : mat) {

        if (x.first == materialID) {
            x.second->release();
        }
    }
}

PhysicsObject PhysxWorld::createRigidbody()
{
    // create instance of the object (on the stack)
    int UUID = 0;

    // here should create 1 of them 
    PhysxObject obj;   // assume assign UUID
    obj.id = UUID;
    obj.rd.rigidDynamic = physx_system::getPhysics()->createRigidDynamic(PxTransform(PxVec3(0)));

    // assign UUID 
    m_objects.emplace_back(obj); // store the object

    // return the object i created
    return PhysicsObject{ UUID, this }; // a copy
}

void PhysxWorld::removeRigidbody(PhysicsObject obj)
{
    // check/find the id from the obj vector then if match 
    // remove from that vector then release

    auto begin = std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& elem) { return elem.id == obj.id; });
    //begin->destroy();
    m_objects.erase(begin);
    
    //m_objects.erase(begin, m_objects.end());


    //for(PhysxObject const& x : m_objects) {
    //    
    //    if (x.id == obj.id) {
    //        m_objects.erase(std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& elem) { return elem.id == obj.id; }))
    //    }
    //
    //}

    //PxRigidDynamic* body;
    //body->release();

    //obj.
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

//PhysxObject::~PhysxObject() {
//
//    if (rs != nullptr) {
//        rs->getRigidStatic()->release();
//    }
//
//    if (rd != nullptr) {
//        rd->getRigidDynamic()->release();
//    }
//
//    //switch (rigidID) {
//    //
//    //case rigid::rstatic:
//    //    rs->getRigidStatic()->release();
//    //    break;
//    //
//    //case rigid::rdynamic:
//    //    rd->getRigidDynamic()->release();
//    //    break;
//    //
//    //default:
//    //    break;
//    //}
//}

//PhysxWorld world;
//
//PhysicsObject obj = world.createPhysicsObject();
//obj.getMaterial()
//
//world.removeObject(obj);




/*-----------------------------------------------------------------------------*/
/*                               PhysxObject                                   */
/*-----------------------------------------------------------------------------*/
//PhysxObject::rigid PhysxObject::getRigidType() {
//
//    return rigidID;
//
//    //rigidID == rigid::rstatic || 
//    //return 0; // static
//    //return 1; // dynamic
//}

//PhysxObject::PhysxObject(int id, 
//                         int matID,
//                         RigidDynamic rd, 
//                         RigidStatic rs, 
//                         rigid rigidID,
//                         bool gravity, 
//                         bool kinematic) : id{ id }, 
//                                           matID{ matID },
//                                           rd{ rd }, 
//                                           rs{ rs }, 
//                                           rigidID { rigidID }, 
//                                           gravity { gravity }, 
 //                                          kinematic { kinematic } {}

void PhysxObject::enableKinematic(bool kine) {

    //PxRigidDynamic* body; // need to change

    if (rigidID == rigid::rdynamic) {
        rd.rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kine);
    }

}

void PhysxObject::enableGravity(bool gravity) {

    //PxRigidDynamic* body; // need to change

    rd.rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, gravity);
}

/*-----------------------------------------------------------------------------*/
/*                               PhysicsObject                                 */
/*-----------------------------------------------------------------------------*/

Material PhysicsObject::getMaterial() const {

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

    return PxVec3{ world->m_objects[id].rd.rigidDynamic->getGlobalPose().p.x,
                   world->m_objects[id].rd.rigidDynamic->getGlobalPose().p.y,
                   world->m_objects[id].rd.rigidDynamic->getGlobalPose().p.z };
}

void PhysicsObject::setposition(PxVec3 pos) {

    world->m_objects[id].rd.rigidDynamic->setGlobalPose(PxTransform(PxVec3(pos.x, pos.y, pos.z)));
}


/*-----------------------------------------------------------------------------*/
/*                               RigidDynamic                                  */
/*-----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------*/
/*                               RigidStatic                                   */
/*-----------------------------------------------------------------------------*/
PxRigidStatic* RigidStatic::getRigidStatic() const {

    return rigidStatic;
}


/*-----------------------------------------------------------------------------*/
/*                               PVD                                           */
/*-----------------------------------------------------------------------------*/
PxPvd* PVD::createPvd(PxFoundation* foundation, const char* ip) {

    mPVD = PxCreatePvd(*foundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(ip, 5425, 10);
    mPVD->connect(*transport, PxPvdInstrumentationFlag::eALL);

    return mPVD;
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




/*-----------------------------------------------------------------------------*/
/*                               MATERIAL                                      */
/*-----------------------------------------------------------------------------*/

/*
Material::Material(PxReal sf, PxReal df, PxReal r) : staticFriction{ sf },
                                                     dynamicFriction{ df },
                                                     restitution{ r } {}


PxReal const& Material::getStaticFriction() const {

    return staticFriction;
}

PxReal const& Material::getDynamicFriction() const {

    return dynamicFriction;
}

PxReal const& Material::getRestitution() const {

    return restitution;
}

void Material::setStaticFriction(PxMaterial* mat, PxReal sf) {

    mat->setStaticFriction(sf);
}

void Material::setDynamicFriction(PxMaterial* mat, PxReal df) {

    mat->setDynamicFriction(df);
}

void Material::setRestitution(PxMaterial* mat, PxReal r) {

    mat->setRestitution(r);
}
*/





