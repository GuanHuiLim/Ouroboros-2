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

void Phy::phyTest() 
{
    std::cout << "Hello World!\n";

    // Declare variables
    PxDefaultAllocator      mDefaultAllocatorCallback;
    PxDefaultErrorCallback  mDefaultErrorCallback;
    PxDefaultCpuDispatcher* mDispatcher = NULL;
    PxTolerancesScale       mToleranceScale;

    PxFoundation* mFoundation = NULL;
    PxPhysics* mPhysics = NULL;

    PxScene* mScene = NULL;
    PxMaterial* mMaterial = NULL;

    PxPvd* mPvd = NULL;

    PxReal stackZ = 10.0f;

    // Init physx
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);
    if (!mFoundation) throw("PxCreateFoundation failed!");

    mPvd = PxCreatePvd(*mFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("192.168.1.162", 5425, 10);
    mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    mToleranceScale.length = 100;        // typical length of an object
    mToleranceScale.speed = 981;         // typical speed of an object, gravity*1s is a reasonable choice
    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale, true, mPvd);
    //mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, mToleranceScale);

    // Setup scene description
    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    mDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    mScene = mPhysics->createScene(sceneDesc);

    // Setup PVP 
    PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    // Create simulation
    mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, PxPlane(0, 1, 0, 50), *mMaterial); // collider shape
    mScene->addActor(*groundPlane);

    //PxShape* circleshape = mPhysics->createShape(PxSphereGeometry(10), *mMaterial);

    // Create box shape
    float halfExtent = .5f;
    PxShape* shape = mPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *mMaterial);
    PxU32 size = 5;
    PxTransform t(PxVec3(0));

    PxTransform t2(PxVec3(0, 20, 100));

    // Create wall
    for (PxU32 i = 0; i < size; i++) {
        for (PxU32 j = 0; j < size - i; j++) {

            PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
            PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
            body->attachShape(*shape);

            PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
            mScene->addActor(*body);
        }
    }
    shape->release();

    PxVec3 position(0, 30, 0);
    PxRigidDynamic* aSphereActor = PxCreateDynamic(*mPhysics, PxTransform(position), PxSphereGeometry(3), *mMaterial, 1.f);
    aSphereActor->setMass(1);
    mScene->addActor(*aSphereActor);

    PxReal mymass = aSphereActor->getMass();
    PxVec3 linearvel = aSphereActor->getLinearVelocity();

    printf("%f: mass", mymass);
    printf("\n%f: velo", linearvel.x);

    //PxRigidDynamic* dynamic = PxCreateDynamic(*mPhysics, t2, PxSphereGeometry(5), *mMaterial, 10.0f);
    //dynamic->setAngularDamping(0.5f);
    //dynamic->setLinearVelocity(PxVec3(0, -25, -100));
    //mScene->addActor(*dynamic);

    ////PxRigidDynamic* ball = createDynamic(PxTransform(PxVec3(0, 20, 100)), PxSphereGeometry(5), PxVec3(0, -25, -100));
    //PxRigidBodyExt::updateMassAndInertia(*dynamic, 1000.f);


    // test 2
    //PxRigidDynamic* aCapsuleActor = mPhysics->createRigidDynamic(PxTransform(PxVec3(0, 20, 100)));
    //PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));
    //PxShape* aCapsuleShape = PxRigidActorExt::createExclusiveShape(*aCapsuleActor, PxCapsuleGeometry(PxReal(5.f), PxReal(10.f)), mMaterial);
    //aCapsuleShape->setLocalPose(relativePose);
    //PxRigidBodyExt::updateMassAndInertia(*aCapsuleActor, 10.f);
    //mScene->addActor(*aCapsuleActor);

    // test
    //for (PxU32 i = 0; i < 40; i++)
    //    createStack(PxTransform(PxVec3(0, 0, stackZ -= 10.0f)), 20, 1.0f);

    //PxRigidDynamic* ball = createDynamic(PxTransform(PxVec3(0, 20, 100)), PxSphereGeometry(5), PxVec3(0, -25, -100));
    //PxRigidBodyExt::updateMassAndInertia(*ball, 1000.f);

    // Run simulation
    //while (1) {
    //    mScene->simulate(1.0f / 60.0f); // 60 frame per seconds
    //    mScene->fetchResults(true);
    //
    //    //std::string hello;
    //    //std::cin >> hello;
    //}


}
