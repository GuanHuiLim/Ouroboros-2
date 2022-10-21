/************************************************************************************//*!
\file           phy.cpp
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

#include <iostream>
#include "phy.h"

using namespace physx;

static constexpr bool use_debugger = false;

static myPhysx::EventCallBack mEventCallback;

myPhysx::PVD myPVD;


/* release sequence
actor might / not release
scene (might release all the actor)
mDispatcher
mPhysics
check if got pvd then release the pvd then transport
mFoundation
*/

PxDefaultAllocator      mDefaultAllocatorCallback;
PxDefaultErrorCallback  mDefaultErrorCallback;
PxDefaultCpuDispatcher* mDispatcher;

PxTolerancesScale       mToleranceScale;

PxFoundation* mFoundation; 
PxPhysics* mPhysics;

/*-----------------------------------------------------------------------------*/
/*                               Physx                                         */
/*-----------------------------------------------------------------------------*/
namespace myPhysx
{
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

        // detects a trigger using the shape's simulation filter data
        bool isTrigger(const PxFilterData& data) {

            if (data.word0 != 0xffffffff || data.word1 != 0xffffffff || 
                data.word2 != 0xffffffff || data.word3 != 0xffffffff)
                return false;

            return true;
        }

        bool isTriggerShape(PxShape* shape) {

            // detects native built-in triggers.
            if (shape->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
                return true;

            // detects our emulated triggers using the simulation filter data
            if (physx_system::isTrigger(shape->getSimulationFilterData()))
                return true;

            // detects our emulated triggers using the simulation filter callback
            if (shape->userData)
                return true;

            return false;
        }

        void provideCurrentWorld(PhysxWorld* world) {

            // retrieving the current world for me to access (get/set) neccessary data
            currentWorld = world;
        }

        void init() {

            createFoundation();

            if constexpr (use_debugger) {
                printf("DEBUGGER ON\n");
                myPVD.createPvd(getFoundation(), "192.168.2.32");
            }
            else {
                printf("DEBUGGER OFF\n");
            }

            createPhysics();
        }

        void shutdown() {

            getPhysics()->release();

            // pvd release here
            if constexpr (use_debugger) {

                myPVD.pvd__()->release();

                myPVD.getTransport()->release();
            }

            getFoundation()->release();
        }

    }

/*-----------------------------------------------------------------------------*/
/*                               PhysxWorld                                    */
/*-----------------------------------------------------------------------------*/
    PhysxWorld::PhysxWorld(PxVec3 grav)
    {
        // Setup scene description
        PxSceneDesc sceneDesc(physx_system::getPhysics()->getTolerancesScale());
        sceneDesc.gravity = grav; // PxVec3(0.0f, -9.81f, 0.0f);
        gravity = sceneDesc.gravity;

        //PxInitExtensions(*physx_system::getPhysics(), myPVD.pvd__());
        mDispatcher = PxDefaultCpuDispatcherCreate(2);

        sceneDesc.cpuDispatcher = mDispatcher;
        sceneDesc.filterShader = PxDefaultSimulationFilterShader;

        // report all the kin-kin contacts
        sceneDesc.kineKineFilteringMode = PxPairFilteringMode::eKEEP;
        // report all the static-kin contacts
        sceneDesc.staticKineFilteringMode = PxPairFilteringMode::eKEEP;

        sceneDesc.simulationEventCallback = &mEventCallback;
        //sceneDesc.filterShader = contactReportFilterShader;

        scene = physx_system::getPhysics()->createScene(sceneDesc);
    }

    PhysxWorld::~PhysxWorld()
    {
        // maybe here never shutdown prop

        // release all the materials
        for (auto const& i : mat) {
            i.second->release();
        }

        // release rigidstatic or rigiddynamic
        /*
        for(auto const& j : all_objects) {

            PhysxObject* underlying_obj = &m_objects[j.second];

            underlying_obj->m_shape->release();

            if (underlying_obj->rigidID == rigid::rstatic)
                underlying_obj->rb.rigidStatic->release();

            else if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->release();
        }
        */

        scene->release();

        if (mDispatcher)
        {
            mDispatcher->release();
            mDispatcher = nullptr;
        }

        //PxCloseExtensions();
    }

    void PhysxWorld::updateScene(float dt) {

        scene->simulate(dt); // 1.f / 60.f
        //scene->collide(dt);
        scene->fetchResults(true);
    }

    PxVec3 PhysxWorld::getWorldGravity() const {

        return gravity;
    }

    void PhysxWorld::setWorldGravity(PxVec3 gra) {

        scene->setGravity(gra);
        gravity = gra;
    }

    PhysicsObject PhysxWorld::createInstance() {

        // create instance of the object (on the stack)
        //phy_uuid::UUID UUID = phy_uuid::UUID{};

        PhysxObject obj;
        obj.id = std::make_unique<phy_uuid::UUID>();
        // This is important!
        phy_uuid::UUID generated_uuid = *obj.id;
        // store the object
        m_objects.emplace_back(std::move(obj));
        all_objects.insert({ generated_uuid, m_objects.size() - 1 }); // add back the m_objects last element
        
        // return the object i created
        return PhysicsObject{ generated_uuid, this }; // a copy
    }

    void PhysxWorld::removeInstance(PhysicsObject obj)
    {
        // check what need to release 

        // release rigidstatic or rigiddynamic
        if (all_objects.contains(obj.id)) {

            PhysxObject* underlying_obj = &m_objects[all_objects.at(obj.id)];

            if (underlying_obj->rigidID == rigid::rstatic)
                underlying_obj->rb.rigidStatic->release();

            else if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->release();

            // release shape
            //underlying_obj->m_shape->release();
        }

        // check/find the id from the obj vector then if match 
        // remove from that vector then release
        // NOTE: DON't MIXUP ELEM ID and OBJ ID HERE, 2 different types.
        auto begin = std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& elem) { return *elem.id == obj.id; });
        
        //begin->destroy();
        
        m_objects.erase(begin);
    }

    std::queue<TriggerManifold>* PhysxWorld::getTriggerData() {

        return &m_collisionPairs;
    }

    void PhysxWorld::clearTriggerData() {

        while (!m_collisionPairs.empty())
            m_collisionPairs.pop();

        //std::queue<TriggerManifold> empty;
        //std::swap(m_collisionPairs, empty);
    }


/*-----------------------------------------------------------------------------*/
/*                               PhysicsObject                                 */
/*-----------------------------------------------------------------------------*/
    void PhysicsObject::setRigidType(rigid type) {

        PxTransform temp_trans{ PxVec3(0) }; // set default to 0

        // CHECK GOT THE INSTANCE CREATED OR NOT
        if (world->all_objects.contains(id)) {

            //PhysxObject* underlying_obj = world->all_objects.at(id);
            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            PxRigidStatic* rstat = underlying_obj->rb.rigidStatic;
            PxRigidDynamic* rdyna = underlying_obj->rb.rigidDynamic;

            underlying_obj->rigidID = type;

            // CHECK IF HAVE RIGIDBODY CREATED OR NOT
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
                underlying_obj->rb.rigidStatic = physx_system::getPhysics()->createRigidStatic(temp_trans);
                underlying_obj->rb.rigidStatic->userData = underlying_obj->id.get();
                printf("actl value %llu vs pointer value: %llu \n", id, *reinterpret_cast<phy_uuid::UUID*>(underlying_obj->rb.rigidStatic->userData));
                world->scene->addActor(*underlying_obj->rb.rigidStatic);
            }
            else if (type == rigid::rdynamic) {
                underlying_obj->rb.rigidDynamic = physx_system::getPhysics()->createRigidDynamic(temp_trans);
                underlying_obj->rb.rigidDynamic->userData = underlying_obj->id.get();
                printf("actl value %llu vs pointer value: %llu \n", id, *reinterpret_cast<phy_uuid::UUID*>(underlying_obj->rb.rigidDynamic->userData));
                world->scene->addActor(*underlying_obj->rb.rigidDynamic);
            }

            // Check how many actors created in the scene
            //PxActorTypeFlags desiredTypes = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
            //PxU32 count = world->scene->getNbActors(desiredTypes);
            //PxActor** buffer = new PxActor * [count];
            //
            //PxU32 noo = world->scene->getActors(desiredTypes, buffer, count);
            //printf("%d - actors\n\n", noo);
        }
    }

    void PhysicsObject::setShape(shape shape) {

        //PxRigidActorExt::createExclusiveShape (another method)

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            PxMaterial* material = world->mat.at(underlying_obj->matID); // might need check if this set or not

            // CHECK IF HAVE SHAPE CREATED OR NOT
            if (underlying_obj->shape != shape::none) {

                // DETACH OLD SHAPE
                if (underlying_obj->rigidID == rigid::rstatic)
                    underlying_obj->rb.rigidStatic->detachShape(*underlying_obj->m_shape);

                else if (underlying_obj->rigidID == rigid::rdynamic)
                    underlying_obj->rb.rigidDynamic->detachShape(*underlying_obj->m_shape);
            }

            underlying_obj->shape = shape; // set new shape enum

            // CHECK AGAINST THE TYPE OF SHAPE
            if (shape == shape::box) {
                PxBoxGeometry temp_box{ 0.5f,0.5f,0.5f };
                underlying_obj->m_shape = physx_system::getPhysics()->createShape(temp_box, *material, true);
            }
            else if (shape == shape::sphere) {
                PxSphereGeometry temp_sphere{ 0.5f };
                underlying_obj->m_shape = physx_system::getPhysics()->createShape(temp_sphere, *material, true);
            }
            else if (shape == shape::plane) {
                //PxCreatePlane()
                //PxPlaneGeometry temp_sphere{ PxPlane{0.f,1.f,0.f,50.f} };
                //PxTransformFromPlaneEquation(PxPlane{ 0.f,1.f,0.f,50.f });
                underlying_obj->m_shape = physx_system::getPhysics()->createShape(PxPlaneGeometry(), *material, true);
            }
            else if (shape == shape::capsule) {
                PxCapsuleGeometry temp_cap{ 0.5f, 1.f };
                underlying_obj->m_shape = physx_system::getPhysics()->createShape(temp_cap, *material, true);
            }
            
            // ATTACH THE SHAPE TO THE OBJECT
            if (underlying_obj->rigidID == rigid::rstatic)
                underlying_obj->rb.rigidStatic->attachShape(*underlying_obj->m_shape);

            else if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->attachShape(*underlying_obj->m_shape);

            // later check where need to release shape
            //underlying_obj->m_shape->release();
        }
    }

    void PhysicsObject::removeShape() {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->shape != shape::none) {

                if (underlying_obj->rigidID == rigid::rstatic)
                    underlying_obj->rb.rigidStatic->detachShape(*underlying_obj->m_shape);

                else if (underlying_obj->rigidID == rigid::rdynamic)
                    underlying_obj->rb.rigidDynamic->detachShape(*underlying_obj->m_shape);

                underlying_obj->shape = shape::none; // set new shape enum
            }

            // need check whether need release shape need or not
        }
    }

    void PhysicsObject::enableKinematic(bool kine) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic) {

                underlying_obj->rb.rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kine);

                underlying_obj->kinematic = kine;
            }
        }
    }

    void PhysicsObject::disableGravity(bool grav) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            underlying_obj->rb.rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, grav);

            underlying_obj->gravity = grav;
        }
    }

    void PhysicsObject::setTriggerShape(bool trigger) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->m_shape) {

                underlying_obj->trigger = trigger;

                if (trigger)
                    underlying_obj->m_shape->setFlags(PxShapeFlag::eVISUALIZATION | PxShapeFlag::eTRIGGER_SHAPE);
                else
                    underlying_obj->m_shape->setFlags(PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSIMULATION_SHAPE);
            }
        }
    }

    void PhysicsObject::setBoxProperty(float halfextent_width, float halfextent_height, float halfextent_depth) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->shape == shape::box)
                underlying_obj->m_shape->setGeometry(PxBoxGeometry(halfextent_width, halfextent_height, halfextent_depth));
            //underlying_obj->m_shape->getGeometry().box().halfExtents = PxVec3{ halfextent_width , halfextent_height, halfextent_depth };
        }
    }

    void PhysicsObject::setSphereProperty(float radius) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->shape == shape::sphere)
                underlying_obj->m_shape->setGeometry(PxSphereGeometry(radius));
            //underlying_obj->m_shape->getGeometry().sphere().radius = radius;
        }
    }

    void PhysicsObject::setCapsuleProperty(float radius, float halfHeight) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            if (underlying_obj->shape == shape::capsule) {
                underlying_obj->m_shape->setGeometry(PxCapsuleGeometry(radius, halfHeight));
                //underlying_obj->m_shape->getGeometry().capsule().radius = radius;
                //underlying_obj->m_shape->getGeometry().capsule().halfHeight = halfHeight;
            }
        }
    }

    Material PhysicsObject::getMaterial() const {

        Material m_material{};

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

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

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            // CHECK WHETHER THE OBJ HAVE MATERIAL 
            if (world->mat.contains(underlying_obj->matID)) {

                PxMaterial* temp_mat = world->mat.at(underlying_obj->matID);

                temp_mat->setStaticFriction(material.staticFriction);
                temp_mat->setDynamicFriction(material.dynamicFriction);
                temp_mat->setRestitution(material.restitution);
            }
            else {

                // CREATE NEW MATERIAL
                PxMaterial* newMat = physx_system::getPhysics()->createMaterial(material.staticFriction,
                    material.dynamicFriction,
                    material.restitution);

                phy_uuid::UUID UUID = phy_uuid::UUID{};

                world->mat.emplace(UUID, newMat);

                underlying_obj->matID = UUID; // set material id for that object
            }
        }
    }

    PxVec3 PhysicsObject::getposition() const {

        // contains the key the return the value of that key
        //all_objects[key]

        // no have then return a default?

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rstatic) {

                return PxVec3{ underlying_obj->rb.rigidStatic->getGlobalPose().p.x,
                               underlying_obj->rb.rigidStatic->getGlobalPose().p.y,
                               underlying_obj->rb.rigidStatic->getGlobalPose().p.z };
            }
            else if (underlying_obj->rigidID == rigid::rdynamic) {

                return PxVec3{ underlying_obj->rb.rigidDynamic->getGlobalPose().p.x,
                               underlying_obj->rb.rigidDynamic->getGlobalPose().p.y,
                               underlying_obj->rb.rigidDynamic->getGlobalPose().p.z };
            }

        }

        // default return.
        return PxVec3{};
    }

    void PhysicsObject::setPosOrientation(PxVec3 pos, PxQuat quat) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rstatic)
                underlying_obj->rb.rigidStatic->setGlobalPose(PxTransform{ PxVec3(pos.x, pos.y, pos.z), quat });

            else if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setGlobalPose(PxTransform{ PxVec3(pos.x, pos.y, pos.z), quat });
        }
    }

    PxQuat PhysicsObject::getOrientation() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rstatic)
                return underlying_obj->rb.rigidStatic->getGlobalPose().q;

            else if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getGlobalPose().q;
        }

        // default return.
        return PxQuat{};
    }

    PxReal PhysicsObject::getMass() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getMass();
        }

        // default return.
        return PxReal{};
    }

    PxReal PhysicsObject::getInvMass() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getInvMass();
        }

        // default return.
        return PxReal{};
    }

    PxReal PhysicsObject::getAngularDamping() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getAngularDamping();
        }

        // default return.
        return PxReal{};
    }

    PxVec3 PhysicsObject::getAngularVelocity() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getAngularVelocity();
        }

        // default return.
        return PxVec3{};
    }

    PxReal PhysicsObject::getLinearDamping() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getLinearDamping();
        }

        // default return.
        return PxReal{};
    }

    PxVec3 PhysicsObject::getLinearVelocity() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getLinearVelocity();
        }

        // default return.
        return PxVec3{};
    }

    bool PhysicsObject::getTrigger() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            return underlying_obj->trigger;
        }

        // default return.
        return false;
    }

    bool PhysicsObject::getGravity() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->gravity;
        }

        // default return.
        return false;
    }

    bool PhysicsObject::getKinematic() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->kinematic;
        }

        // default return.
        return false;
    }

    void PhysicsObject::setMass(PxReal mass) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setMass(mass);
        }
    }

    void PhysicsObject::setMassSpaceInertia(PxVec3 mass) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setMassSpaceInertiaTensor(mass);
        }
    }

    void PhysicsObject::setAngularDamping(PxReal angularDamping) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setAngularDamping(angularDamping);
        }
    }

    void PhysicsObject::setAngularVelocity(PxVec3 angularVelocity) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setAngularVelocity(angularVelocity);
        }
    }

    void PhysicsObject::setLinearDamping(PxReal linearDamping) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setLinearDamping(linearDamping);
        }
    }

    void PhysicsObject::setLinearVelocity(PxVec3 linearVelocity) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setLinearVelocity(linearVelocity);
        }
    }

    void PhysicsObject::addForce(PxVec3 f_amount, force type) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic) {

                switch (type) {
                case force::force:
                    underlying_obj->rb.rigidDynamic->addForce(f_amount, PxForceMode::eFORCE);
                    break;

                case force::impulse:
                    underlying_obj->rb.rigidDynamic->addForce(f_amount, PxForceMode::eIMPULSE);
                    break;

                case force::velocityChanged:
                    underlying_obj->rb.rigidDynamic->addForce(f_amount, PxForceMode::eVELOCITY_CHANGE);
                    break;

                case force::acceleration:
                    underlying_obj->rb.rigidDynamic->addForce(f_amount, PxForceMode::eACCELERATION);
                    break;

                default:
                    break;
                }
            }
        }
    }

    void PhysicsObject::addTorque(PxVec3 f_amount, force type) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic) {

                switch (type) {
                case force::force:
                    underlying_obj->rb.rigidDynamic->addTorque(f_amount, PxForceMode::eFORCE);
                    break;

                case force::impulse:
                    underlying_obj->rb.rigidDynamic->addTorque(f_amount, PxForceMode::eIMPULSE);
                    break;

                case force::velocityChanged:
                    underlying_obj->rb.rigidDynamic->addTorque(f_amount, PxForceMode::eVELOCITY_CHANGE);
                    break;

                case force::acceleration:
                    underlying_obj->rb.rigidDynamic->addTorque(f_amount, PxForceMode::eACCELERATION);
                    break;

                default:
                    break;
                }
            }
        }
    }

    

/*-----------------------------------------------------------------------------*/
/*                               PVD                                           */
/*-----------------------------------------------------------------------------*/
    PxPvd* PVD::createPvd(PxFoundation* foundation, const char* ip) {

        mPVD = PxCreatePvd(*foundation);
        mTransport = PxDefaultPvdSocketTransportCreate(ip, 5425, 10);
        mPVD->connect(*mTransport, PxPvdInstrumentationFlag::eALL);

        //mPVD->getTransport()->release();

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

    PxPvdTransport* PVD::getTransport() {

        return mTransport;

        //return mPVD->getTransport();
    }

    PxPvd*& PVD::pvd__() {

        return mPVD;
    }

    PxPvd* const& PVD::pvd__() const {

        return mPVD;
    }
    
    void EventCallBack::onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) {
        printf("CALLBACK: onConstraintBreak\n");
    }
    void EventCallBack::onWake(PxActor** actors, PxU32 count) {
        printf("CALLBACK: onWake\n");
    }
    void EventCallBack::onSleep(PxActor** actors, PxU32 count) {
        printf("CALLBACK: onSleep\n");
    }
    void EventCallBack::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 count) {
        printf("CALLBACK: onContact\n");
        printf("PAIRS: %d\n", count);

        while (count--) {

            const PxContactPair& current = *pairs++;

            if (current.events & (PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_CCD))
                printf("Shape is entering trigger contact volume\n");

            if (current.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
                printf("Shape is still within trigger contact volume\n");

            if (current.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
                printf("Shape is leaving trigger contact volume\n");

            //if (physx_system::isTriggerShape(current.shapes[0]) && physx_system::isTriggerShape(current.shapes[1]))
            //    printf("Trigger-trigger overlap detected\n");
        }

    }
    void EventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count) {
        printf("CALLBACK: onTrigger\n");
        printf("PAIRS: %d\n", count);

        while (count--) {

            trigger state = trigger::none;
            const PxTriggerPair& current = *pairs++;

            auto trigger_id = *reinterpret_cast<phy_uuid::UUID*>(current.triggerActor->userData);
            auto other_id = *reinterpret_cast<phy_uuid::UUID*>(current.otherActor->userData);
            printf("trigger actor %llu, other actor %llu \n", trigger_id, other_id);

            if (current.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) { // OnTriggerEnter
                state = trigger::onTriggerEnter;
                printf("Shape is entering trigger volume\n");
            }
            if (current.status & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) {// OnTriggerStay
                state = trigger::onTriggerStay;
                printf("Shape is still within trigger volume\n");
            }
            if (current.status & PxPairFlag::eNOTIFY_TOUCH_LOST) { // OnTriggerExit
                state = trigger::onTriggerExit;
                printf("Shape is leaving trigger volume\n");
            }

            // Store all the ID of the actors that collided with trigger)
            std::queue<TriggerManifold>* temp = physx_system::currentWorld->getTriggerData();

            // Add in the TriggerManifold data into the queue
            temp->emplace(TriggerManifold{ trigger_id, other_id, state });
            
            //int msize = temp->size();
            //while(!temp->empty()) {
            //
            //    TriggerManifold val = temp->front();
            //    std::cout << "TRI: " << val.triggerID << " OTH: " << val.otherID;
            //    printf(" STATE: %d\n", val.status);
            //    
            //    temp->pop();
            //}
        }
    }
    void EventCallBack::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) {
        printf("CALLBACK: onAdvance\n");
    }
}

