#include "physics.h"

#include <assert.h>

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
        //for (auto const& i : mat) 
        //{
        //    i.second->release();
        //}

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

    // TODO : Take in position and orientation straight away for instant initialization.
    PhysicsObject PhysicsWorld::createInstance() 
    {
        PhysxObject obj;
        obj.id = std::make_unique<phy_uuid::UUID>();
        obj.m_material = mPhysics->createMaterial(0.f,0.f,0.f);
        
        // This is important!
        // default initialize here
        phy_uuid::UUID generated_uuid = *obj.id;
        if (obj.rigid_type == rigid::rstatic)
        {
            // create new rigidbody
            obj.rb.rigidStatic = mPhysics->createRigidStatic(PxTransform{});
        }
        else // obj.rigid_type == rigid::rdynamic
        {
            obj.rb.rigidDynamic = mPhysics->createRigidDynamic(PxTransform{});
        }

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

        //underlying_obj.m_shape->release(); // not 100% sure need or not

        if (underlying_obj.rigid_type == rigid::rstatic)
            underlying_obj.rb.rigidStatic->release();
        else if (underlying_obj.rigid_type == rigid::rdynamic)
            underlying_obj.rb.rigidDynamic->release();

        underlying_obj.m_material->release();

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

            setAllData(physicsNewObject, initialized_object);

            return physicsNewObject; // a copy
        }

        return PhysicsObject{}; // return empty
    }

    std::unordered_map<phy_uuid::UUID, PhysicsObject> PhysicsWorld::retrieveCurrentObjects() const
    {
        std::unordered_map<phy_uuid::UUID, PhysicsObject> updatedObjects;

        for (auto&& physx_obj : m_physx_objects)
        {
            PhysicsObject new_obj;

            retrieveOldData(new_obj, physx_obj);

            updatedObjects.insert({ new_obj.id, new_obj });
        }

        return updatedObjects;
    }

    void PhysicsWorld::retrieveOldData(PhysicsObject& physics_Obj, const PhysxObject& init_Obj) const {

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
        default:
            break;
        }

        // AXIS PROPERTIES
        physics_Obj.lockPositionAxis = init_Obj.lockPositionAxis;
        physics_Obj.lockRotationAxis = init_Obj.lockRotationAxis;

        physics_Obj.is_trigger = init_Obj.is_trigger;
        physics_Obj.gravity_enabled = init_Obj.gravity_enabled;
        physics_Obj.is_kinematic = init_Obj.is_kinematic;
        physics_Obj.is_collider_enabled = init_Obj.is_collider_enabled;
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

            setAllData(updatedObj, underlying_obj);
        }
    }

    void PhysicsWorld::setAllData(PhysicsObject& updatedPhysicsObj, PhysxObject& underlying_Obj) {

        // MATERIAL PROPERTIES
        underlying_Obj.m_material->setStaticFriction(updatedPhysicsObj.material.staticFriction);
        underlying_Obj.m_material->setDynamicFriction(updatedPhysicsObj.material.dynamicFriction);
        underlying_Obj.m_material->setRestitution(updatedPhysicsObj.material.restitution);

        PxRigidActor* rigidbody = nullptr;

        // CHECK IF CHANGING IN RIGID TYPE
        if (underlying_Obj.rigid_type != updatedPhysicsObj.rigid_type) 
        {
            // what is our existing shape? need to handle removing them first.
            if (underlying_Obj.rigid_type == rigid::rstatic) 
            {
                // clear the current data
                scene->removeActor(*underlying_Obj.rb.rigidStatic);
                underlying_Obj.rb.rigidStatic = nullptr;
            }
            else // underlying_Obj.rigid_type == rigid::rdynamic
            {
                scene->removeActor(*underlying_Obj.rb.rigidDynamic);
                underlying_Obj.rb.rigidDynamic = nullptr;
            }

            // Update our rigid type to desired type
            underlying_Obj.rigid_type = updatedPhysicsObj.rigid_type;

            // we check with the newly assigned rigid type to determine
            if (underlying_Obj.rigid_type == rigid::rstatic)
            {
                // create new rigidbody
                underlying_Obj.rb.rigidStatic = mPhysics->createRigidStatic(PxTransform{ updatedPhysicsObj.position, updatedPhysicsObj.orientation });
                rigidbody = underlying_Obj.rb.rigidStatic;
            }
            else // underlying_Obj.rigid_type == rigid::rdynamic
            {
                underlying_Obj.rb.rigidDynamic = mPhysics->createRigidDynamic(PxTransform{ updatedPhysicsObj.position, updatedPhysicsObj.orientation });
                rigidbody = underlying_Obj.rb.rigidDynamic;
            }

            assert(rigidbody, "This should NOT be nullptr at all at this point!");
            
            rigidbody->userData = underlying_Obj.id.get();
            scene->addActor(*rigidbody);

            // REATTACH SHAPE INTO NEW RIGIDBODY
            if (underlying_Obj.m_shape && underlying_Obj.shape_type != shape::none) 
            {
                rigidbody->attachShape(*underlying_Obj.m_shape);

                physx_system::setupFiltering(underlying_Obj.m_shape);
            }
        }
        
        // next we check for positional changes.
        {
            // set new position and orientation
            if (underlying_Obj.rigid_type == rigid::rstatic)
                underlying_Obj.rb.rigidStatic->setGlobalPose(PxTransform{ updatedPhysicsObj.position, updatedPhysicsObj.orientation });

            else if (underlying_Obj.rigid_type == rigid::rdynamic)
                underlying_Obj.rb.rigidDynamic->setGlobalPose(PxTransform{ updatedPhysicsObj.position, updatedPhysicsObj.orientation });
        }

        // RIGIDBODY PROPERTIES
        if (underlying_Obj.rigid_type == rigid::rstatic) {

            setShape(updatedPhysicsObj, underlying_Obj);
        }
        else if (underlying_Obj.rigid_type == rigid::rdynamic) {

            setShape(updatedPhysicsObj, underlying_Obj);

            underlying_Obj.rb.rigidDynamic->setMass(updatedPhysicsObj.mass);
            underlying_Obj.rb.rigidDynamic->setLinearDamping(updatedPhysicsObj.linearDamping);
            underlying_Obj.rb.rigidDynamic->setAngularDamping(updatedPhysicsObj.angularDamping);

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
        underlying_Obj.is_collider_enabled = updatedPhysicsObj.is_collider_enabled;
        underlying_Obj.is_trigger = updatedPhysicsObj.is_trigger;

        if (underlying_Obj.shape_type != shape::none) {

            if(underlying_Obj.is_collider_enabled) 
                underlying_Obj.m_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, underlying_Obj.is_collider_enabled);

            if (underlying_Obj.is_trigger)
                underlying_Obj.m_shape->setFlags(PxShapeFlag::eVISUALIZATION | PxShapeFlag::eTRIGGER_SHAPE);
            else
                underlying_Obj.m_shape->setFlags(PxShapeFlag::eVISUALIZATION | PxShapeFlag::eSIMULATION_SHAPE);
        }

    }

    void PhysicsWorld::setShape(PhysicsObject& updated_Obj, PhysxObject& underlying_Obj) {

        PxRigidActor* rigidbody = nullptr;

        if (underlying_Obj.rigid_type == rigid::rstatic)
            rigidbody = underlying_Obj.rb.rigidStatic;
        
        else if (underlying_Obj.rigid_type == rigid::rdynamic)
            rigidbody = underlying_Obj.rb.rigidDynamic;

        // CHECK IF WE NEED TO CHANGE SHAPE 
        if (underlying_Obj.shape_type != updated_Obj.shape_type) 
        {
            // if we have an existing shape, we must remove it
            if (underlying_Obj.shape_type != shape::none && underlying_Obj.m_shape)
                rigidbody->detachShape(*underlying_Obj.m_shape);

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
            case shape::none:
            default:
                return; // NOTE we return here because code below requires a shape!
            }

            // ATTACH THE NEW SHAPE TO THE OBJECT
            rigidbody->attachShape(*underlying_Obj.m_shape);

            physx_system::setupFiltering(underlying_Obj.m_shape);
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
            case shape::none:
            default:
                return; // NOTE we return here because code below requires a shape!
            }
        }

        //underlying_obj.m_shape->setContactOffset(1);
    }

    void PhysicsWorld::submitPhysicsCommand(std::vector<PhysicsCommand> physicsCommand) {

        for (auto&& updatedCommandObj : physicsCommand)
        {
            // CHECK WHETHER OBJECT EXISTED
            if (!m_objects_lookup.contains(updatedCommandObj.Id))
                continue;

            PhysxObject& underlying_obj = m_physx_objects[m_objects_lookup.at(updatedCommandObj.Id)];

            if (underlying_obj.rigid_type != rigid::rdynamic)
                continue;

            if(updatedCommandObj.AngularVel)
                underlying_obj.rb.rigidDynamic->setAngularVelocity(updatedCommandObj.angularVel);

            if (updatedCommandObj.LinearVel)
                underlying_obj.rb.rigidDynamic->setLinearVelocity(updatedCommandObj.linearVel);

            setForce(underlying_obj, updatedCommandObj);
            
            setTorque(underlying_obj, updatedCommandObj);
        }

    }

    void PhysicsWorld::setForce(PhysxObject& underlying_Obj, PhysicsCommand& command_Obj) {

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

    void PhysicsWorld::setTorque(PhysxObject& underlying_Obj, PhysicsCommand& command_Obj) {

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
    PhysxObject::PhysxObject(const PhysxObject& other) : //matID(other.matID),
                                                         m_material{other.m_material},
                                                         shape_type(other.shape_type),
                                                         rigid_type(other.rigid_type),
                                                         lockPositionAxis(other.lockPositionAxis),
                                                         lockRotationAxis(other.lockRotationAxis),
                                                         is_trigger(other.is_trigger),
                                                         gravity_enabled(other.gravity_enabled),
                                                         is_kinematic(other.is_kinematic),
                                                         is_collider_enabled(other.is_collider_enabled),
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
