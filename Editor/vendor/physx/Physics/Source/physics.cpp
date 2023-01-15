#include "physics.h"

using namespace physx;

static constexpr bool use_debugger = false;

static phy::EventCallBack mEventCallback;

phy::PVD myPVD;
PxDefaultAllocator      mDefaultAllocatorCallback;
PxDefaultErrorCallback  mDefaultErrorCallback;
PxDefaultCpuDispatcher* mDispatcher;
PxTolerancesScale       mToleranceScale;
PxFoundation* mFoundation;
PxPhysics* mPhysics;

namespace phy
{
    namespace physx_system 
    {
        PxFoundation* getFoundation() 
        {
            return mFoundation;
        }

        PxPhysics* getPhysics() 
        {
            return mPhysics;
        }

        PxFoundation* createFoundation() 
        {
            mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);

            if (!mFoundation)
                throw("PxCreateFoundation failed!");

            return mFoundation;
        }

        PxPhysics* createPhysics() 
        {
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
        bool isTrigger(const PxFilterData& data) 
        {
            if (data.word0 != 0xffffffff || data.word1 != 0xffffffff ||
                data.word2 != 0xffffffff || data.word3 != 0xffffffff)
                return false;

            return true;
        }

        bool isTriggerShape(PxShape* shape) 
        {
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

        void setCurrentWorld(PhysicsWorld* world) 
        {
            // retrieving the current world for me to access (get/set) neccessary data
            currentWorld = world;
        }

        PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
            PxFilterObjectAttributes attributes1, PxFilterData filterData1,
            PxPairFlags& pairFlags, const void* /*constantBlock*/,
            PxU32 /*constantBlockSize*/) 
        {
            //let triggers through
            if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) 
            {
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

        void setupFiltering(PxShape* shape) 
        {
            PxFilterData filterData;
            filterData.word0 = 1; //filterGroup; // word0 = own ID
            filterData.word1 = 1; //filterMask;  // word1 = ID mask to filter pairs that trigger a contact callback;

            shape->setSimulationFilterData(filterData);
        }

        void init() 
        {
            createFoundation();

            if constexpr (use_debugger) 
            {
                printf("DEBUGGER ON\n");
                myPVD.createPvd(mFoundation, "172.28.68.41");
            }
            else 
            {
                printf("DEBUGGER OFF\n");
            }

            createPhysics();
        }

        void shutdown() 
        {
            mPhysics->release();

            // pvd release here
            if constexpr (use_debugger) 
            {
                myPVD.pvd__()->release();

                myPVD.getTransport()->release();
            }

            mFoundation->release();
        }

    }

    /*-----------------------------------------------------------------------------*/
    /*                               PhysxWorld                                    */
    /*-----------------------------------------------------------------------------*/
    PhysicsWorld::PhysicsWorld(PxVec3 grav)
    {
        // Setup scene description
        PxSceneDesc sceneDesc{ mPhysics->getTolerancesScale() };
        sceneDesc.gravity = grav; // default is PxVec3(0.0f, -9.81f, 0.0f);

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

        // just take note we have 5000 objects reserved to prevent resizing problems
        m_physx_objects.reserve(5000);
    }

    PhysicsWorld::~PhysicsWorld()
    {
        for (auto const& i : mat) 
        {
            i.second->release();
        }

        scene->release();

        if (mDispatcher)
        {
            mDispatcher->release();
            mDispatcher = nullptr;
        }
    }

    void PhysicsWorld::updateWorld(float dt) 
    {
        scene->simulate(dt); // 1.f / 60.f
        scene->fetchResults(true);
    }

    PxVec3 PhysicsWorld::getWorldGravity() const 
    {
        return scene->getGravity();
    }

    void PhysicsWorld::setWorldGravity(PxVec3 gra) 
    {
        scene->setGravity(gra);
    }

    PhysicsObject PhysicsWorld::createInstance() 
    {
        PhysxObject obj;
        obj.id = std::make_unique<phy_uuid::UUID>();
        // This is important!
        phy_uuid::UUID generated_uuid = *obj.id;
        // store the object
        m_physx_objects.emplace_back(std::move(obj));
        m_objects_lookup.insert({ generated_uuid, m_physx_objects.size() - 1 }); // add back the m_objects last element

        return PhysicsObject{ generated_uuid }; // a copy
    }

    void PhysicsWorld::removeInstance(PhysicsObject obj) 
    {
        if (m_objects_lookup.contains(obj.id) == false)
            return;

        std::size_t current_index = m_objects_lookup.at(obj.id);

        PhysxObject underlying_obj = m_physx_objects[current_index];

        if (underlying_obj.rigid_type == rigid::rstatic)
            underlying_obj.rb.rigidStatic->release();
        else if (underlying_obj.rigid_type == rigid::rdynamic)
            underlying_obj.rb.rigidDynamic->release();

        m_objects_lookup.erase(obj.id);
        for (auto i = m_objects_lookup.begin(); i != m_objects_lookup.end(); i++) 
        {
            if (i->second > current_index) 
            {
                std::size_t current = i->second;
                i->second = current - 1;
            }
        }

        auto begin = std::find_if(m_physx_objects.begin(), m_physx_objects.end(), [&](auto&& elem) { return *elem.id == obj.id; });
        m_physx_objects.erase(begin);
    }

    PhysicsObject PhysicsWorld::duplicateObject(phy_uuid::UUID id) 
    {
        if (m_objects_lookup.contains(id)) 
        {
            size_t index = m_objects_lookup.at(id);

            PhysxObject physxObject{ m_physx_objects.at(index) };

            PhysicsObject physicsNewObject{ *physxObject.id };

            // we insert into the list now after constructing the objects properly.
            m_physx_objects.emplace_back(std::move(physxObject));

            // IMPORTANT
            PhysxObject& initialized_object = m_physx_objects.back();

            m_objects_lookup.insert({ *initialized_object.id, m_physx_objects.size() - 1 }); // add back the m_objects last element

            // we set this object's properties to be equal to the object we are copying from.
            // does order matter?
            // are they allocating memory at the back in physX
            
            //setAllOldData(physicsNewObject, initialized_object, index);

            retrieveOldData(physicsNewObject, initialized_object);
            // do a bit more things here to set it up correctly

            return physicsNewObject; // a copy
        }

        return PhysicsObject{}; // return empty
    }

    std::vector<PhysicsObject> PhysicsWorld::retrieveCurrentObjects() const
    {
        std::vector<PhysicsObject> updatedObjects;

        for (auto&& physx_obj : m_physx_objects)
        {
            PhysicsObject new_obj;

            retrieveOldData(new_obj, physx_obj);

            updatedObjects.emplace_back(new_obj);
        }

        return updatedObjects;
    }

    void PhysicsWorld::retrieveOldData(PhysicsObject& physics_Obj, const PhysxObject& init_Obj) const {

        physics_Obj.id = *init_Obj.id;

        // MATERIAL PROPERTIES
        physics_Obj.matID = init_Obj.matID;
        PxMaterial* currentMat = mat.at(init_Obj.matID);
        physics_Obj.material = Material{ currentMat->getStaticFriction(),
                                         currentMat->getDynamicFriction(),
                                         currentMat->getRestitution() };

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
        default:
            break;
        }

        // NOT SURE IF EMPTY CAN RETRIEVE OR NOT
        //physics_Obj.box = init_Obj.m_shape->getGeometry().box();
        //physics_Obj.sphere = init_Obj.m_shape->getGeometry().sphere();
        //physics_Obj.plane = init_Obj.m_shape->getGeometry().plane();
        //physics_Obj.capsule = init_Obj.m_shape->getGeometry().capsule();

        // AXIS PROPERTIES
        physics_Obj.lockPositionAxis = init_Obj.lockPositionAxis;
        physics_Obj.lockRotationAxis = init_Obj.lockRotationAxis;

        physics_Obj.is_trigger = init_Obj.is_trigger;
        physics_Obj.gravity_enabled = init_Obj.gravity_enabled;
        physics_Obj.is_kinematic = init_Obj.is_kinematic;
        physics_Obj.is_collider = init_Obj.is_collider;
    }

    template<typename Type>
    void PhysicsWorld::retrievePosOri(PhysicsObject& physics_Obj, Type data) const {

        physics_Obj.position = PxVec3{ data->getGlobalPose().p.x,
                                       data->getGlobalPose().p.y,
                                       data->getGlobalPose().p.z, };

        physics_Obj.orientation = data->getGlobalPose().q;
    }

    void PhysicsWorld::submitUpdatedObjects(std::vector<PhysicsObject> updatedObjects)
    {
        for (auto&& updatedObj : updatedObjects)
        {
            // CHECK WHETHER OBJECT EXISTED
            if (!m_objects_lookup.contains(updatedObj.id))
                continue;
            
            PhysxObject& underlying_obj = m_physx_objects[m_objects_lookup.at(updatedObj.id)];

            // MATERIAL PROPERTIES
            underlying_obj.matID = updatedObj.matID;
            
            // CHECK WHETHER IS AN EXISTING MATERIAL
            if (mat.contains(underlying_obj.matID)) {

                PxMaterial* temp_mat = mat.at(underlying_obj.matID);

                temp_mat->setStaticFriction(updatedObj.material.staticFriction);
                temp_mat->setDynamicFriction(updatedObj.material.dynamicFriction);
                temp_mat->setRestitution(updatedObj.material.restitution);
            }
            else {

                // CREATE NEW MATERIAL
                PxMaterial* newMat = mPhysics->createMaterial(updatedObj.material.staticFriction,
                                                                updatedObj.material.dynamicFriction,
                                                                updatedObj.material.restitution);

                phy_uuid::UUID UUID = phy_uuid::UUID{};

                mat.emplace(UUID, newMat);

                underlying_obj.matID = UUID; // set material id for that object
            }

            // CHECK IF THIS OBJ HAVE RSTATIC OR RDYAMIC INIT OR NOT
            PxRigidStatic* rstat = underlying_obj.rb.rigidStatic;
            PxRigidDynamic* rdyna = underlying_obj.rb.rigidDynamic;

            if (rstat) {
                scene->removeActor(*rstat);
                underlying_obj.rb.rigidStatic = nullptr; // clear the current data
            }
            else if (rdyna) {
                scene->removeActor(*rdyna);
                underlying_obj.rb.rigidDynamic = nullptr;
            }

            // SET NEW RIDID TYPE
            underlying_obj.rigid_type = updatedObj.rigid_type;

            // RIGIDBODY PROPERTIES
            if (underlying_obj.rigid_type == rigid::rstatic) {

                setRigidShape(updatedObj, underlying_obj, underlying_obj.rb.rigidStatic);

            }
            else if (underlying_obj.rigid_type == rigid::rdynamic) {

                setRigidShape(updatedObj, underlying_obj, underlying_obj.rb.rigidDynamic);

                underlying_obj.rb.rigidDynamic->setMass(updatedObj.mass);
                underlying_obj.rb.rigidDynamic->setLinearDamping(updatedObj.linearDamping);
                underlying_obj.rb.rigidDynamic->setAngularDamping(updatedObj.angularDamping);
                underlying_obj.rb.rigidDynamic->setLinearVelocity(updatedObj.linearVel);
                underlying_obj.rb.rigidDynamic->setAngularVelocity(updatedObj.angularVel);

                // AXIS PROPERTIES
                underlying_obj.lockPositionAxis = updatedObj.lockPositionAxis;
                underlying_obj.lockRotationAxis = updatedObj.lockRotationAxis;

                LockingAxis lockPos = underlying_obj.lockPositionAxis;
                underlying_obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, lockPos.x_axis);
                underlying_obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, lockPos.y_axis);
                underlying_obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, lockPos.z_axis);

                LockingAxis lockRot = underlying_obj.lockRotationAxis;
                underlying_obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, lockRot.x_axis);
                underlying_obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, lockRot.y_axis);
                underlying_obj.rb.rigidDynamic->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, lockRot.z_axis);
                
                // KINE | GRAVITY PROPERTIES
                underlying_obj.is_kinematic = updatedObj.is_kinematic;
                underlying_obj.gravity_enabled = updatedObj.gravity_enabled;

                underlying_obj.rb.rigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !underlying_obj.gravity_enabled);
                underlying_obj.rb.rigidDynamic->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, underlying_obj.is_kinematic);
            }
                    
            // COLLIDER PROPERTIES
            underlying_obj.is_collider = updatedObj.is_collider;
            underlying_obj.m_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, underlying_obj.is_collider);

            // TRIGGER PROPERTIES
            underlying_obj.is_trigger = updatedObj.is_trigger;

            if (underlying_obj.is_trigger)
                underlying_obj.m_shape->setFlags(PxShapeFlag::eVISUALIZATION | PxShapeFlag::eTRIGGER_SHAPE);
            else
                underlying_obj.m_shape->setFlags(PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSIMULATION_SHAPE);
            
        }
    }

    template<typename Type>
    void PhysicsWorld::setRigidShape(PhysicsObject& updated_Obj, PhysxObject& underlying_obj, Type data) {

        // CHECK IF THIS OBJ HAVE SHAPE DATA INIT OR NOT
        if (underlying_obj.m_shape)
            underlying_obj.rb.rigidStatic->detachShape(*underlying_obj.m_shape);

        // SHAPE PROPERTIES
        underlying_obj.shape_type = updated_Obj.shape_type;
        
        PxMaterial* material = mat.at(underlying_obj.matID);

        if (underlying_obj.shape_type == shape::box)
            underlying_obj.m_shape = mPhysics->createShape(updated_Obj.box, *material, true);

        else if (underlying_obj.shape_type == shape::sphere)
            underlying_obj.m_shape = mPhysics->createShape(updated_Obj.sphere, *material, true);

        else if (underlying_obj.shape_type == shape::plane)
            underlying_obj.m_shape = mPhysics->createShape(updated_Obj.plane, *material, true);

        else if (underlying_obj.shape_type == shape::capsule)
            underlying_obj.m_shape = mPhysics->createShape(updated_Obj.capsule, *material, true);

        //underlying_obj.m_shape->setContactOffset(1);

        // RIGID ACTOR
        underlying_obj.rb.rigidStatic = mPhysics->createRigidStatic(PxTransform{ updated_Obj.position, updated_Obj.orientation });
        underlying_obj.rb.rigidStatic->userData = underlying_obj.id.get();
        scene->addActor(*underlying_obj.rb.rigidStatic);

        // ATTACH THE NEW SHAPE TO THE OBJECT
        underlying_obj.rb.rigidStatic->attachShape(*underlying_obj.m_shape);

        physx_system::setupFiltering(underlying_obj.m_shape);
    }

    void PhysicsWorld::updateTriggerState(phy_uuid::UUID id) 
    {
        std::queue<TriggerManifold> temp = m_triggerCollisionPairs;

        while (!m_triggerCollisionPairs.empty()) 
        {
            TriggerManifold val = m_triggerCollisionPairs.front();

            if (val.triggerID == id || val.otherID == id)
                temp.emplace(val.triggerID, val.otherID, trigger::onTriggerExit, true);

            m_triggerCollisionPairs.pop();
        }

        m_triggerCollisionPairs = temp;
    }

    bool PhysicsWorld::hasObject(phy_uuid::UUID id)  const
    {
        return m_objects_lookup.contains(id);
    }

    std::map<phy_uuid::UUID, std::size_t> PhysicsWorld::getLookupTable() const
    {
        return m_objects_lookup;
    }

    std::queue<TriggerManifold> PhysicsWorld::getTriggerData() const
    {
        //updateTriggerState();
        return m_triggerCollisionPairs;
    }

    void PhysicsWorld::clearTriggerData() 
    {
        while (!m_triggerCollisionPairs.empty())
            m_triggerCollisionPairs.pop();

        //std::queue<TriggerManifold> empty;
        //std::swap(m_triggerCollisionPairs, empty);
    }

    std::queue<ContactManifold> PhysicsWorld::getCollisionData() const
    {
        return m_collisionPairs;
    }

    void PhysicsWorld::clearCollisionData() 
    {
        while (!m_collisionPairs.empty())
            m_collisionPairs.pop();
    }

    RaycastHit PhysicsWorld::raycast(PxVec3 origin, PxVec3 direction, PxReal distance) 
    {
        RaycastHit hit{};
        PxRaycastBuffer hitBuffer;

        PxHitFlags hitFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL | PxHitFlag::eUV | PxHitFlag::eMESH_ANY;

        hit.intersect = scene->raycast(origin, direction, distance, hitBuffer, hitFlags);

        // HAVE INTERSECTION
        if (hit.intersect) 
        {
            hit.object_ID = *reinterpret_cast<phy_uuid::UUID*>(hitBuffer.block.actor->userData);

            hit.position = hitBuffer.block.position;
            hit.normal = hitBuffer.block.normal;
            hit.distance = hitBuffer.block.distance;
        }

        return hit;
    }

    std::vector<RaycastHit> PhysicsWorld::raycastAll(PxVec3 origin, PxVec3 direction, PxReal distance) 
    {
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
    /*                           PHYSX OBJECT                                      */
    /*-----------------------------------------------------------------------------*/
    PhysxObject::PhysxObject(const PhysxObject& other) : matID(other.matID),
        shape_type(other.shape_type),
        rigid_type(other.rigid_type),
        lockPositionAxis(other.lockPositionAxis),
        lockRotationAxis(other.lockRotationAxis),
        is_trigger(other.is_trigger),
        gravity_enabled(other.gravity_enabled),
        is_kinematic(other.is_kinematic),
        is_collider(other.is_collider),
        m_shape{ nullptr },
        rb{},
        // Create new UUID when we attempt to make a copy
        id{ std::make_unique<phy_uuid::UUID>() }   {}

    PhysxObject& PhysxObject::operator=(const PhysxObject& object) 
    {
        PhysxObject copy{ object };
        std::swap(*this, copy);

        return *this;
    }


    /*-----------------------------------------------------------------------------*/
    /*                           EVENT CALLBACK                                    */
    /*-----------------------------------------------------------------------------*/

    void EventCallBack::onConstraintBreak(PxConstraintInfo* /*constraints*/, PxU32 /*count*/) 
    {
        //printf("CALLBACK: onConstraintBreak\n");
    }

    void EventCallBack::onWake(PxActor** /*actors*/, PxU32 /*count*/) 
    {
        //printf("CALLBACK: onWake\n");
    }

    void EventCallBack::onSleep(PxActor** /*actors*/, PxU32 /*count*/) 
    {
        //printf("CALLBACK: onSleep\n");
    }

    void EventCallBack::onContact(const PxContactPairHeader& /*pairHeader*/, const PxContactPair* pairs, PxU32 count) 
    {
        //printf("CALLBACK: onContact -- ");
        //printf("PAIRS: %d\n", count);

        while (count--) 
        {
            collision state = collision::none;
            const PxContactPair& current = *pairs++;

            if (current.contactCount > 0) 
            {
                auto shape1_id = *reinterpret_cast<phy_uuid::UUID*>(current.shapes[0]->getActor()->userData);
                auto shape2_id = *reinterpret_cast<phy_uuid::UUID*>(current.shapes[1]->getActor()->userData);
                //printf("shape1 actor %llu, shape2 actor %llu \n", shape1_id, shape2_id);

                //current.shapes[0]->getGeometryType() // PxGeometryType::eBOX;

                const PxU32 bufferSize = 64;
                PxContactPairPoint contacts[bufferSize];
                PxU32 nbContacts = current.extractContacts(contacts, bufferSize);

                std::vector<ContactPoint> tempCP = {};
                PxU8 contactCount = current.contactCount;

                for (PxU32 j = 0; j < nbContacts; j++) 
                {
                    PxVec3 normal = contacts[j].normal;
                    PxVec3 point = contacts[j].position;
                    PxVec3 impulse = contacts[j].impulse;

                    ContactPoint cp = { normal, point, impulse };
                    tempCP.emplace_back(cp);
                    //printf("%d - NORMAL: %f - X, %f - Y, %f - Z\n", j, normal.x, normal.y, normal.z);
                    //printf("%d - POINT: %f - X, %f - Y, %f - Z\n", j, point.x, point.y, point.z);
                    //printf("%d - IMPUSLE: %f - X, %f - Y, %f - Z\n", j, impulse.x, impulse.y, impulse.z);
                }

                // OnCollisionEnter
                if (current.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) 
                { 
                    state = collision::onCollisionEnter;
                    //printf("Shape is ENTERING CONTACT volume\n");
                }

                // OnCollisionStay
                if (current.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) 
                { 
                    state = collision::onCollisionStay;
                    //printf("Shape is STAYING CONTACT volume\n");
                }
                
                // OnCollisionExit
                if (current.events & PxPairFlag::eNOTIFY_TOUCH_LOST) 
                {
                    state = collision::onCollisionExit;
                    //printf("Shape is LEAVING CONTACT volume\n");
                }

                // Store all the ID of the actors that collided
                std::queue<ContactManifold> collision_data = physx_system::currentWorld->getCollisionData();

                // Add new ContactManifold data into the queue
                collision_data.emplace(ContactManifold{ shape1_id, shape2_id, state, tempCP, contactCount });

                //if (physx_system::isTriggerShape(current.shapes[0]) && physx_system::isTriggerShape(current.shapes[1]))
                //    printf("Trigger-trigger overlap detected\n");
            }
        }

    }

    void EventCallBack::onTrigger(PxTriggerPair* pairs, PxU32 count) 
    {
        //printf("CALLBACK: onTrigger -- ");
        //printf("PAIRS: %d\n", count);

        while (count--) 
        {
            //bool stayTrigger = false;
            trigger state = trigger::none;
            const PxTriggerPair& current = *pairs++;

            if (current.triggerActor != nullptr && current.otherActor != nullptr) 
            {
                auto trigger_id = *reinterpret_cast<phy_uuid::UUID*>(current.triggerActor->userData);
                auto other_id = *reinterpret_cast<phy_uuid::UUID*>(current.otherActor->userData);
                //printf("trigger actor %llu, other actor %llu \n", trigger_id, other_id);

                // OnTriggerEnter
                if (current.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) 
                { 
                    //stayTrigger = true;
                    state = trigger::onTriggerEnter;
                    //printf("Shape is ENTERING TRIGGER volume\n");
                }
                
                // OnTriggerExit
                if (current.status & PxPairFlag::eNOTIFY_TOUCH_LOST) 
                { 
                    //stayTrigger = false;
                    state = trigger::onTriggerExit;
                    //printf("trigger actor %llu, other actor %llu, state: %d\n", current.triggerActor->userData, current.otherActor->userData, state);
                    //printf("Shape is LEAVING TRIGGER volume\n");
                }

                // Store all the ID of the actors that collided with trigger)
                std::queue<TriggerManifold> trigger_data = physx_system::currentWorld->getTriggerData();

                // Add new TriggerManifold data into the queue
                trigger_data.emplace(TriggerManifold{ trigger_id, other_id, state });
            }
        }
    }

    void EventCallBack::onAdvance(const PxRigidBody* const* /*bodyBuffer*/, const PxTransform* /*poseBuffer*/, const PxU32 /*count*/) 
    {
        //printf("CALLBACK: onAdvance\n");
    }

    /*-----------------------------------------------------------------------------*/
    /*                               PVD                                           */
    /*-----------------------------------------------------------------------------*/
    PxPvd* PVD::createPvd(PxFoundation* foundation, const char* ip) 
    {
        mPVD = PxCreatePvd(*foundation);
        mTransport = PxDefaultPvdSocketTransportCreate(ip, 5425, 10);
        mPVD->connect(*mTransport, PxPvdInstrumentationFlag::eALL);

        return mPVD;
    }

    void PVD::setupPvd(PxScene* scene) 
    {
        PxPvdSceneClient* pvdClient = scene->getScenePvdClient();

        if (pvdClient)
        {
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
    }

    PxPvdTransport* PVD::getTransport() 
    {
        return mTransport;
    }

    PxPvd*& PVD::pvd__() 
    {
        return mPVD;
    }

    PxPvd* const& PVD::pvd__() const 
    {
        return mPVD;
    }

}