/************************************************************************************//*!
\file           phy.h
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

#pragma once

#pragma warning( push )
#pragma warning( disable : 26812 )
#pragma warning( disable : 26495 )
#pragma warning( disable : 26451 )
#pragma warning( disable : 33010 )
#include <Physics/Physx/include/PxPhysicsAPI.h>
#pragma warning( pop )

#include <iostream>
#include <vector>
#include <map>
//#include <glm/glm.hpp>

#include "uuid.h"

#define PVD_DEBUGGER false

using namespace physx;

namespace myPhysx {

    class Collision;

    class PhysxWorld;
    class PVD;
    struct PhysxObject;
    struct PhysicsObject;

    enum class rigid { none, rstatic, rdynamic };
    enum class shape { none, box, sphere, capsule, plane };
    enum class force { force, acceleration, impulse, velocityChanged };

    class EventCallBack : public PxSimulationEventCallback {

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

        void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 count) override {
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

        void onTrigger(PxTriggerPair* pairs, PxU32 count) override {
            printf("CALLBACK: onTrigger\n");
            printf("PAIRS: %d\n", count);

            while (count--) {

                const PxTriggerPair& current = *pairs++;

                if (current.status & PxPairFlag::eNOTIFY_TOUCH_FOUND) // OnTriggerEnter
                    printf("Shape is entering trigger volume\n");

                if (current.status & PxPairFlag::eNOTIFY_TOUCH_PERSISTS) // OnTriggerStay
                    printf("Shape is still within trigger volume\n");

                if (current.status & PxPairFlag::eNOTIFY_TOUCH_LOST) // OnTriggerExit
                    printf("Shape is leaving trigger volume\n");
            }
        }

        void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override {
            printf("CALLBACK: onAdvance\n");
        }
    };

    struct RigidDynamic {

        PxRigidDynamic* rigidDynamic = nullptr;
    };

    struct RigidStatic {

        PxRigidStatic* rigidStatic = nullptr;
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

        bool isTrigger(const PxFilterData& data);

        bool isTriggerShape(PxShape* shape);
    };

    // describes a physics scene
    class PhysxWorld {

    private:

        friend struct PhysicsObject;

        PxScene* scene = nullptr;
        std::map<phy_uuid::UUID, PxMaterial*> mat;
        PxVec3 gravity;

        std::map<phy_uuid::UUID, int> all_objects; // store all the index of the objects (lookups for keys / check if empty)

        std::vector<PhysxObject> m_objects; // to iterate through for setting the data

    public:

        // SCENE
        PhysxWorld(PxVec3 gravity);
        ~PhysxWorld();
        void updateScene(float dt);

        // GRAVITY
        PxVec3 getWorldGravity() const;
        void setWorldGravity(PxVec3 gra);

        // RIGIDBODY
        PhysicsObject createInstance();
        void removeInstance(PhysicsObject obj);

        //CHECKING QUERY
    };

    // associated to each object in the physics world (me store)
    struct PhysxObject {

        phy_uuid::UUID id = 0;
        phy_uuid::UUID matID = 0;

        // shape
        PxShape* m_shape = nullptr; // prob no need this
        shape shape = shape::none;

        // ensure at least static or dynamic is init
        RigidStatic rs{};
        RigidDynamic rd{};

        rigid rigidID = rigid::none;

        bool trigger = false;
        bool gravity = true; // static should be false
        bool kinematic = false;
    };

    struct PhysicsObject { // you store

        phy_uuid::UUID id;
        PhysxWorld* world;

        // GETTERS
        Material getMaterial() const;
        PxVec3 getposition() const;
        PxQuat getOrientation() const;

        PxReal getMass() const;
        PxReal getInvMass() const;
        PxReal getAngularDamping() const;
        PxVec3 getAngularVelocity() const;
        PxReal getLinearDamping() const;
        PxVec3 getLinearVelocity() const;

        bool getTrigger() const;
        bool getGravity() const;
        bool getKinematic() const;

        // SETTERS
        void setRigidType(rigid type);
        void setMaterial(Material material);
        void setPosOrientation(PxVec3 pos, PxQuat quat);

        void setMass(PxReal mass);
        void setMassSpaceInertia(PxVec3 mass);
        void setAngularDamping(PxReal angularDamping);
        void setAngularVelocity(PxVec3 angularVelocity);
        void setLinearDamping(PxReal linearDamping);
        void setLinearVelocity(PxVec3 linearVelocity);

        void disableGravity(bool gravity);
        void enableKinematic(bool kine);

        // TRIGGERS
        void setTriggerShape(bool trigger);
        
        // FORCE
        void addForce(PxVec3 f_amount, force f);
        void addTorque(PxVec3 f_amount, force f);

        // set default value for each type of shape & can change shape too
        void setShape(shape shape);
        void removeShape();

        // change each individual property based on its shape
        void setBoxProperty(float halfextent_width, float halfextent_height, float halfextent_depth);
        void setSphereProperty(float radius);
        void setCapsuleProperty(float radius, float halfHeight);
        //void setPlaneProperty(float radius);

    };


    // Physx visual degguer
    class PVD {

    private:

        PxPvd* mPVD;

        PxPvdTransport* mTransport;

    public:

        PxPvd* createPvd(PxFoundation* foundation, const char* ip);

        void setupPvd(PxScene* scene);

        PxPvdTransport* getTransport();

        PxPvd*& pvd__();

        PxPvd* const& pvd__() const;
    };
};