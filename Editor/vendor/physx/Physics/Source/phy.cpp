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

static constexpr bool use_debugger = true;

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
PxCooking* mCooking;

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

            mToleranceScale.length = 1;             // typical length of an object
            mToleranceScale.speed = 9.81f;          // typical speed of an object, gravity*1s is a reasonable choice

            //if (PVD_DEBUGGER)
            if constexpr (use_debugger)
                mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, myPVD.pvd__());
            else
                mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale);

            // Init Cooking
            mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, PxCookingParams(mToleranceScale));

            
            if (!mCooking)
                throw("PxCreateCooking failed!");

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
            
            // trigger the contact callback for pairs (A,B) where the filtermask of A contains the ID of B and vice versa.
            if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1)) {

                //let triggers through but also within the layering
                if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {

                    pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
                    return PxFilterFlag::eDEFAULT;
                }

                // generate contacts for all that were not filtered above
                pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST | 
                             PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::eDETECT_DISCRETE_CONTACT | PxPairFlag::eSOLVE_CONTACT;
            }
            else {
                return PxFilterFlag::eSUPPRESS;
            }
            
            return PxFilterFlag::eDEFAULT;
        }

        void setupFiltering(PxShape* shape, PxU32 filterGroup, PxU32 filterMask) {
            
            PxFilterData filterData;
            filterData.word0 = filterGroup; // 1 // word0 = own ID
            filterData.word1 = filterMask;  // 1 // word1 = ID mask to filter pairs that trigger a contact callback;
           
            shape->setSimulationFilterData(filterData); // sets the filter data that is used during the physics simulation

            shape->setQueryFilterData(filterData); // sets the filter data that is used when querying the scene for collision detection
        }

        void init() {

            createFoundation();

            if constexpr (use_debugger) {
                printf("DEBUGGER ON\n");
                myPVD.createPvd(mFoundation, "localhost");
            }
            else {
                printf("DEBUGGER OFF\n");
            }

            createPhysics();
        }

        void shutdown() {

            mCooking->release();

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

        // create the character controller
        //control_manager = PxCreateControllerManager(*scene);

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
        obj.m_material = mPhysics->createMaterial(.4f, .2f, .0f);

        // This is important!
        phy_uuid::UUID generated_uuid = *obj.id;
        if (obj.rigid_type == rigid::rstatic)
        {
            obj.rb.rigidStatic = mPhysics->createRigidStatic(PxTransform{ 0,0,0 });
            obj.rb.rigidStatic->userData = obj.id.get();
            scene->addActor(*obj.rb.rigidStatic);
        }
        else
        {
            obj.rb.rigidDynamic = mPhysics->createRigidDynamic(PxTransform{ 0,0,0 });
            obj.rb.rigidDynamic->userData = obj.id.get();
            scene->addActor(*obj.rb.rigidDynamic);
        }

        // store the object
        m_objects.emplace_back(std::move(obj));
        all_objects.insert({ generated_uuid, m_objects.size() - 1 }); // add back the m_objects last element
        
        
        // return the object i created
        return PhysicsObject{ generated_uuid }; // a copy
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

    PhysxObject::PhysxObject(const PhysxObject& other) : //matID(other.matID),
                                                         m_material{ other.m_material },
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
                                                         filterIn(other.filterIn),
                                                         filterOut(other.filterOut),
                                                         // Create new UUID when u attempt to make a copy
                                                         id{ std::make_unique<phy_uuid::UUID>() }   {}

    PhysxObject& PhysxObject::operator=(const PhysxObject& object) {

        PhysxObject copy{ object };
        std::swap(*this, copy);

        return *this;
    }

    PhysicsObject PhysxWorld::duplicateObject(phy_uuid::UUID id) {

        if (all_objects.contains(id)) {

            size_t index = all_objects.at(id);

            PhysxObject physxObject{ m_objects.at(index) };

            PhysicsObject physicsNewObject { *physxObject.id };

            setAllData(physicsNewObject, physxObject, true);

            // we insert into the list now after constructing the objects properly.
            m_objects.emplace_back(std::move(physxObject));
            
            // IMPORTANT
            PhysxObject& initialized_object = m_objects.back();

            all_objects.insert({ *initialized_object.id, m_objects.size() - 1 }); // add back the m_objects last element
            
            // we set this object's properties to be equal to the object we are copying from.
            // does order matter?
            // are they allocating memory at the back in physX
            //setAllOldData(physicsNewObject, initialized_object, index);

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
    }

    bool PhysxWorld::hasObject(phy_uuid::UUID id) const {

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

    RaycastHit PhysxWorld::sweep(PxTransform initialPose, PxVec3 direction, PxReal distance) {

        RaycastHit hit{};
        PxSweepBuffer hitBuffer;                 // [out] Sweep results
        PxGeometry sweepShape = PxBoxGeometry{}; // [in] swept shape
        //PxTransform initialPose = ...;         // [in] initial shape pose (at distance=0)
        //PxVec3 sweepDirection = ...;           // [in] normalized sweep direction

        PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

        //PxSweepHit::hadInitialOverlap()
        
        hit.intersect = scene->sweep(sweepShape, initialPose, direction, distance, hitBuffer, hitFlags);

        if (hit.intersect) {

            //hit.object_ID = *reinterpret_cast<phy_uuid::UUID*>(hitBuffer.block.actor->userData);
            //hit.position = hitBuffer.block.position;
            //hit.normal = hitBuffer.block.normal;
            //hit.distance = hitBuffer.block.distance;

            // Initialize variables to track the closest hit
            float closestDistance = FLT_MAX;
            const PxSweepHit* closestHit = nullptr;

            for (int i = 0; i < hitBuffer.nbTouches; ++i) {

                const PxSweepHit& hitTemp = hitBuffer.touches[i];

                // Check if this hit is closer than the previous closest hit
                if (hitTemp.distance < closestDistance) {
                    closestDistance = hitTemp.distance;
                    closestHit = &hitTemp;
                }
            }

            if (closestHit) {

                hit.object_ID = *reinterpret_cast<phy_uuid::UUID*>(closestHit->actor->userData);
                hit.position = closestHit->position;
                hit.distance = closestHit->distance;
                hit.normal = closestHit->normal;
            }
        }

        return hit;
    }

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

    RaycastHit PhysxWorld::raycast(PxVec3 origin, PxVec3 direction, PxReal distance, std::uint32_t filter) {

        RaycastHit hit{};
        PxRaycastBuffer hitBuffer;

        PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eUV | PxHitFlag::eMESH_ANY;

        // Define the filter data for the raycast
        PxQueryFilterData filterData = PxQueryFilterData();
        filterData.data.word0 = filter;

        hit.intersect = scene->raycast(origin, direction, distance, hitBuffer, hitFlags, filterData);
        
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

        scene->raycast(origin, direction, distance, buffer, hitFlags);

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

    std::vector<RaycastHit> PhysxWorld::raycastAll(PxVec3 origin, PxVec3 direction, PxReal distance, std::uint32_t filter) {

        const PxU32 bufferSize = 200;// 256;           // size of the buffer       
        PxRaycastHit hitBuffer[bufferSize];            // storage of the buffer results
        PxRaycastBuffer buffer(hitBuffer, bufferSize); // blocking and touching hits stored here

        std::vector<RaycastHit> hitAll{}; // store all the raycast hit

        PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eUV | PxHitFlag::eMESH_MULTIPLE;

        // Define the filter data for the raycast
        PxQueryFilterData filterData = PxQueryFilterData();
        filterData.data.word0 = filter;

        scene->raycast(origin, direction, distance, buffer, hitFlags, filterData);

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

    // NEW FUNCTIONS
    std::unordered_map<phy_uuid::UUID, PhysicsObject> PhysxWorld::retrieveCurrentObjects() const
    {
        std::unordered_map<phy_uuid::UUID, PhysicsObject> updatedObjects;

        for (auto&& physx_obj : m_objects)
        {
            PhysicsObject new_obj;

            retrieveOldData(new_obj, physx_obj);

            updatedObjects.insert({ new_obj.id, new_obj });
        }

        return updatedObjects;
    }

    void PhysxWorld::submitUpdatedObjects(std::vector<PhysicsObject> updatedObjects)
    {
        std::for_each(/*std::execution::unseq,*/ updatedObjects.begin(), updatedObjects.end(), [&](auto&& updatedObj)
        {
            // CHECK WHETHER OBJECT EXISTED
            if (!all_objects.contains(updatedObj.id))
                return;

            PhysxObject& underlying_obj = m_objects.at(all_objects.at(updatedObj.id));

            setAllData(updatedObj, underlying_obj, false);
        });
    }

    void PhysxWorld::submitPhysicsCommand(std::vector<PhysicsCommand> physicsCommand) {

        for (auto&& updatedCommandObj : physicsCommand)
        {
            // CHECK WHETHER OBJECT EXISTED
            if (!all_objects.contains(updatedCommandObj.Id))
                continue;

            PhysxObject& underlying_obj = m_objects[all_objects.at(updatedCommandObj.Id)];

            if (underlying_obj.rigid_type != rigid::rdynamic)
                continue;

            setForce(underlying_obj, updatedCommandObj);

            setTorque(underlying_obj, updatedCommandObj);
        }

    }

    void PhysxWorld::retrieveOldData(PhysicsObject& physics_Obj, const PhysxObject& init_Obj) const {

        physics_Obj.id = *init_Obj.id;

        // MATERIAL PROPERTIES
        //physics_Obj.matID = init_Obj.matID;
        //PxMaterial* currentMat = mat.at(init_Obj.matID);
        physics_Obj.material = Material{ init_Obj.m_material->getStaticFriction(),
                                         init_Obj.m_material->getDynamicFriction(),
                                         init_Obj.m_material->getRestitution() };

        // RIGIDBODY PROPERTIES
        physics_Obj.rigid_type = init_Obj.rigid_type;

        if (physics_Obj.rigid_type == rigid::rstatic) {

            // USE TEMPLATE HERE (STATIC/DYNAMIC) FOR POS & ORIENTATION
            retrievePosOri(physics_Obj, init_Obj.rb.rigidStatic);
        }
        else if (physics_Obj.rigid_type == rigid::rdynamic) {

            retrievePosOri(physics_Obj, init_Obj.rb.rigidDynamic);

            physics_Obj.mass = init_Obj.rb.rigidDynamic->getMass();
            physics_Obj.invmass = init_Obj.rb.rigidDynamic->getInvMass();
            physics_Obj.linearDamping = init_Obj.rb.rigidDynamic->getLinearDamping();
            physics_Obj.angularDamping = init_Obj.rb.rigidDynamic->getAngularDamping();
            physics_Obj.linearVel = init_Obj.rb.rigidDynamic->getLinearVelocity();
            physics_Obj.angularVel = init_Obj.rb.rigidDynamic->getAngularVelocity();
        }

        // SHAPE PROPERTIES
        physics_Obj.shape_type = init_Obj.shape_type;

        switch (physics_Obj.shape_type)
        {
        case shape::box:
            physics_Obj.box = init_Obj.m_shape->getGeometry().box();
            break;
        case shape::sphere:
            physics_Obj.sphere = init_Obj.m_shape->getGeometry().sphere();
            break;
        case shape::plane:
            physics_Obj.plane = init_Obj.m_shape->getGeometry().plane();
            break;
        case shape::capsule:
            physics_Obj.capsule = init_Obj.m_shape->getGeometry().capsule();
            break;
        case shape::convex:
            physics_Obj.convex = init_Obj.m_shape->getGeometry().convexMesh();
            break;
        default:
            break;
        }

        // AXIS PROPERTIES
        physics_Obj.lockPositionAxis = init_Obj.lockPositionAxis;
        physics_Obj.lockRotationAxis = init_Obj.lockRotationAxis;

        physics_Obj.is_trigger = init_Obj.is_trigger;
        physics_Obj.gravity_enabled = init_Obj.gravity_enabled;
        physics_Obj.is_kinematic = init_Obj.is_kinematic;
        physics_Obj.is_collider = init_Obj.is_collider;

        // FILTER
        physics_Obj.filterIn = init_Obj.filterIn;
        physics_Obj.filterOut = init_Obj.filterOut;

        // MESH
        physics_Obj.changeVertices = init_Obj.changeVertices;
        physics_Obj.meshScale = init_Obj.meshScale;
        physics_Obj.uploadVertices = init_Obj.uploadVertices;

        physics_Obj.meshVertices = init_Obj.meshVertices;
    }

    template<typename Type>
    void PhysxWorld::retrievePosOri(PhysicsObject& physics_Obj, Type data) const {

        physics_Obj.position = PxVec3{ data->getGlobalPose().p.x,
                                       data->getGlobalPose().p.y,
                                       data->getGlobalPose().p.z, };

        physics_Obj.orientation = data->getGlobalPose().q;
    }

    void PhysxWorld::setAllData(PhysicsObject& updatedPhysicsObj, PhysxObject& underlying_Obj, bool duplicate) {

        // MATERIAL PROPERTIES
        underlying_Obj.m_material->setStaticFriction(updatedPhysicsObj.material.staticFriction);
        underlying_Obj.m_material->setDynamicFriction(updatedPhysicsObj.material.dynamicFriction);
        underlying_Obj.m_material->setRestitution(updatedPhysicsObj.material.restitution);

        PxRigidActor* underlying_rigidbody = nullptr;
        PxTransform transform{ updatedPhysicsObj.position, updatedPhysicsObj.orientation };

        // set our rigidbody to the correct pointer
        if (underlying_Obj.rigid_type == rigid::rstatic)
        {
            underlying_rigidbody = underlying_Obj.rb.rigidStatic;
        }
        else // underlying_Obj.rigid_type == rigid::rdynamic
        {
            underlying_rigidbody = underlying_Obj.rb.rigidDynamic;
        }

        // CHECK IF CHANGING IS RIGID TYPE
        if (underlying_Obj.rigid_type != updatedPhysicsObj.rigid_type || duplicate)
        {
            // no need to remove if duplicate as there's no data
            if (!duplicate) {

                // what is our existing shape? need to handle removing them first.
                if (underlying_Obj.rigid_type == rigid::rstatic)
                {
                    // clear the current data
                    scene->removeActor(*underlying_rigidbody);
                    if (underlying_Obj.m_shape)
                        underlying_rigidbody->detachShape(*underlying_Obj.m_shape);
                    underlying_rigidbody = nullptr;
                }
                else // underlying_Obj.rigid_type == rigid::rdynamic
                {
                    scene->removeActor(*underlying_rigidbody);
                    if (underlying_Obj.m_shape)
                        underlying_rigidbody->detachShape(*underlying_Obj.m_shape);
                    underlying_rigidbody = nullptr;
                }
            }
            else
                transform = { 0,0,0 };

            // Update our rigid type to desired type
            underlying_Obj.rigid_type = updatedPhysicsObj.rigid_type;

            // we check with the newly assigned rigid type to determine
            if (underlying_Obj.rigid_type == rigid::rstatic)
            {
                // create new rigidbody
                underlying_Obj.rb.rigidStatic = mPhysics->createRigidStatic(transform);
                underlying_rigidbody = underlying_Obj.rb.rigidStatic;
            }
            else // underlying_Obj.rigid_type == rigid::rdynamic
            {
                underlying_Obj.rb.rigidDynamic = mPhysics->createRigidDynamic(transform);
                underlying_rigidbody = underlying_Obj.rb.rigidDynamic;
            }

            assert(underlying_rigidbody, "This should NOT be nullptr at all at this point!");

            underlying_rigidbody->userData = underlying_Obj.id.get();
            scene->addActor(*underlying_rigidbody);

            // REATTACH SHAPE INTO NEW RIGIDBODY
            if (underlying_Obj.m_shape)
            {
                underlying_rigidbody->attachShape(*underlying_Obj.m_shape);
                
                physx_system::setupFiltering(underlying_Obj.m_shape, underlying_Obj.filterIn, underlying_Obj.filterOut);
            }
        }

        // next we check for positional changes.
        if (underlying_rigidbody)
        {
            // set new position and orientation
            underlying_rigidbody->setGlobalPose(transform);
        }

        // RIGIDBODY PROPERTIES

        // we must set shape for all rigidbody
        setShape(updatedPhysicsObj, underlying_Obj, underlying_rigidbody, duplicate);

        // Additional work needs to be done for dynamic object
        if (underlying_Obj.rigid_type == rigid::rdynamic)
        {
            if (updatedPhysicsObj.mass >= 0)
                underlying_Obj.rb.rigidDynamic->setMass(updatedPhysicsObj.mass);

            if (updatedPhysicsObj.linearDamping >= 0)
                underlying_Obj.rb.rigidDynamic->setLinearDamping(updatedPhysicsObj.linearDamping);

            if (updatedPhysicsObj.angularDamping >= 0)
                underlying_Obj.rb.rigidDynamic->setAngularDamping(updatedPhysicsObj.angularDamping);

            underlying_Obj.rb.rigidDynamic->setAngularVelocity(updatedPhysicsObj.angularVel);

            underlying_Obj.rb.rigidDynamic->setLinearVelocity(updatedPhysicsObj.linearVel);

            // AXIS PROPERTIES
            underlying_Obj.lockPositionAxis = updatedPhysicsObj.lockPositionAxis;
            underlying_Obj.lockRotationAxis = updatedPhysicsObj.lockRotationAxis;

            LockingAxis lockPos = underlying_Obj.lockPositionAxis;
            underlying_Obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, lockPos.x_axis);
            underlying_Obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, lockPos.y_axis);
            underlying_Obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, lockPos.z_axis);

            LockingAxis lockRot = underlying_Obj.lockRotationAxis;
            underlying_Obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, lockRot.x_axis);
            underlying_Obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, lockRot.y_axis);
            underlying_Obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, lockRot.z_axis);

            // KINE | GRAVITY PROPERTIES
            underlying_Obj.is_kinematic = updatedPhysicsObj.is_kinematic;
            underlying_Obj.gravity_enabled = updatedPhysicsObj.gravity_enabled;

            underlying_Obj.rb.rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !underlying_Obj.gravity_enabled);
            underlying_Obj.rb.rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, underlying_Obj.is_kinematic);
        }

        // COLLIDER | TRIGGER PROPERTIES
        underlying_Obj.is_collider = updatedPhysicsObj.is_collider;
        underlying_Obj.is_trigger = updatedPhysicsObj.is_trigger;

        if (underlying_Obj.shape_type != shape::none)
        {
            if (underlying_Obj.is_trigger)
                underlying_Obj.m_shape->setFlags(PxShapeFlag::eVISUALIZATION | PxShapeFlag::eTRIGGER_SHAPE);

            if (!underlying_Obj.is_trigger && underlying_Obj.is_collider)
                underlying_Obj.m_shape->setFlags(PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSIMULATION_SHAPE);
        }

        // FILTER (not sure need or not)
        if (underlying_Obj.filterIn != updatedPhysicsObj.filterIn || underlying_Obj.filterOut != updatedPhysicsObj.filterOut)
            physx_system::setupFiltering(underlying_Obj.m_shape, updatedPhysicsObj.filterIn, updatedPhysicsObj.filterOut);
    }

    void PhysxWorld::setShape(PhysicsObject& updated_Obj, PhysxObject& underlying_Obj, PxRigidActor* underlying_rigidbody, bool duplicate)
    {
        // CHECK IF WE NEED TO CHANGE SHAPE 
        if (underlying_Obj.shape_type != updated_Obj.shape_type || duplicate)
        {
            // if we have an existing shape, we must remove it
            if (underlying_Obj.shape_type != shape::none && underlying_Obj.m_shape)
                underlying_rigidbody->detachShape(*underlying_Obj.m_shape);

            // SHAPE PROPERTIES
            underlying_Obj.shape_type = updated_Obj.shape_type;

            switch (underlying_Obj.shape_type)
            {
            case shape::box:
                underlying_Obj.m_shape = mPhysics->createShape(updated_Obj.box, *underlying_Obj.m_material, true);
                break;
            case shape::sphere:
                underlying_Obj.m_shape = mPhysics->createShape(updated_Obj.sphere, *underlying_Obj.m_material, true);
                break;
            case shape::plane:
                underlying_Obj.m_shape = mPhysics->createShape(updated_Obj.plane, *underlying_Obj.m_material, true);
                break;
            case shape::capsule:
                underlying_Obj.m_shape = mPhysics->createShape(updated_Obj.capsule, *underlying_Obj.m_material, true);
                // Change the capsule to extend along the Y-axis
                underlying_Obj.m_shape->setLocalPose(PxTransform{ PxQuat(PxHalfPi, PxVec3(0, 0, 1)) });
                break;
            case shape::convex:
            {
                PxConvexMesh* defaultMesh = createConvexMesh({ PxVec3{ 0,0,0 }, PxVec3{0,0,0}, PxVec3{0,0,0} });
                underlying_Obj.m_shape = mPhysics->createShape(PxConvexMeshGeometry(defaultMesh, PxMeshScale(updated_Obj.meshScale)), *underlying_Obj.m_material, true);
                
                if (!updated_Obj.uploadVertices.empty()) {
                    PxConvexMesh* m = createConvexMesh(updated_Obj.uploadVertices);
                    underlying_Obj.m_shape = mPhysics->createShape(PxConvexMeshGeometry(m, PxMeshScale(updated_Obj.meshScale)), *underlying_Obj.m_material, true);
                    updated_Obj.uploadVertices.clear();

                    underlying_Obj.changeVertices = true;
                }

                PxConvexMesh* convexMesh = underlying_Obj.m_shape->getGeometry().convexMesh().convexMesh;
                const PxVec3* convexVerts = convexMesh->getVertices();
                PxU32 nbVerts = convexMesh->getNbVertices();

                underlying_Obj.meshVertices.clear(); // remove old data first

                for (PxU32 i = 0; i < nbVerts; i++) {

                    PxVec3 mesh = { convexVerts[i].x, convexVerts[i].y, convexVerts[i].z };

                    underlying_Obj.meshVertices.emplace_back(mesh);
                }

                underlying_Obj.meshScale = updated_Obj.meshScale;
                underlying_Obj.m_shape->setGeometry(PxConvexMeshGeometry(createConvexMesh(underlying_Obj.meshVertices), PxMeshScale(underlying_Obj.meshScale)));
                break;
            }
            case shape::none:
            default:
                return; // NOTE we return here because code below requires a shape!
            }

            // ATTACH THE NEW SHAPE TO THE OBJECT
            underlying_rigidbody->attachShape(*underlying_Obj.m_shape);

            physx_system::setupFiltering(underlying_Obj.m_shape, underlying_Obj.filterIn, underlying_Obj.filterOut);
        }
        // Otherwise we just update our existing values.
        else
        {
            // UPDATING THE GEOMETRY DIMENSIONS (SHAPE NOW CONFIRM CORRECT - UPDATED)
            switch (underlying_Obj.shape_type)
            {
            case shape::box:
                underlying_Obj.m_shape->setGeometry(updated_Obj.box);
                break;
            case shape::sphere:
                underlying_Obj.m_shape->setGeometry(updated_Obj.sphere);
                break;
            case shape::plane:
                underlying_Obj.m_shape->setGeometry(updated_Obj.plane);
                break;
            case shape::capsule:
                underlying_Obj.m_shape->setGeometry(updated_Obj.capsule);
                // Change the capsule to extend along the Y-axis
                underlying_Obj.m_shape->setLocalPose(PxTransform{ PxQuat(PxHalfPi, PxVec3(0, 0, 1)) });
                break;
            case shape::convex:
            {
                underlying_Obj.meshScale = updated_Obj.meshScale;

                if (!updated_Obj.uploadVertices.empty()) {
                    PxConvexMesh* m = createConvexMesh(updated_Obj.uploadVertices);
                    underlying_Obj.m_shape = mPhysics->createShape(PxConvexMeshGeometry(m, PxMeshScale(updated_Obj.meshScale)), *underlying_Obj.m_material, true);
                    updated_Obj.uploadVertices.clear();

                    underlying_Obj.changeVertices = true;

                    PxConvexMesh* convexMesh = underlying_Obj.m_shape->getGeometry().convexMesh().convexMesh;
                    const PxVec3* convexVerts = convexMesh->getVertices();
                    PxU32 nbVerts = convexMesh->getNbVertices();

                    underlying_Obj.meshVertices.clear(); // remove old data first

                    for (PxU32 i = 0; i < nbVerts; i++) {

                        PxVec3 mesh = { convexVerts[i].x, convexVerts[i].y, convexVerts[i].z };

                        underlying_Obj.meshVertices.emplace_back(mesh);
                    }
                }
                else { // changes to scale
                    underlying_Obj.m_shape->setGeometry(PxConvexMeshGeometry(createConvexMesh(underlying_Obj.meshVertices), PxMeshScale(underlying_Obj.meshScale)));
                }
                break;
            }
            case shape::none:
            default:
                return; // NOTE we return here because code below requires a shape!
            }
        }

        //underlying_obj.m_shape->setContactOffset(1);
    }

    void PhysxWorld::setForce(PhysxObject& underlying_Obj, PhysicsCommand& command_Obj) {

        if (command_Obj.AddForce) {

            switch (command_Obj.Type) {
            case force::force:
                underlying_Obj.rb.rigidDynamic->addForce(command_Obj.Force, PxForceMode::eFORCE);
                break;

            case force::impulse:
                underlying_Obj.rb.rigidDynamic->addForce(command_Obj.Force, PxForceMode::eIMPULSE);
                break;

            case force::velocityChanged:
                underlying_Obj.rb.rigidDynamic->addForce(command_Obj.Force, PxForceMode::eVELOCITY_CHANGE);
                break;

            case force::acceleration:
                underlying_Obj.rb.rigidDynamic->addForce(command_Obj.Force, PxForceMode::eACCELERATION);
                break;

            default:
                break;
            }
        }
    }

    void PhysxWorld::setTorque(PhysxObject& underlying_Obj, PhysicsCommand& command_Obj) {

        if (command_Obj.AddTorque) {

            switch (command_Obj.Type) {
            case force::force:
                underlying_Obj.rb.rigidDynamic->addTorque(command_Obj.Force, PxForceMode::eFORCE);
                break;

            case force::impulse:
                underlying_Obj.rb.rigidDynamic->addTorque(command_Obj.Force, PxForceMode::eIMPULSE);
                break;

            case force::velocityChanged:
                underlying_Obj.rb.rigidDynamic->addTorque(command_Obj.Force, PxForceMode::eVELOCITY_CHANGE);
                break;

            case force::acceleration:
                underlying_Obj.rb.rigidDynamic->addTorque(command_Obj.Force, PxForceMode::eACCELERATION);
                break;

            default:
                break;

            }
        }
    }

/*-----------------------------------------------------------------------------*/
/*                               PhysicsObject                                 */
/*-----------------------------------------------------------------------------*/

    //void PhysicsObject::setBoxProperty(float halfextent_width, float halfextent_height, float halfextent_depth) {

    //    if (world->all_objects.contains(id)) {

    //        PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

    //        if (underlying_obj->shape_type == shape::box)
    //            underlying_obj->m_shape->setGeometry(PxBoxGeometry(halfextent_width, halfextent_height, halfextent_depth));
    //        //underlying_obj->m_shape->getGeometry().box().halfExtents = PxVec3{ halfextent_width , halfextent_height, halfextent_depth };
    //    }
    //}

    //void PhysicsObject::setSphereProperty(float radius) {

    //    if (world->all_objects.contains(id)) {

    //        PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

    //        if (underlying_obj->shape_type == shape::sphere)
    //            underlying_obj->m_shape->setGeometry(PxSphereGeometry(radius));
    //        //underlying_obj->m_shape->getGeometry().sphere().radius = radius;
    //    }
    //}

    //void PhysicsObject::setCapsuleProperty(float radius, float halfHeight) {

    //    if (world->all_objects.contains(id)) {

    //        PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];
    //        if (underlying_obj->shape_type == shape::capsule) {
    //            underlying_obj->m_shape->setGeometry(PxCapsuleGeometry(radius, halfHeight));

    //            if (underlying_obj->rigid_type == rigid::rdynamic) {
    //                PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
    //                underlying_obj->m_shape->setLocalPose(relativePose);
    //            }

    //            //underlying_obj->m_shape->getGeometry().capsule().radius = radius;
    //            //underlying_obj->m_shape->getGeometry().capsule().halfHeight = halfHeight;
    //        }
    //    }
    //}

    //void PhysicsObject::setConvexProperty(std::vector<PxVec3> vert, PxVec3 scale) { //  PxConvexMesh* mesh

    //    if (world->all_objects.contains(id)) {

    //        PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

    //        if (underlying_obj->shape_type == shape::convex) {

    //            underlying_obj->m_shape->setGeometry(PxConvexMeshGeometry(createConvexMesh(vert), PxMeshScale(scale)));

    //            //underlying_obj->m_shape->getGeometry().convexMesh().convexMesh->getVertices()
    //        }
    //    }
    //}

    //void PhysicsObject::storeMeshVertices(std::vector<PxVec3> vert) {

    //    if (world->all_objects.contains(id)) {

    //        PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

    //        if (underlying_obj->shape_type == shape::convex)
    //        {
    //            underlying_obj->meshVertices = vert;

    //            underlying_obj->m_shape = physx_system::getPhysics()->createShape(PxConvexMeshGeometry(createConvexMesh(underlying_obj->meshVertices)), *world->mat.at(underlying_obj->matID), true);
    //        
    //            if(underlying_obj->rigid_type == rigid::rdynamic) 
    //                underlying_obj->rb.rigidDynamic->attachShape(*underlying_obj->m_shape);

    //            if (underlying_obj->rigid_type == rigid::rstatic)
    //                underlying_obj->rb.rigidStatic->attachShape(*underlying_obj->m_shape);

    //            //getAllMeshVertices();
    //        }
    //    }
    //}

    //std::vector<PxVec3> PhysicsObject::getAllMeshVertices() {

    //    if (world->all_objects.contains(id)) {

    //        PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

    //        if (underlying_obj->shape_type == shape::convex)
    //        {
    //            // Retrieve the vertices
    //            PxConvexMesh* convexMesh = underlying_obj->m_shape->getGeometry().convexMesh().convexMesh;
    //            const PxVec3* convexVerts = convexMesh->getVertices();
    //            PxU32 nbVerts = convexMesh->getNbVertices();

    //            world->m_meshVertices.clear(); // remove old data first

    //            for (PxU32 i = 0; i < nbVerts; i++) {

    //                PxVec3 mesh = { convexVerts[i].x, convexVerts[i].y, convexVerts[i].z };

    //                world->m_meshVertices.emplace_back(mesh);
    //            }

    //            //int size = world->m_meshVertices.size();
    //            //std::cout << "SIZE: " << size << std::endl;
    //        }
    //    }

    //    return world->m_meshVertices;
    //}

    //void PhysicsObject::setFiltering(std::uint32_t currentGroup, std::uint32_t maskGroup) {

    //    if (world->all_objects.contains(id)) {

    //        PhysxObject* underlying_obj = &world->m_objects[world->all_objects.at(id)];

    //        if (underlying_obj->m_shape) {

    //            underlying_obj->filterIn = currentGroup;

    //            underlying_obj->filterOut = maskGroup;

    //            physx_system::setupFiltering(underlying_obj->m_shape, currentGroup, maskGroup);
    //        }
    //    }
    //}

    PxConvexMesh* PhysxWorld::createConvexMesh(std::vector<PxVec3> vert) {

        // vertices
        //static const PxVec3 convexVerts[] = { PxVec3(0,1,0),PxVec3(1,0,0),PxVec3(-1,0,0),PxVec3(0,0,1),PxVec3(0,0,-1) };

        // Construct the convex data
        PxConvexMeshDesc convexDesc;
        convexDesc.points.count = static_cast<physx::PxU32>(vert.size()); //5;
        convexDesc.points.stride = sizeof(PxVec3);
        convexDesc.points.data = static_cast<void*>(vert.data()); //convexVerts;
        convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

        //PxConvexMeshCookingType::eQUICKHULL;

        // Construct the mesh with the cooking library
        PxDefaultMemoryOutputStream buffer;

#pragma warning(disable : 26812)
        PxConvexMeshCookingResult::Enum result;
#pragma warning(default : 26812)

        if (convexDesc.points.count == 0)
            return NULL;
        if (!mCooking->cookConvexMesh(convexDesc, buffer, &result))
            return NULL;

        // Fastest method (Vertex Points, Indices and Polygons are Provided)
        /*
        PxConvexMeshDesc convexDesc;
        convexDesc.points.count = 12;
        convexDesc.points.stride = sizeof(PxVec3);
        convexDesc.points.data = convexVerts;
        
        convexDescPolygons.polygons.count = 20;
        convexDescPolygons.polygons.stride = sizeof(PxHullPolygon);
        convexDescPolygons.polygons.data = hullPolygons;
        convexDesc.flags = 0;

        PxDefaultMemoryOutputStream buf;
        if (!mCooking.cookConvexMesh(convexDesc, buf))
            return NULL;
        */

        PxDefaultMemoryInputData input(buffer.getData(), buffer.getSize());
        PxConvexMesh* convexMesh = mPhysics->createConvexMesh(input);

        // Create shape which instances the mesh
        //PxShape* convexShape = PxRigidActorExt::createExclusiveShape();
        
        return convexMesh;
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

            if (current.contactCount > 0) {

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

    }
    void EventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count) {
        //printf("CALLBACK: onTrigger -- ");
        //printf("PAIRS: %d\n", count);

        while (count--) {

            //bool stayTrigger = false;
            trigger state = trigger::none;
            const PxTriggerPair& current = *pairs++;

            if (current.triggerActor != nullptr && current.otherActor != nullptr) {

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
    }
    void EventCallBack::onAdvance(const PxRigidBody* const* /*bodyBuffer*/, const PxTransform* /*poseBuffer*/, const PxU32 /*count*/) {
        //printf("CALLBACK: onAdvance\n");
    }

}