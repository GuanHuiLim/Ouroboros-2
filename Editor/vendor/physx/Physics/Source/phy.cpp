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

        if(mDispatcher)
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


phy_uuid::UUID PhysxWorld::createMat(PhysicsObject obj, Material material) {

    PxMaterial* newMat = physx_system::getPhysics()->createMaterial(material.staticFriction,
                                                                    material.dynamicFriction,
                                                                    material.restitution);

    phy_uuid::UUID UUID = phy_uuid::UUID{};

    mat.emplace(UUID, newMat);

    all_objects.at(obj.id)->matID = UUID; // set material for that object

    return UUID;
}

/*
void PhysxWorld::updateMat(phy_uuid::UUID materialID, Material material) {

    // SEARCH FOR THAT KEY UUID IN THE MAP (if have then set the new material in)

    if (mat.contains(materialID)) {
        mat.at(materialID)->setStaticFriction(material.staticFriction);
        mat.at(materialID)->setDynamicFriction(material.dynamicFriction);
        mat.at(materialID)->setRestitution(material.restitution);
    }
}
*/

// check again need call this at where
void PhysxWorld::destroyMat(phy_uuid::UUID materialID) {

    if (mat.contains(materialID)) {
        mat.at(materialID)->release();
    }
}

/*
PhysicsObject PhysxWorld::createRigidbody(rigid type)
{
    // create instance of the object (on the stack)
    //phy_uuid::UUID UUID = phy_uuid::UUID{};

    // here should create 1 of them 
    PhysxObject obj;   
    obj.id = phy_uuid::UUID{};

    if (type == rigid::rstatic) {
        obj.rs.rigidStatic = physx_system::getPhysics()->createRigidStatic(PxTransform(PxVec3(0)));
        scene->addActor(*obj.rs.rigidStatic);
    }
    else if (type == rigid::rdynamic) {
        obj.rd.rigidDynamic = physx_system::getPhysics()->createRigidDynamic(PxTransform(PxVec3(0))); // PxCreateDynamic()
        scene->addActor(*obj.rd.rigidDynamic);
    }

    //scene->removeActor()

    obj.rigidID = type;

    // store the object
    m_objects.emplace_back(obj); 
    all_objects.insert({ obj.id, &m_objects.at(m_objects.size() - 1) }); // add back the m_objects last element
    
    // return the object i created
    return PhysicsObject{ obj.id, this }; // a copy
}
*/

void PhysxWorld::removeRigidbody(PhysicsObject obj)
{
    // check what need to release 
    
    // check/find the id from the obj vector then if match 
    // remove from that vector then release

    auto begin = std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& elem) { return elem.id == obj.id; });
    //begin->destroy();
    m_objects.erase(begin);
}

void PhysxWorld::createShape(PhysicsObject obj, shape shape) {

    //PxRigidActorExt::createExclusiveShape (another method)
    
    // search for that object
    if (all_objects.contains(obj.id)) {

        PhysxObject* underlying_obj = all_objects.at(obj.id);
        PxMaterial* material = mat.at(underlying_obj->matID);

        //switch (shape) {
        //}

        if (shape == shape::box) {
            PxBoxGeometry temp_box{ 0.5f,0.5f,0.5f };
            underlying_obj->m_shape = physx_system::getPhysics()->createShape(temp_box, *material);
            //m_objects.at(obj.id).m_shape = physx_system::getPhysics()->createShape(PxBoxGeometry{}, *mat.at(all_objects.at(obj.id)->matID));
        }
        else if (shape == shape::sphere) {
            underlying_obj->m_shape = physx_system::getPhysics()->createShape(PxSphereGeometry(),*material);
        }
        else if (shape == shape::plane) {
            underlying_obj->m_shape = physx_system::getPhysics()->createShape(PxPlaneGeometry(),*material);
        }
        else if (shape == shape::capsule) {
            underlying_obj->m_shape = physx_system::getPhysics()->createShape(PxCapsuleGeometry(),*material);
        }

        //m_objects.at(obj.id).m_shape->getGeometry().sphere().radius = 5.f;
        
        underlying_obj->shape = shape;

        if (underlying_obj->rigidID == rigid::rstatic)
            underlying_obj->rs.rigidStatic->attachShape(*underlying_obj->m_shape);

        else if (underlying_obj->rigidID == rigid::rdynamic)
            underlying_obj->rd.rigidDynamic->attachShape(*underlying_obj->m_shape);


        // later check where need to release shape
        underlying_obj->m_shape->release();
    }

}

//PhysxWorld world;
//
//PhysicsObject obj = world.createPhysicsObject();
//obj.getMaterial()
//
//world.removeObject(obj);

/*-----------------------------------------------------------------------------*/
/*                               PhysxObject                                   */
/*-----------------------------------------------------------------------------*/

void PhysxObject::enableKinematic(bool kine) {

    if (rigidID == rigid::rdynamic)
        rd.rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kine);
}

void PhysxObject::enableGravity(bool grav) {

    rd.rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, grav);
}

/*-----------------------------------------------------------------------------*/
/*                               PhysicsObject                                 */
/*-----------------------------------------------------------------------------*/
void PhysicsObject::setRigidType(rigid type) {

    PxTransform temp_trans{ PxVec3(0) }; // set default to 0

    // CHECK IF HAVE RIGIDBODY CREATED OR NOT
    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);
        PxRigidStatic* rstat = underlying_obj->rs.rigidStatic;
        PxRigidDynamic* rdyna = underlying_obj->rd.rigidDynamic;

        underlying_obj->rigidID = type;

        // CHECK IF THIS OBJ HAVE RSTATIC OR RDYAMIC INIT OR NOT
        if (rstat) {
            temp_trans = rstat->getGlobalPose();
            world->scene->removeActor(*rstat);
        }
        else if (rdyna) {
            temp_trans = rdyna->getGlobalPose();
            world->scene->removeActor(*rdyna);
        }
        // CREATE RSTATIC OR RDYNAMIC ACCORDINGLY
        if (type == rigid::rstatic) {
            rstat = physx_system::getPhysics()->createRigidStatic(temp_trans);
            world->scene->addActor(*rstat);
        }
        else if (type == rigid::rdynamic) {
            rdyna = physx_system::getPhysics()->createRigidDynamic(temp_trans);
            world->scene->addActor(*rdyna);
        }
    }
    else {

        // CREATE NEW RIGIDBODY 
        PhysxObject obj;
        obj.id = phy_uuid::UUID{};
        obj.rigidID = type;

        // CREATE RSTATIC OR RDYNAMIC ACCORDINGLY
        if (type == rigid::rstatic) {
            obj.rs.rigidStatic = physx_system::getPhysics()->createRigidStatic(temp_trans);
            world->scene->addActor(*obj.rs.rigidStatic);
        }
        else if (type == rigid::rdynamic) {
            obj.rd.rigidDynamic = physx_system::getPhysics()->createRigidDynamic(temp_trans); // PxCreateDynamic()
            world->scene->addActor(*obj.rd.rigidDynamic);
        }

        // ADD INTO MAP & VECTOR
        world->m_objects.emplace_back(obj);
        world->all_objects.insert({ obj.id, &world->m_objects.at(world->m_objects.size() - 1) }); // add back the m_objects last element
    }
}

Material PhysicsObject::getMaterial() const {

    Material m_material{};

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (world->mat.contains(underlying_obj->matID)) {
            m_material.staticFriction = world->mat[underlying_obj->matID]->getStaticFriction();
            m_material.dynamicFriction = world->mat[underlying_obj->matID]->getDynamicFriction();
            m_material.restitution = world->mat[underlying_obj->matID]->getRestitution();
        }   
    }

    return m_material;
}

void PhysicsObject::setMaterial(Material material) {

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);
        PxMaterial* temp_mat = world->mat.at(underlying_obj->matID);

        if (world->mat.contains(underlying_obj->matID)) {
            temp_mat->setStaticFriction(material.staticFriction);
            temp_mat->setDynamicFriction(material.dynamicFriction);
            temp_mat->setRestitution(material.restitution);
        }
    }
}

PxVec3 PhysicsObject::getposition() const {

    // contains the key the return the value of that key
    //all_objects[key]

    // no have then return a default?

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (underlying_obj->rigidID == rigid::rstatic) {

            return PxVec3{ underlying_obj->rs.rigidStatic->getGlobalPose().p.x,
                           underlying_obj->rs.rigidStatic->getGlobalPose().p.y,
                           underlying_obj->rs.rigidStatic->getGlobalPose().p.z };
        }
        else if (underlying_obj->rigidID == rigid::rdynamic) {

            return PxVec3{ underlying_obj->rd.rigidDynamic->getGlobalPose().p.x,
                           underlying_obj->rd.rigidDynamic->getGlobalPose().p.y,
                           underlying_obj->rd.rigidDynamic->getGlobalPose().p.z };
        }

    }

    return PxVec3{};
}

void PhysicsObject::setposition(PxVec3 pos) {

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (underlying_obj->rigidID == rigid::rstatic)
            underlying_obj->rs.rigidStatic->setGlobalPose(PxTransform(PxVec3(pos.x, pos.y, pos.z)));
       
        else if (underlying_obj->rigidID == rigid::rdynamic)
            underlying_obj->rd.rigidDynamic->setGlobalPose(PxTransform(PxVec3(pos.x, pos.y, pos.z)));
    }

}

PxQuat PhysicsObject::getOrientation() const {

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (underlying_obj->rigidID == rigid::rstatic)
            return underlying_obj->rs.rigidStatic->getGlobalPose().q;

        else if (underlying_obj->rigidID == rigid::rdynamic)
            return underlying_obj->rd.rigidDynamic->getGlobalPose().q;
    }
}

void PhysicsObject::setShape(shape shape) {

    //here
}



void PhysicsObject::setMass(PxReal mass) {

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (underlying_obj->rigidID == rigid::rdynamic)
            underlying_obj->rd.rigidDynamic->setMass(mass);
    }
}

void PhysicsObject::setAngularDamping(PxReal angularDamping) {

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (underlying_obj->rigidID == rigid::rdynamic)
            underlying_obj->rd.rigidDynamic->setAngularDamping(angularDamping);
    }
}

void PhysicsObject::setAngularVelocity(PxVec3 angularVelocity) {

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (underlying_obj->rigidID == rigid::rdynamic)
            underlying_obj->rd.rigidDynamic->setAngularVelocity(angularVelocity);
    }
}

void PhysicsObject::setLinearDamping(PxReal linearDamping) {

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (underlying_obj->rigidID == rigid::rdynamic)
            underlying_obj->rd.rigidDynamic->setLinearDamping(linearDamping);
    }
}

void PhysicsObject::setLinearVelocity(PxVec3 linearVelocity) {

    if (world->all_objects.contains(id)) {

        PhysxObject* underlying_obj = world->all_objects.at(id);

        if (underlying_obj->rigidID == rigid::rdynamic)
            underlying_obj->rd.rigidDynamic->setLinearVelocity(linearVelocity);
    }
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