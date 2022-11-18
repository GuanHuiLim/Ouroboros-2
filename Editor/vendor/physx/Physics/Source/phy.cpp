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
            if (!mFoundation) 
                throw("PxCreateFoundation failed!");

            return mFoundation;
        }

        PxPhysics* createPhysics() {

            mToleranceScale.length = 1;        // typical length of an object
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
            if (isTrigger(shape->getSimulationFilterData()))
                return true;

            // detects our emulated triggers using the simulation filter callback
            if (shape->userData)
                return true;

            return false;
        }

        void setCurrentWorld(PhysxWorld* world) {

            // retrieving the current world for me to access (get/set) neccessary data
            currentWorld = world;
        }

        PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
                                                PxFilterObjectAttributes attributes1, PxFilterData filterData1,
                                                PxPairFlags& pairFlags, const void* /*constantBlock*/,
                                                PxU32 /*constantBlockSize*/) {

            //let triggers through
            if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {
            
                pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
                return PxFilterFlag::eDEFAULT;
            }
            
            // generate contacts for all that were not filtered above
            pairFlags = PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_DISCRETE_CONTACT | PxPairFlag::eCONTACT_DEFAULT;

            // trigger the contact callback for pairs (A,B) where the filtermask of A contains the ID of B and vice versa.
            if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1)) {
                pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST
                           | PxPairFlag::eNOTIFY_CONTACT_POINTS;
            }
            
            return PxFilterFlag::eDEFAULT;

        }

        void setupFiltering(PxShape* shape) {
            
            PxFilterData filterData;
            filterData.word0 = 1; //filterGroup; // word0 = own ID
            filterData.word1 = 1; //filterMask;  // word1 = ID mask to filter pairs that trigger a contact callback;
           
            shape->setSimulationFilterData(filterData);
        }

        void init() {

            createFoundation();

            if constexpr (use_debugger) {
                printf("DEBUGGER ON\n");
                myPVD.createPvd(mFoundation, "172.28.68.41");
            }
            else {
                printf("DEBUGGER OFF\n");
            }

            createPhysics();
        }

        void shutdown() {

            mPhysics->release();

            // pvd release here
            if constexpr (use_debugger) {

                myPVD.pvd__()->release();

                myPVD.getTransport()->release();
            }

            mFoundation->release();
        }

    }

/*-----------------------------------------------------------------------------*/
/*                               PhysxWorld                                    */
/*-----------------------------------------------------------------------------*/
    PhysxWorld::PhysxWorld(PxVec3 grav)
    {
        // Setup scene description
        PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
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
        sceneDesc.filterShader = physx_system::contactReportFilterShader;

        scene = mPhysics->createScene(sceneDesc);

        // just take note we have 5000 objects reserved.
        m_objects.reserve(5000);
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

            if (underlying_obj->rigid_type == rigid::rstatic)
                underlying_obj->rb.rigidStatic->release();

            else if (underlying_obj->rigid_type == rigid::rdynamic)
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

    void PhysxWorld::removeInstance(PhysicsObject obj) {

        // release rigidstatic or rigiddynamic
        if (all_objects.contains(obj.id)) {

            std::size_t current_index = all_objects.at(obj.id);

            PhysxObject* underlying_obj = &m_objects[all_objects.at(obj.id)];

            if (underlying_obj->rigid_type == rigid::rstatic)
                underlying_obj->rb.rigidStatic->release();

            else if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->release();

            // release shape
            //underlying_obj->m_shape->release();

            all_objects.erase(obj.id);
            //all_objects.erase(*underlying_obj->id.get());

            // UPDATE THE INDEX AFTER DELETING
            for (auto i = all_objects.begin(); i != all_objects.end(); i++) {

                if (i->second > current_index) {
                    std::size_t current = i->second;
                    i->second = current - 1;
                }
            }

            // CHECK ON TRIGGER 
            //updateTriggerState(obj.id);

            // check/find the id from the obj vector then if match 
            // remove from that vector then release
            // NOTE: DON't MIXUP ELEM ID and OBJ ID HERE, 2 different types.
            auto begin = std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& elem) { return *elem.id == obj.id; });
            //auto begin = std::find_if(m_objects.begin(), m_objects.end(), [&](auto&& elem) { return *elem.id == *underlying_obj->id.get(); });

            //if(begin != m_objects.end())
            //    m_objects.erase(begin);

            m_objects.erase(begin);
        }
    }

    PhysxObject::PhysxObject(const PhysxObject& other) : matID(other.matID),
                                                          shape_type(other.shape_type),
                                                          rigid_type(other.rigid_type),
                                                          lockPositionAxis(other.lockPositionAxis),
                                                          lockRotationAxis(other.lockRotationAxis),
                                                          is_trigger(other.is_trigger),
                                                          gravity_enabled(other.gravity_enabled),
                                                          is_kinematic(other.is_kinematic),
                                                          is_collider(other.is_collider),
        m_shape{nullptr},
        rb {},
        // Create new UUID when u attempt to make a copy
        id { std::make_unique<phy_uuid::UUID>() }
    {
    }

    PhysxObject& PhysxObject::operator=(const PhysxObject& object) {

        PhysxObject copy{ object };
        std::swap(*this, copy);

        return *this;
    }

    PhysicsObject PhysxWorld::duplicateObject(phy_uuid::UUID id) {

        if (all_objects.contains(id)) {

            std::size_t index = all_objects.at(id);

            PhysxObject physxObject{ m_objects.at(index) };

            PhysicsObject physicsNewObject { *physxObject.id, this };

            // we insert into the list now after constructing the objects properly.
            m_objects.emplace_back(std::move(physxObject));
            
            // IMPORTANT
            PhysxObject& initialized_object = m_objects.back();

            all_objects.insert({ *initialized_object.id, m_objects.size() - 1 }); // add back the m_objects last element
            
            // we set this object's properties to be equal to the object we are copying from.
            // does order matter?
            // are they allocating memory at the back in physX

            // MATERIAL
            PxMaterial* material = physx_system::currentWorld->mat.at(initialized_object.matID);
            physicsNewObject.setMaterial(Material{ material->getStaticFriction(), material->getDynamicFriction(), material->getRestitution() });

            // RIGID
            physicsNewObject.setRigidType(initialized_object.rigid_type);

            // SHAPE
            //physicsNewObject.setShape(initialized_object.shape_type);
           
            if (initialized_object.shape_type == shape::box) {
                PxBoxGeometry box = m_objects.at(index).m_shape->getGeometry().box();
                physicsNewObject.reAttachShape(initialized_object.rigid_type, box);

                //PxVec3 boxProp = m_objects.at(index).m_shape->getGeometry().box().halfExtents;
                //physicsNewObject.setBoxProperty(boxProp.x, boxProp.y, boxProp.z);
            }
            else if (initialized_object.shape_type == shape::sphere) {
                physicsNewObject.reAttachShape(initialized_object.rigid_type, m_objects.at(index).m_shape->getGeometry().sphere());
                //PxReal rad = m_objects.at(index).m_shape->getGeometry().sphere().radius;
                //physicsNewObject.setSphereProperty(rad);
            } 
            else if (initialized_object.shape_type == shape::capsule) {
                physicsNewObject.reAttachShape(initialized_object.rigid_type, m_objects.at(index).m_shape->getGeometry().capsule());
                //PxCapsuleGeometry cap = m_objects.at(index).m_shape->getGeometry().capsule();
                //physicsNewObject.setCapsuleProperty(cap.radius, cap.halfHeight);
            } 
            //else if (initialized_object.shape_type == shape::plane) {
            //    PxPlaneGeometry plane = m_objects.at(index).m_shape->getGeometry().plane();
            //    physicsNewObject.setPlaneProperty();
            //}

            // LOCK POSTION AXIS
            LockingAxis posLock = initialized_object.lockPositionAxis;
            if (posLock.x_axis)
                physicsNewObject.lockPositionX(true);
            if (posLock.y_axis)
                physicsNewObject.lockPositionY(true);
            if (posLock.z_axis)
                physicsNewObject.lockPositionZ(true);

            // LOCK ROTATION AXIS
            LockingAxis rotLock = initialized_object.lockRotationAxis;
            if (rotLock.x_axis)
                physicsNewObject.lockRotationX(true);
            if (rotLock.y_axis)
                physicsNewObject.lockRotationY(true);
            if (rotLock.z_axis)
                physicsNewObject.lockRotationZ(true);

            // TRIGGER
            physicsNewObject.setTriggerShape(initialized_object.is_trigger);

            // GRAVITY
            physicsNewObject.enableGravity(initialized_object.gravity_enabled);

            // KINEMATIC
            physicsNewObject.enableKinematic(initialized_object.is_kinematic);

            // COLLIDER
            physicsNewObject.enableCollider(initialized_object.is_collider);
            
            return physicsNewObject; // a copy
        }

        return PhysicsObject{}; // return empty
    }

    void PhysxWorld::updateTriggerState(phy_uuid::UUID id) {

        std::queue<TriggerManifold>* temp = &m_triggerCollisionPairs;

        while (!m_triggerCollisionPairs.empty()) {

            TriggerManifold val = m_triggerCollisionPairs.front();

            if (val.triggerID == id || val.otherID == id)
                temp->emplace(val.triggerID, val.otherID, trigger::onTriggerExit, true);

            m_triggerCollisionPairs.pop();
        }

        m_triggerCollisionPairs = *temp;

        //for (TriggerManifold& temp : m_triggerCollisionPairs) {
        //
        //    if (temp.isStaying) {
        //        temp.status = trigger::onTriggerStay;
        //        printf("Shape is still within trigger volume\n");
        //    }
        //}

       //std::queue<TriggerManifold> temp;
       //
       //while(!m_triggerCollisionPairs.empty()) {
       //
       //    TriggerManifold val = m_triggerCollisionPairs.front();
       //
       //    if (all_objects.contains(val.triggerID) && all_objects.contains(val.otherID))
       //        temp.emplace(val);
       //
       //    m_triggerCollisionPairs.pop();
       //}
       //
       //m_triggerCollisionPairs = temp;
    }

    bool PhysxWorld::hasObject(phy_uuid::UUID id) {

        if (all_objects.contains(id))
            return true;

        return false;
    }
    
    std::map<phy_uuid::UUID, std::size_t>* PhysxWorld::getAllObject() {

        return &all_objects;
    }

    std::queue<TriggerManifold>* PhysxWorld::getTriggerData() {

        //updateTriggerState();

        return &m_triggerCollisionPairs;
    }

    void PhysxWorld::clearTriggerData() {

        while (!m_triggerCollisionPairs.empty())
            m_triggerCollisionPairs.pop();

        //std::queue<TriggerManifold> empty;
        //std::swap(m_triggerCollisionPairs, empty);
    }

    std::queue<ContactManifold>* PhysxWorld::getCollisionData() {

        return &m_collisionPairs;
    }

    void PhysxWorld::clearCollisionData() {

        while (!m_collisionPairs.empty())
            m_collisionPairs.pop();
    }

    //void PhysxWorld::createPhysicsObjectFromPhysxObject(PhysicsObject& phyiscsNewObject, PhysxObject& objectToCopyFrom)
    //{
    //    // Create new PhysxObject - copy and assign all the old data
    //    PhysxObject newPhysxObject = objectToCopyFrom;
    //
    //    // Assign PhysicsObject with new UUID and the world
    //    phyiscsNewObject.id = *newPhysxObject.id;
    //    phyiscsNewObject.world= physx_system::currentWorld;
    //
    //    // Retrieve old rigidbody data (transform)
    //    PxTransform trans;
    //    if (newPhysxObject.rigid_type == rigid::rstatic)
    //        trans = objectToCopyFrom.rb.rigidStatic->getGlobalPose();
    //    else if (newPhysxObject.rigid_type == rigid::rdynamic)
    //        trans = objectToCopyFrom.rb.rigidDynamic->getGlobalPose();
    //
    //    // Create new shape and rigidbody data 
    //    if (newPhysxObject.shape_type == shape::box)
    //        phyiscsNewObject.reCreateRigidbody(newPhysxObject, trans, newPhysxObject.rigid_type, objectToCopyFrom.m_shape->getGeometry().box());
    //    else if (newPhysxObject.shape_type == shape::sphere)
    //        phyiscsNewObject.reCreateRigidbody(newPhysxObject, trans, newPhysxObject.rigid_type, objectToCopyFrom.m_shape->getGeometry().sphere());
    //    else if (newPhysxObject.shape_type == shape::capsule)
    //        phyiscsNewObject.reCreateRigidbody(newPhysxObject, trans, newPhysxObject.rigid_type, objectToCopyFrom.m_shape->getGeometry().capsule());
    //    else if (newPhysxObject.shape_type == shape::plane)
    //        phyiscsNewObject.reCreateRigidbody(newPhysxObject, trans, newPhysxObject.rigid_type, objectToCopyFrom.m_shape->getGeometry().plane());
    //    
    //    // Store the new duplicate objects into vector/map
    //    m_objects.emplace_back(std::move(newPhysxObject));
    //    all_objects.insert({ phyiscsNewObject.id,m_objects.size() - 1 });
    //}

    RaycastHit PhysxWorld::raycast(PxVec3 origin, PxVec3 direction, PxReal distance) {

        RaycastHit hit{};
        PxRaycastBuffer hitBuffer;

        PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eUV | PxHitFlag::eMESH_ANY;

        hit.intersect = scene->raycast(origin, direction, distance, hitBuffer, hitFlags);

        // HAVE INTERSECTION
        if (hit.intersect) {

            hit.object_ID = *reinterpret_cast<phy_uuid::UUID*>(hitBuffer.block.actor->userData);

            hit.position = hitBuffer.block.position;
            hit.normal = hitBuffer.block.normal;
            hit.distance = hitBuffer.block.distance;
        }

        return hit;
    }

    std::vector<RaycastHit> PhysxWorld::raycastAll(PxVec3 origin, PxVec3 direction, PxReal distance) {

        const PxU32 bufferSize = 200;// 256;           // size of the buffer       
        PxRaycastHit hitBuffer[bufferSize];            // storage of the buffer results
        PxRaycastBuffer buffer(hitBuffer, bufferSize); // blocking and touching hits stored here

        std::vector<RaycastHit> hitAll{}; // store all the raycast hit

        PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eUV | PxHitFlag::eMESH_MULTIPLE;
        PxQueryFlags queryFlags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::eNO_BLOCK;

        scene->raycast(origin, direction, distance, buffer, hitFlags, PxQueryFilterData(queryFlags));

        // loop based on how many touches return by the query
        for (PxU32 i = 0; i < buffer.nbTouches; i++)
        {
            RaycastHit hit{};

            hit.intersect = true;
            hit.object_ID = *reinterpret_cast<phy_uuid::UUID*>(buffer.touches[i].actor->userData);

            hit.position = buffer.touches[i].position;
            hit.normal = buffer.touches[i].normal;
            hit.distance = buffer.touches[i].distance;

            hitAll.emplace_back(hit); // store each hit data into the container
        }

        return hitAll;
    }

/*-----------------------------------------------------------------------------*/
/*                               PhysicsObject                                 */
/*-----------------------------------------------------------------------------*/
    void PhysicsObject::setRigidType(rigid type) {

        PxTransform temp_trans{ PxVec3(0) }; // set default to 0

        // CHECK GOT THE INSTANCE CREATED OR NOT
        if (world->all_objects.contains(id)) {

            bool changingType = false;

            //PhysxObject* underlying_obj = world->all_objects.at(id);
            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            PxRigidStatic* rstat = underlying_obj->rb.rigidStatic;
            PxRigidDynamic* rdyna = underlying_obj->rb.rigidDynamic;

            // CHECK IF HAVE RIGIDBODY CREATED OR NOT
            // CHECK IF THIS OBJ HAVE RSTATIC OR RDYAMIC INIT OR NOT
            if (rstat) {
                changingType = true;
                temp_trans = rstat->getGlobalPose();
                world->scene->removeActor(*rstat);
                underlying_obj->rb.rigidStatic = nullptr; // clear the current data
            }
            else if (rdyna) {
                changingType = true;
                temp_trans = rdyna->getGlobalPose();
                world->scene->removeActor(*rdyna);
                underlying_obj->rb.rigidDynamic = nullptr;
            }

            // ASSIGN TO THE NEW RIGID TYPE
            underlying_obj->rigid_type = type;

            // CREATE RSTATIC OR RDYNAMIC ACCORDINGLY
            if (type == rigid::rstatic) {
                underlying_obj->rb.rigidStatic = physx_system::getPhysics()->createRigidStatic(temp_trans);
                underlying_obj->rb.rigidStatic->userData = underlying_obj->id.get();
                //printf("STAT: actl value %llu vs pointer value: %llu \n", id, *reinterpret_cast<phy_uuid::UUID*>(underlying_obj->rb.rigidStatic->userData));
                world->scene->addActor(*underlying_obj->rb.rigidStatic);
            }
            else if (type == rigid::rdynamic) {
                underlying_obj->rb.rigidDynamic = physx_system::getPhysics()->createRigidDynamic(temp_trans);
                underlying_obj->rb.rigidDynamic->userData = underlying_obj->id.get();
                //printf("DYNA: actl value %llu vs pointer value: %llu \n", id, *reinterpret_cast<phy_uuid::UUID*>(underlying_obj->rb.rigidDynamic->userData));
                world->scene->addActor(*underlying_obj->rb.rigidDynamic);
            }

            // CHECK WHETHER IS CHANGING OF RIGID TYPE
            if (changingType) {

                // ATTACH THE NEW SHAPE
                if (underlying_obj->m_shape) {

                    //printf("SHAPE TYPE: %d", underlying_obj->shape);
                    
                    // ATTACH THE NEW SHAPE BASED THE SHAPE TYPE
                    if (underlying_obj->shape_type == shape::box) 
                        reAttachShape(type, underlying_obj->m_shape->getGeometry().box());

                    else if (underlying_obj->shape_type == shape::sphere) 
                        reAttachShape(type, underlying_obj->m_shape->getGeometry().sphere());

                    else if (underlying_obj->shape_type == shape::plane) 
                        reAttachShape(type, underlying_obj->m_shape->getGeometry().plane());

                    else if (underlying_obj->shape_type == shape::capsule) 
                        reAttachShape(type, underlying_obj->m_shape->getGeometry().capsule());
                }
            }

            // Check how many actors created in the scene
            //PxActorTypeFlags desiredTypes = PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC;
            //PxU32 count = world->scene->getNbActors(desiredTypes);
            //PxActor** buffer = new PxActor * [count];
            
            //PxU32 noo = world->scene->getActors(desiredTypes, buffer, count);
            //printf("%d - actors\n\n", noo);
        }
    }

    template<typename Type>
    void PhysicsObject::reAttachShape(rigid rigidType, Type data) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            PxMaterial* material = world->mat.at(underlying_obj->matID); // might need check if this set or not

            underlying_obj->m_shape = physx_system::getPhysics()->createShape(data, *material, true);

            // ATTACH THE SHAPE TO THE OBJECT
            if (rigidType == rigid::rstatic)
                underlying_obj->rb.rigidStatic->attachShape(*underlying_obj->m_shape);

            else if (rigidType == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->attachShape(*underlying_obj->m_shape);

            physx_system::setupFiltering(underlying_obj->m_shape);
        }
    }

    template<typename Type>
    void PhysicsObject::reCreateRigidbody(PhysxObject& obj, PxTransform transform, rigid rigidType, Type data) {

        PxMaterial* material = world->mat.at(obj.matID); // might need check if this set or not

        obj.m_shape = physx_system::getPhysics()->createShape(data, *material, true);

        // ATTACH THE SHAPE TO THE OBJECT
        if (rigidType == rigid::rstatic) {
            obj.rb.rigidStatic = physx_system::getPhysics()->createRigidStatic(transform);
            obj.rb.rigidStatic->userData = obj.id.get();
            obj.rb.rigidStatic->attachShape(*obj.m_shape);

            world->scene->addActor(*obj.rb.rigidStatic);
        }
        else if (rigidType == rigid::rdynamic) {
            obj.rb.rigidDynamic = physx_system::getPhysics()->createRigidDynamic(transform);
            obj.rb.rigidDynamic->userData = obj.id.get();
            obj.rb.rigidDynamic->attachShape(*obj.m_shape);

            world->scene->addActor(*obj.rb.rigidDynamic);
        }
    
        physx_system::setupFiltering(obj.m_shape);
    }

    void PhysicsObject::setShape(shape shape) {

        //PxRigidActorExt::createExclusiveShape (another method)

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            PxMaterial* material = world->mat.at(underlying_obj->matID); // might need check if this set or not

            // CHECK IF HAVE SHAPE CREATED OR NOT
            if (underlying_obj->m_shape) {
            //if (underlying_obj->shape != shape::none) {

                // DETACH OLD SHAPE
                if (underlying_obj->rigid_type == rigid::rstatic)
                    underlying_obj->rb.rigidStatic->detachShape(*underlying_obj->m_shape);

                else if (underlying_obj->rigid_type == rigid::rdynamic)
                    underlying_obj->rb.rigidDynamic->detachShape(*underlying_obj->m_shape);
            }

            underlying_obj->shape_type = shape; // set new shape enum

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
            if (underlying_obj->rigid_type == rigid::rstatic)
                underlying_obj->rb.rigidStatic->attachShape(*underlying_obj->m_shape);

            else if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->attachShape(*underlying_obj->m_shape);

            // later check where need to release shape
            //underlying_obj->m_shape->release();

            physx_system::setupFiltering(underlying_obj->m_shape);
        }
    }

    void PhysicsObject::removeShape() {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->shape_type != shape::none) {

                if (underlying_obj->rigid_type == rigid::rstatic)
                    underlying_obj->rb.rigidStatic->detachShape(*underlying_obj->m_shape);

                else if (underlying_obj->rigid_type == rigid::rdynamic)
                    underlying_obj->rb.rigidDynamic->detachShape(*underlying_obj->m_shape);

                // release shape
                underlying_obj->m_shape->release();
                underlying_obj->shape_type = shape::none; // set new shape enum
            }
        }
    }

    void PhysicsObject::lockPositionX(bool lock) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic) {

                //// ROTATE IN Z
                //PxRigidDynamicLockFlag::eLOCK_LINEAR_Z | PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
                //// ROTATE IN Y
                //PxRigidDynamicLockFlag::eLOCK_LINEAR_Y | PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;

                underlying_obj->rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, lock);

                underlying_obj->lockPositionAxis.x_axis = lock;
            }
        }
    }

    void PhysicsObject::lockPositionY(bool lock) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic) {

                underlying_obj->rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, lock);

                underlying_obj->lockPositionAxis.y_axis = lock;
            }
        }
    }

    void PhysicsObject::lockPositionZ(bool lock) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic) {

                underlying_obj->rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, lock);

                underlying_obj->lockPositionAxis.z_axis = lock;
            }
        }
    }

    void PhysicsObject::lockRotationX(bool lock) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic) {

                underlying_obj->rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, lock);

                underlying_obj->lockRotationAxis.x_axis = lock;
            }
        }
    }

    void PhysicsObject::lockRotationY(bool lock) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic) {

                underlying_obj->rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, lock);

                underlying_obj->lockRotationAxis.y_axis = lock;
            }
        }
    }

    void PhysicsObject::lockRotationZ(bool lock) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic) {

                underlying_obj->rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, lock);

                underlying_obj->lockRotationAxis.z_axis = lock;
            }
        }
    }

    void PhysicsObject::enableCollider(bool collide) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type != rigid::none) {

                if (underlying_obj->m_shape) {

                    underlying_obj->m_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, collide);

                    underlying_obj->is_collider = collide;
                }
            }
        }
    }

    void PhysicsObject::enableKinematic(bool kine) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic) {

                underlying_obj->rb.rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kine);

                underlying_obj->is_kinematic = kine;
            }
        }
    }

    void PhysicsObject::enableGravity(bool enable) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            
            underlying_obj->rb.rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !enable);

            underlying_obj->gravity_enabled = enable;
        }
    }

    void PhysicsObject::setTriggerShape(bool trigger) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->m_shape) {

                underlying_obj->is_trigger = trigger;

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

            if (underlying_obj->shape_type == shape::box)
                underlying_obj->m_shape->setGeometry(PxBoxGeometry(halfextent_width, halfextent_height, halfextent_depth));
            //underlying_obj->m_shape->getGeometry().box().halfExtents = PxVec3{ halfextent_width , halfextent_height, halfextent_depth };
        }
    }

    void PhysicsObject::setSphereProperty(float radius) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->shape_type == shape::sphere)
                underlying_obj->m_shape->setGeometry(PxSphereGeometry(radius));
            //underlying_obj->m_shape->getGeometry().sphere().radius = radius;
        }
    }

    void PhysicsObject::setCapsuleProperty(float radius, float halfHeight) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            if (underlying_obj->shape_type == shape::capsule) {
                underlying_obj->m_shape->setGeometry(PxCapsuleGeometry(radius, halfHeight));
                //underlying_obj->m_shape->getGeometry().capsule().radius = radius;
                //underlying_obj->m_shape->getGeometry().capsule().halfHeight = halfHeight;
            }
        }
    }

    LockingAxis PhysicsObject::getLockPositionAxis() const {

        LockingAxis posAxis{ false };

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            posAxis.x_axis = underlying_obj->lockPositionAxis.x_axis;
            posAxis.y_axis = underlying_obj->lockPositionAxis.y_axis;
            posAxis.z_axis = underlying_obj->lockPositionAxis.z_axis;
        }

        return posAxis;
    }

    LockingAxis PhysicsObject::getLockRotationAxis() const {

        LockingAxis rotAxis{ false };

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            rotAxis.x_axis = underlying_obj->lockRotationAxis.x_axis;
            rotAxis.y_axis = underlying_obj->lockRotationAxis.y_axis;
            rotAxis.z_axis = underlying_obj->lockRotationAxis.z_axis;
        }

        return rotAxis;
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

            if (underlying_obj->rigid_type == rigid::rstatic) {

                return PxVec3{ underlying_obj->rb.rigidStatic->getGlobalPose().p.x,
                               underlying_obj->rb.rigidStatic->getGlobalPose().p.y,
                               underlying_obj->rb.rigidStatic->getGlobalPose().p.z };
            }
            else if (underlying_obj->rigid_type == rigid::rdynamic) {

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

            //printf("M_OBJ SIZE: %d\n", world->m_objects.size());
            //printf("M_OBJ ID: %d\n", world->all_objects.at(id));

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rstatic)
                underlying_obj->rb.rigidStatic->setGlobalPose(PxTransform{ pos, quat });

            else if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setGlobalPose(PxTransform{ pos, quat });
        }
    }

    PxQuat PhysicsObject::getOrientation() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
            
            //underlying_obj->rb.rigidStatic->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, true);

            if (underlying_obj->rigid_type == rigid::rstatic)
                return underlying_obj->rb.rigidStatic->getGlobalPose().q;

            else if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getGlobalPose().q;
        }

        // default return.
        return PxQuat{};
    }

    PxReal PhysicsObject::getMass() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getMass();
        }

        // default return.
        return PxReal{};
    }

    PxReal PhysicsObject::getInvMass() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getInvMass();
        }

        // default return.
        return PxReal{};
    }

    PxReal PhysicsObject::getAngularDamping() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getAngularDamping();
        }

        // default return.
        return PxReal{};
    }

    PxVec3 PhysicsObject::getAngularVelocity() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getAngularVelocity();
        }

        // default return.
        return PxVec3{};
    }

    PxReal PhysicsObject::getLinearDamping() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getLinearDamping();
        }

        // default return.
        return PxReal{};
    }

    PxVec3 PhysicsObject::getLinearVelocity() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->rb.rigidDynamic->getLinearVelocity();
        }

        // default return.
        return PxVec3{};
    }

    bool PhysicsObject::isTrigger() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            return underlying_obj->is_trigger;
        }

        // default return.
        return false;
    }

    bool PhysicsObject::isGravityEnabled() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->gravity_enabled;
        }

        // default return.
        return false;
    }

    bool PhysicsObject::isKinematic() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                return underlying_obj->is_kinematic;
        }

        // default return.
        return false;
    }

    bool PhysicsObject::isColliderEnabled() const {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type != rigid::none)
                return underlying_obj->is_collider;
        }

        // default return.
        return true;
    }

    void PhysicsObject::setMass(PxReal mass) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setMass(mass);
        }
    }

    void PhysicsObject::setMassSpaceInertia(PxVec3 mass) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setMassSpaceInertiaTensor(mass);
        }
    }

    void PhysicsObject::setAngularDamping(PxReal angularDamping) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setAngularDamping(angularDamping);
        }
    }

    void PhysicsObject::setAngularVelocity(PxVec3 angularVelocity) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setAngularVelocity(angularVelocity);
        }
    }

    void PhysicsObject::setLinearDamping(PxReal linearDamping) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setLinearDamping(linearDamping);
        }
    }

    void PhysicsObject::setLinearVelocity(PxVec3 linearVelocity) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic)
                underlying_obj->rb.rigidDynamic->setLinearVelocity(linearVelocity);
        }
    }

    void PhysicsObject::addForce(PxVec3 f_amount, force type) {

        if (world->all_objects.contains(id)) {

            PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

            if (underlying_obj->rigid_type == rigid::rdynamic) {

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

            if (underlying_obj->rigid_type == rigid::rdynamic) {

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
    
/*-----------------------------------------------------------------------------*/
/*                           EVENT CALLBACK                                    */
/*-----------------------------------------------------------------------------*/
    void EventCallBack::onConstraintBreak(PxConstraintInfo* /*constraints*/, PxU32 /*count*/) {
        //printf("CALLBACK: onConstraintBreak\n");
    }
    void EventCallBack::onWake(PxActor** /*actors*/, PxU32 /*count*/) {
        //printf("CALLBACK: onWake\n");
    }
    void EventCallBack::onSleep(PxActor** /*actors*/, PxU32 /*count*/) {
        //printf("CALLBACK: onSleep\n");
    }
    void EventCallBack::onContact(const PxContactPairHeader& /*pairHeader*/, const PxContactPair* pairs, PxU32 count) {
        //printf("CALLBACK: onContact -- ");
        //printf("PAIRS: %d\n", count);

        while (count--) {

            collision state = collision::none;
            const PxContactPair& current = *pairs++;

            auto shape1_id = *reinterpret_cast<phy_uuid::UUID*>(current.shapes[0]->getActor()->userData);
            auto shape2_id = *reinterpret_cast<phy_uuid::UUID*>(current.shapes[1]->getActor()->userData);
            //printf("shape1 actor %llu, shape2 actor %llu \n", shape1_id, shape2_id);

            //current.shapes[0]->getGeometryType() // PxGeometryType::eBOX;
            
            const PxU32 bufferSize = 64;
            PxContactPairPoint contacts[bufferSize];
            PxU32 nbContacts = current.extractContacts(contacts, bufferSize);
            
            std::vector<ContactPoint> tempCP = {};
            PxU8 contactCount = current.contactCount;

            for (PxU32 j = 0; j < nbContacts; j++) {

                PxVec3 normal = contacts[j].normal;
                PxVec3 point = contacts[j].position;
                PxVec3 impulse = contacts[j].impulse;

                ContactPoint cp = { normal, point, impulse };
                tempCP.emplace_back(cp);
                //printf("%d - NORMAL: %f - X, %f - Y, %f - Z\n", j, normal.x, normal.y, normal.z);
                //printf("%d - POINT: %f - X, %f - Y, %f - Z\n", j, point.x, point.y, point.z);
                //printf("%d - IMPUSLE: %f - X, %f - Y, %f - Z\n", j, impulse.x, impulse.y, impulse.z);
            }

            if (current.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) { // OnCollisionEnter
                state = collision::onCollisionEnter;
                //printf("Shape is ENTERING CONTACT volume\n");
            }

            if (current.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) { // OnCollisionStay
                state = collision::onCollisionStay;
                //printf("Shape is STAYING CONTACT volume\n");
            }

            if (current.events & PxPairFlag::eNOTIFY_TOUCH_LOST) { // OnCollisionExit
                state = collision::onCollisionExit;
                //printf("Shape is LEAVING CONTACT volume\n");
            }

            // Store all the ID of the actors that collided
            std::queue<ContactManifold>* collision_data = physx_system::currentWorld->getCollisionData();

            // Add new ContactManifold data into the queue
            collision_data->emplace(ContactManifold{ shape1_id, shape2_id, state, tempCP, contactCount });

            //if (physx_system::isTriggerShape(current.shapes[0]) && physx_system::isTriggerShape(current.shapes[1]))
            //    printf("Trigger-trigger overlap detected\n");
        }

    }
    void EventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count) {
        //printf("CALLBACK: onTrigger -- ");
        //printf("PAIRS: %d\n", count);

        while (count--) {

            //bool stayTrigger = false;
            trigger state = trigger::none;
            const PxTriggerPair& current = *pairs++;

            auto trigger_id = *reinterpret_cast<phy_uuid::UUID*>(current.triggerActor->userData);
            auto other_id = *reinterpret_cast<phy_uuid::UUID*>(current.otherActor->userData);
            //printf("trigger actor %llu, other actor %llu \n", trigger_id, other_id);

            if (current.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) { // OnTriggerEnter
                //stayTrigger = true;
                state = trigger::onTriggerEnter;
                //printf("Shape is ENTERING TRIGGER volume\n");
            }
            if (current.status & PxPairFlag::eNOTIFY_TOUCH_LOST) { // OnTriggerExit
                //stayTrigger = false;
                state = trigger::onTriggerExit;
                //printf("trigger actor %llu, other actor %llu, state: %d\n", current.triggerActor->userData, current.otherActor->userData, state);
                //printf("Shape is LEAVING TRIGGER volume\n");
            }

            // Store all the ID of the actors that collided with trigger)
            std::queue<TriggerManifold>* trigger_data = physx_system::currentWorld->getTriggerData();

            // Add new TriggerManifold data into the queue
            trigger_data->emplace(TriggerManifold{ trigger_id, other_id, state });

            //// Check object exist
            //std::map<phy_uuid::UUID, int>* all_object = physx_system::currentWorld->getAllObject();
            //
            //if (all_object->contains(trigger_id) && all_object->contains(other_id)) {
            //
            //    // Add new TriggerManifold data into the queue
            //    trigger_data->emplace(TriggerManifold{ trigger_id, other_id, state });
            //}
            //
            //if (!all_object->contains(trigger_id) || !all_object->contains(other_id))
            //    trigger_data->emplace(TriggerManifold{ trigger_id, other_id, trigger::onTriggerExit, true });


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
    void EventCallBack::onAdvance(const PxRigidBody* const* /*bodyBuffer*/, const PxTransform* /*poseBuffer*/, const PxU32 /*count*/) {
        //printf("CALLBACK: onAdvance\n");
    }
}