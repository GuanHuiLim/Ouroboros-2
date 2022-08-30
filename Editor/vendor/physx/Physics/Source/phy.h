#pragma once

//#include "PxConfig.h"
//#include "PxPhysics.h"

#include <Physics/Physx/include/PxPhysicsAPI.h>
//#include "iPxPhysicsAPI.h"

//#include "vehicle/PxVehicleSDK.h"

#include <iostream>
#include "phy.h"

using namespace physx;

//PxPhysics* gPhysics = NULL;
//PxScene* gScene = NULL;
//PxMaterial* gMaterial = NULL;


//PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0))
//{
//    PxRigidDynamic* dynamic = PxCreateDynamic(*mPhysics, t, geometry, *mMaterial, 10.0f);
//    dynamic->setAngularDamping(0.5f);
//    dynamic->setLinearVelocity(velocity);
//    mScene->addActor(*dynamic);
//    return dynamic;
//}
//
//void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
//{
//    PxShape* shape = mPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *mMaterial);
//    for (PxU32 i = 0; i < size; i++)
//    {
//        for (PxU32 j = 0; j < size - i; j++)
//        {
//            PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
//            PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
//            body->attachShape(*shape);
//            PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
//            mScene->addActor(*body);
//        }
//    }
//    shape->release();
//}

struct Phy {

    static void phyTest();
};
