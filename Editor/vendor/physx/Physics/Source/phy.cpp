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
                underlying_obj->rs.rigidStatic->release();

            else if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->release();
        }
        */

        scene->release();

        if (mDispatcher)
        {
            mDispatcher->release();
            mDispatcher = nullptr;
        }

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
        obj.id = phy_uuid::UUID{};

        // store the object
        m_objects.emplace_back(obj);
        all_objects.insert({ obj.id, m_objects.size() - 1 }); // add back the m_objects last element

        // return the object i created
        return PhysicsObject{ obj.id, this }; // a copy
    }

    void PhysxWorld::removeInstance(PhysicsObject obj)
    {
        // check what need to release 

        // release rigidstatic or rigiddynamic
        if (all_objects.contains(obj.id)) {

            PhysxObject* underlying_obj = &m_objects[all_objects.at(obj.id)];

            if (underlying_obj->rigidID == rigid::rstatic)
                underlying_obj->rs.rigidStatic->release();

            else if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->release();

            // release shape
            //underlying_obj->m_shape->release();
        }

        // check/find the id from the obj vector then if match 
        // remove from that vector then release
        auto begin = std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& elem) { return elem.id == obj.id; });
        //begin->destroy();
        m_objects.erase(begin);
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
            PxRigidStatic* rstat = underlying_obj->rs.rigidStatic;
            PxRigidDynamic* rdyna = underlying_obj->rd.rigidDynamic;

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
                underlying_obj->rs.rigidStatic = physx_system::getPhysics()->createRigidStatic(temp_trans);
                underlying_obj->rs.rigidStatic->userData = this;
                printf("actl value %llu vs pointer value: %llu \n", id, reinterpret_cast<PhysicsObject*>(underlying_obj->rs.rigidStatic->userData)->id);
                world->scene->addActor(*underlying_obj->rs.rigidStatic);
            }
            else if (type == rigid::rdynamic) {
                underlying_obj->rd.rigidDynamic = physx_system::getPhysics()->createRigidDynamic(temp_trans);
                underlying_obj->rd.rigidDynamic->userData = this;
                printf("actl value %llu vs pointer value: %llu \n", id, reinterpret_cast<PhysicsObject*>(underlying_obj->rd.rigidDynamic->userData)->id);
                world->scene->addActor(*underlying_obj->rd.rigidDynamic);
            }
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
                    underlying_obj->rs.rigidStatic->detachShape(*underlying_obj->m_shape);

                else if (underlying_obj->rigidID == rigid::rdynamic)
                    underlying_obj->rd.rigidDynamic->detachShape(*underlying_obj->m_shape);
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
                underlying_obj->rs.rigidStatic->attachShape(*underlying_obj->m_shape);

            else if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->attachShape(*underlying_obj->m_shape);

            // later check where need to release shape
            //underlying_obj->m_shape->release();
        }
    }

    void PhysicsObject::removeShape() {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->shape != shape::none) {

                if (underlying_obj->rigidID == rigid::rstatic)
                    underlying_obj->rs.rigidStatic->detachShape(*underlying_obj->m_shape);

                else if (underlying_obj->rigidID == rigid::rdynamic)
                    underlying_obj->rd.rigidDynamic->detachShape(*underlying_obj->m_shape);

                underlying_obj->shape = shape::none; // set new shape enum
            }

            // need check whether need release shape need or not
        }
    }

    void PhysicsObject::enableKinematic(bool kine) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic) {

                underlying_obj->rd.rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kine);

                underlying_obj->kinematic = kine;
            }
        }
    }

    void PhysicsObject::disableGravity(bool grav) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            underlying_obj->rd.rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, grav);

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

        // default return.
        return PxVec3{};
    }

    void PhysicsObject::setPosOrientation(PxVec3 pos, PxQuat quat) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rstatic)
                underlying_obj->rs.rigidStatic->setGlobalPose(PxTransform{ PxVec3(pos.x, pos.y, pos.z), quat });

            else if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->setGlobalPose(PxTransform{ PxVec3(pos.x, pos.y, pos.z), quat });
        }
    }

    PxQuat PhysicsObject::getOrientation() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rstatic)
                return underlying_obj->rs.rigidStatic->getGlobalPose().q;

            else if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rd.rigidDynamic->getGlobalPose().q;
        }

        // default return.
        return PxQuat{};
    }

    PxReal PhysicsObject::getMass() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rd.rigidDynamic->getMass();
        }

        // default return.
        return PxReal{};
    }

    PxReal PhysicsObject::getInvMass() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rd.rigidDynamic->getInvMass();
        }

        // default return.
        return PxReal{};
    }

    PxReal PhysicsObject::getAngularDamping() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rd.rigidDynamic->getAngularDamping();
        }

        // default return.
        return PxReal{};
    }

    PxVec3 PhysicsObject::getAngularVelocity() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rd.rigidDynamic->getAngularVelocity();
        }

        // default return.
        return PxVec3{};
    }

    PxReal PhysicsObject::getLinearDamping() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rd.rigidDynamic->getLinearDamping();
        }

        // default return.
        return PxReal{};
    }

    PxVec3 PhysicsObject::getLinearVelocity() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                return underlying_obj->rd.rigidDynamic->getLinearVelocity();
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
                underlying_obj->rd.rigidDynamic->setMass(mass);
        }
    }

    void PhysicsObject::setMassSpaceInertia(PxVec3 mass) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->setMassSpaceInertiaTensor(mass);
        }
    }

    void PhysicsObject::setAngularDamping(PxReal angularDamping) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->setAngularDamping(angularDamping);
        }
    }

    void PhysicsObject::setAngularVelocity(PxVec3 angularVelocity) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->setAngularVelocity(angularVelocity);
        }
    }

    void PhysicsObject::setLinearDamping(PxReal linearDamping) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->setLinearDamping(linearDamping);
        }
    }

    void PhysicsObject::setLinearVelocity(PxVec3 linearVelocity) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic)
                underlying_obj->rd.rigidDynamic->setLinearVelocity(linearVelocity);
        }
    }

    void PhysicsObject::addForce(PxVec3 f_amount, force type) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigidID == rigid::rdynamic) {

                switch (type) {
                case force::force:
                    underlying_obj->rd.rigidDynamic->addForce(f_amount, PxForceMode::eFORCE);
                    break;

                case force::impulse:
                    underlying_obj->rd.rigidDynamic->addForce(f_amount, PxForceMode::eIMPULSE);
                    break;

                case force::velocityChanged:
                    underlying_obj->rd.rigidDynamic->addForce(f_amount, PxForceMode::eVELOCITY_CHANGE);
                    break;

                case force::acceleration:
                    underlying_obj->rd.rigidDynamic->addForce(f_amount, PxForceMode::eACCELERATION);
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
                    underlying_obj->rd.rigidDynamic->addTorque(f_amount, PxForceMode::eFORCE);
                    break;

                case force::impulse:
                    underlying_obj->rd.rigidDynamic->addTorque(f_amount, PxForceMode::eIMPULSE);
                    break;

                case force::velocityChanged:
                    underlying_obj->rd.rigidDynamic->addTorque(f_amount, PxForceMode::eVELOCITY_CHANGE);
                    break;

                case force::acceleration:
                    underlying_obj->rd.rigidDynamic->addTorque(f_amount, PxForceMode::eACCELERATION);
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

            auto trigger_id = reinterpret_cast<PhysicsObject*>(pairs->triggerActor->userData)->id;
            auto other_id = reinterpret_cast<PhysicsObject*>(pairs->otherActor->userData)->id;
            printf("trigger actor %llu, other actor %llu \n", trigger_id, other_id);

            const PxTriggerPair& current = *pairs++;

            if (current.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) // OnTriggerEnter
                printf("Shape is entering trigger volume\n");

            if (current.status & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) // OnTriggerStay
                printf("Shape is still within trigger volume\n");

            if (current.status & PxPairFlag::eNOTIFY_TOUCH_LOST) // OnTriggerExit
                printf("Shape is leaving trigger volume\n");
        }
    }
    void EventCallBack::onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) {
        printf("CALLBACK: onAdvance\n");
    }
}

