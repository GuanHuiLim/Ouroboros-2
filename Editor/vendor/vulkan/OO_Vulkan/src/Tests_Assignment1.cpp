#include "Tests_Assignment1.h"
#include <iostream>
#include <iomanip>

//auto TestSphereSphereFn = coll::SphereSphere;
bool(*TestSphereSphere)(const Sphere&, const Sphere&) = coll::SphereSphere;
//auto TestAabbAabb = coll::AabbAabb;
bool(*TestAabbAabb)(const Aabb&, const Aabb&) = coll::AabbAabb;
//auto TestPointPlane = coll::PointPlane;
float(*TestPointPlane)(const Point&, const Plane&) = coll::DistPointPlane;
//auto TestPointSphere = coll::PointAabb;
bool(*TestPointSphere)(const Point&, const Sphere&) = coll::PointSphere;
//auto TestPointAabb = TestPointAabb;
bool(*TestPointAabb)(const Point&, const Aabb&) = coll::PointAabb;
//auto TestBarycentricTriangle = TestBarycentricTriangle;
bool(*TestBarycentricTriangle)( const Point&,const Point&,const Point&, float,float,float) = coll::BaryCheckUVW;
//auto TestRayPlane = TestRayPlane;
bool(*TestRayPlane)(const Ray&, const Plane&, float& t) = coll::RayPlane;
//auto TestRayTriangle = TestRayTriangle;
bool(*TestRayTriangle)(const Ray&, const Triangle&, float& t) = coll::RayTriangle;
//auto TestRaySphere = TestRaySphere;
bool(*TestRaySphere)(const Ray&, const Sphere&, float& t) = coll::RaySphere;
//auto TestRayAabb = TestRayAabb;
bool(*TestRayAabb)(const Ray&, const Aabb&, float&) = coll::RayAabb;
//auto TestPlaneSphere = TestPlaneSphere;
bool(*TestPlaneSphere)(const Plane&, const Sphere&, float& t) = coll::PlaneSphere;
//auto TestPlaneAabb = TestPlaneAabb;
bool(*TestPlaneAabb)(const Plane&, const Aabb&, float& t) = coll::PlaneAabb;

void PrintResult(bool s)
{
	std::cout << "  Result:" << (s ?"true":"false") << std::endl;
}

void PrintResultUVW(bool s, float u, float v, float w)
{
	std::cout << "  Result:" << (s ?"true":"false") << " (u, v, w): ("<<std::fixed <<std::setprecision(2) << u << ", " << v << ", " << w << ")" << std::endl;
}

void PrintResultRay(bool s, const float& t)
{
	if (s)
	{
		std::cout << "  Result:" <<"true" << " t:"<< t << std::endl;
		return;
	}
	std::cout << "  Result:" << "false" << std::endl;	
}

void PrintResultPlane(const float& t, float episilon)
{
	std::cout << "  Result:";
	if (std::abs(t) < episilon)
	{
		std::cout << "Coplanar" << std::endl;
		return;
	}
	if (t > 0.0f)
	{
		std::cout << "Inside" << std::endl;		
		return;
	}	
	std::cout << "Outside" << std::endl;		
}

void PrintResultPlane(bool s, const float& t)
{
	std::cout << "  Result:";
	if (s)
	{
		std::cout << "Overlaps" << std::endl;
		return;
	}
	if (t > 0.0f)
	{
		std::cout << "Inside" << std::endl;		
		return;
	}	
	std::cout << "Outside" << std::endl;		
}

int RunAllTests()
{
	SphereSphereTest1("SphereSphereTest1");
	SphereSphereTest2("SphereSphereTest2");
	SphereSphereTest3("SphereSphereTest3");
	SphereSphereTest4("SphereSphereTest4");
	SphereSphereTest5("SphereSphereTest5");
	SphereSphereTest6("SphereSphereTest6");
	SphereSphereTest7("SphereSphereTest7");
	SphereSphereTest8("SphereSphereTest8");
	SphereSphereTest9("SphereSphereTest9");
	SphereSphereTest10("SphereSphereTest10");
	SphereSphereTest11("SphereSphereTest11");
	SphereSphereTest12("SphereSphereTest12");
	SphereSphereTest13("SphereSphereTest13");
	SphereSphereTest14("SphereSphereTest14");
	SphereSphereTest15("SphereSphereTest15");

	AabbAabbTest1("AabbAabbTest1");
	AabbAabbTest2("AabbAabbTest2");
	AabbAabbTest3("AabbAabbTest3");
	AabbAabbTest4("AabbAabbTest4");
	AabbAabbTest5("AabbAabbTest5");
	AabbAabbTest6("AabbAabbTest6");
	AabbAabbTest7("AabbAabbTest7");
	AabbAabbTest8("AabbAabbTest8");
	AabbAabbTest9("AabbAabbTest9");
	AabbAabbTest10("AabbAabbTest10");
	AabbAabbTest11("AabbAabbTest11");
	AabbAabbTest12("AabbAabbTest12");
	AabbAabbTest13("AabbAabbTest13");
	AabbAabbTest14("AabbAabbTest14");
	AabbAabbTest15("AabbAabbTest15");
	AabbAabbTest16("AabbAabbTest16");
	AabbAabbTest17("AabbAabbTest17");
	AabbAabbTest18("AabbAabbTest18");
	AabbAabbTest19("AabbAabbTest19");
	AabbAabbTest20("AabbAabbTest20");
	AabbAabbTest21("AabbAabbTest21");
	AabbAabbTest22("AabbAabbTest22");
	AabbAabbTest23("AabbAabbTest23");
	AabbAabbTest24("AabbAabbTest24");
	AabbAabbTest25("AabbAabbTest25");
	AabbAabbTest26("AabbAabbTest26");
	AabbAabbTest27("AabbAabbTest27");
	AabbAabbTest28("AabbAabbTest28");
	AabbAabbTest29("AabbAabbTest29");
	AabbAabbTest30("AabbAabbTest30");
	AabbAabbTest31("AabbAabbTest31");
	AabbAabbTest32("AabbAabbTest32");
	AabbAabbTest33("AabbAabbTest33");
	AabbAabbTest34("AabbAabbTest34");
	AabbAabbTest35("AabbAabbTest35");
	AabbAabbTest36("AabbAabbTest36");
	AabbAabbTest37("AabbAabbTest37");
	AabbAabbTest38("AabbAabbTest38");
	AabbAabbTest39("AabbAabbTest39");
	AabbAabbTest40("AabbAabbTest40");
	AabbAabbTest41("AabbAabbTest41");
	AabbAabbTest42("AabbAabbTest42");
	AabbAabbTest43("AabbAabbTest43");
	AabbAabbTest44("AabbAabbTest44");
	AabbAabbTest45("AabbAabbTest45");
	AabbAabbTest46("AabbAabbTest46");
	AabbAabbTest47("AabbAabbTest47");
	AabbAabbTest48("AabbAabbTest48");
	AabbAabbTest49("AabbAabbTest49");
	AabbAabbTest50("AabbAabbTest50");
	AabbAabbTest51("AabbAabbTest51");
	AabbAabbTest52("AabbAabbTest52");
	AabbAabbTest53("AabbAabbTest53");
	AabbAabbTest54("AabbAabbTest54");
	AabbAabbTest55("AabbAabbTest55");
	AabbAabbTest56("AabbAabbTest56");
	AabbAabbTest57("AabbAabbTest57");
	AabbAabbTest58("AabbAabbTest58");
	AabbAabbTest59("AabbAabbTest59");
	AabbAabbTest60("AabbAabbTest60");
	AabbAabbTest61("AabbAabbTest61");
	AabbAabbTest62("AabbAabbTest62");
	AabbAabbTest63("AabbAabbTest63");
	AabbAabbTest64("AabbAabbTest64");
	AabbAabbTest65("AabbAabbTest65");
	AabbAabbTest66("AabbAabbTest66");
	AabbAabbTest67("AabbAabbTest67");
	AabbAabbTest68("AabbAabbTest68");
	AabbAabbTest69("AabbAabbTest69");
	AabbAabbTest70("AabbAabbTest70");
	AabbAabbTest71("AabbAabbTest71");
	AabbAabbTest72("AabbAabbTest72");
	AabbAabbTest73("AabbAabbTest73");
	AabbAabbTest74("AabbAabbTest74");
	AabbAabbTest75("AabbAabbTest75");
	AabbAabbTest76("AabbAabbTest76");
	AabbAabbTest77("AabbAabbTest77");
	AabbAabbTest78("AabbAabbTest78");
	AabbAabbTest79("AabbAabbTest79");
	AabbAabbTest80("AabbAabbTest80");
	AabbAabbTest81("AabbAabbTest81");
	AabbAabbTest82("AabbAabbTest82");
	AabbAabbTest83("AabbAabbTest83");
	AabbAabbTest84("AabbAabbTest84");

	PointPlaneTest1("PointPlaneTest1");
	PointPlaneTest2("PointPlaneTest2");
	PointPlaneTest3("PointPlaneTest3");
	PointPlaneTest4("PointPlaneTest4");
	PointPlaneTest5("PointPlaneTest5");
	PointPlaneTest6("PointPlaneTest6");
	PointPlaneTest7("PointPlaneTest7");
	PointPlaneTest8("PointPlaneTest8");
	PointPlaneTest9("PointPlaneTest9");
	PointPlaneTest10("PointPlaneTest10");
	PointPlaneTest11("PointPlaneTest11");
	PointPlaneTest12("PointPlaneTest12");
	PointPlaneTest13("PointPlaneTest13");
	PointPlaneTest14("PointPlaneTest14");
	PointPlaneTest15("PointPlaneTest15");
	PointPlaneTest16("PointPlaneTest16");
	PointPlaneTest17("PointPlaneTest17");
	PointPlaneTest18("PointPlaneTest18");
	PointPlaneTest19("PointPlaneTest19");
	PointPlaneTest20("PointPlaneTest20");
	PointPlaneTest21("PointPlaneTest21");
	PointPlaneTest22("PointPlaneTest22");
	PointPlaneTest23("PointPlaneTest23");
	PointPlaneTest24("PointPlaneTest24");
	PointPlaneTest25("PointPlaneTest25");
	PointPlaneTest26("PointPlaneTest26");
	PointPlaneTest27("PointPlaneTest27");
	PointPlaneTest28("PointPlaneTest28");
	PointPlaneTest29("PointPlaneTest29");
	PointPlaneTest30("PointPlaneTest30");
	PointPlaneTest31("PointPlaneTest31");
	PointPlaneTest32("PointPlaneTest32");
	PointPlaneTest33("PointPlaneTest33");
	PointPlaneTest34("PointPlaneTest34");
	PointPlaneTest35("PointPlaneTest35");
	PointPlaneTest36("PointPlaneTest36");
	PointPlaneTest37("PointPlaneTest37");
	PointPlaneTest38("PointPlaneTest38");
	PointPlaneTest39("PointPlaneTest39");
	PointPlaneTest40("PointPlaneTest40");
	PointSphereTest1("PointSphereTest1");
	PointSphereTest2("PointSphereTest2");
	PointSphereTest3("PointSphereTest3");
	PointSphereTest4("PointSphereTest4");
	PointSphereTest5("PointSphereTest5");
	PointSphereTest6("PointSphereTest6");
	PointSphereTest7("PointSphereTest7");
	PointSphereTest8("PointSphereTest8");
	PointSphereTest9("PointSphereTest9");
	PointSphereTest10("PointSphereTest10");
	PointSphereTest11("PointSphereTest11");
	PointSphereTest12("PointSphereTest12");
	PointSphereTest13("PointSphereTest13");
	PointSphereTest14("PointSphereTest14");
	PointSphereTest15("PointSphereTest15");
	PointSphereTest16("PointSphereTest16");
	PointSphereTest17("PointSphereTest17");
	PointSphereTest18("PointSphereTest18");
	PointSphereTest19("PointSphereTest19");
	PointSphereTest20("PointSphereTest20");
	PointSphereTest21("PointSphereTest21");
	PointSphereTest22("PointSphereTest22");
	PointSphereTest23("PointSphereTest23");
	PointSphereTest24("PointSphereTest24");
	PointSphereTest25("PointSphereTest25");
	PointSphereTest26("PointSphereTest26");

	PointAabbTest1("PointAabbTest1");
	PointAabbTest2("PointAabbTest2");
	PointAabbTest3("PointAabbTest3");
	PointAabbTest4("PointAabbTest4");
	PointAabbTest5("PointAabbTest5");
	PointAabbTest6("PointAabbTest6");
	PointAabbTest7("PointAabbTest7");
	PointAabbTest8("PointAabbTest8");
	PointAabbTest9("PointAabbTest9");
	PointAabbTest10("PointAabbTest10");
	PointAabbTest11("PointAabbTest11");
	PointAabbTest12("PointAabbTest12");
	PointAabbTest13("PointAabbTest13");
	PointAabbTest14("PointAabbTest14");
	PointAabbTest15("PointAabbTest15");
	PointAabbTest16("PointAabbTest16");
	PointAabbTest17("PointAabbTest17");
	PointAabbTest18("PointAabbTest18");
	PointAabbTest19("PointAabbTest19");
	PointAabbTest20("PointAabbTest20");
	PointAabbTest21("PointAabbTest21");
	PointAabbTest22("PointAabbTest22");
	PointAabbTest23("PointAabbTest23");
	PointAabbTest24("PointAabbTest24");
	PointAabbTest25("PointAabbTest25");
	PointAabbTest26("PointAabbTest26");
	PointAabbTest27("PointAabbTest27");
	PointAabbTest28("PointAabbTest28");
	PointAabbTest29("PointAabbTest29");
	PointAabbTest30("PointAabbTest30");
	PointAabbTest31("PointAabbTest31");
	PointAabbTest32("PointAabbTest32");
	PointAabbTest33("PointAabbTest33");
	PointAabbTest34("PointAabbTest34");
	PointAabbTest35("PointAabbTest35");
	PointAabbTest36("PointAabbTest36");
	PointAabbTest37("PointAabbTest37");
	PointAabbTest38("PointAabbTest38");
	PointAabbTest39("PointAabbTest39");
	PointAabbTest40("PointAabbTest40");
	PointAabbTest41("PointAabbTest41");
	PointAabbTest42("PointAabbTest42");
	PointAabbTest43("PointAabbTest43");
	PointAabbTest44("PointAabbTest44");
	PointAabbTest45("PointAabbTest45");
	PointAabbTest46("PointAabbTest46");
	PointAabbTest47("PointAabbTest47");
	PointAabbTest48("PointAabbTest48");
	PointAabbTest49("PointAabbTest49");
	PointAabbTest50("PointAabbTest50");
	PointAabbTest51("PointAabbTest51");
	PointAabbTest52("PointAabbTest52");
	PointAabbTest53("PointAabbTest53");
	PointAabbTest54("PointAabbTest54");
	PointAabbTest55("PointAabbTest55");
	PointAabbTest56("PointAabbTest56");
	PointAabbTest57("PointAabbTest57");
	PointAabbTest58("PointAabbTest58");
	PointAabbTest59("PointAabbTest59");
	PointAabbTest60("PointAabbTest60");
	PointAabbTest61("PointAabbTest61");
	PointAabbTest62("PointAabbTest62");
	PointAabbTest63("PointAabbTest63");
	PointAabbTest64("PointAabbTest64");
	PointAabbTest65("PointAabbTest65");
	PointAabbTest66("PointAabbTest66");
	PointAabbTest67("PointAabbTest67");
	PointAabbTest68("PointAabbTest68");
	PointAabbTest69("PointAabbTest69");
	PointAabbTest70("PointAabbTest70");
	PointAabbTest71("PointAabbTest71");
	PointAabbTest72("PointAabbTest72");
	PointAabbTest73("PointAabbTest73");
	PointAabbTest74("PointAabbTest74");
	PointAabbTest75("PointAabbTest75");
	PointAabbTest76("PointAabbTest76");
	PointAabbTest77("PointAabbTest77");
	PointAabbTest78("PointAabbTest78");
	PointAabbTest79("PointAabbTest79");
	PointAabbTest80("PointAabbTest80");
	PointAabbTest81("PointAabbTest81");
	PointAabbTest82("PointAabbTest82");
	PointAabbTest83("PointAabbTest83");
	PointAabbTest84("PointAabbTest84");

	BarycentricTriangleTest0("BarycentricTriangleTest0");
	BarycentricTriangleTest1("BarycentricTriangleTest1");
	BarycentricTriangleTest2("BarycentricTriangleTest2");
	BarycentricTriangleTest3("BarycentricTriangleTest3");
	BarycentricTriangleTest4("BarycentricTriangleTest4");
	BarycentricTriangleTest5("BarycentricTriangleTest5");
	BarycentricTriangleTest6("BarycentricTriangleTest6");
	BarycentricTriangleTest7("BarycentricTriangleTest7");
	BarycentricTriangleTest8("BarycentricTriangleTest8");
	BarycentricTriangleTest9("BarycentricTriangleTest9");
	BarycentricTriangleTest10("BarycentricTriangleTest10");
	BarycentricTriangleTest11("BarycentricTriangleTest11");
	BarycentricTriangleTest12("BarycentricTriangleTest12");
	BarycentricTriangleTest13("BarycentricTriangleTest13");
	BarycentricTriangleTest14("BarycentricTriangleTest14");
	BarycentricTriangleTest15("BarycentricTriangleTest15");
	BarycentricTriangleTest16("BarycentricTriangleTest16");
	BarycentricTriangleTest17("BarycentricTriangleTest17");
	BarycentricTriangleTest18("BarycentricTriangleTest18");
	BarycentricTriangleTest19("BarycentricTriangleTest19");
	BarycentricTriangleTest20("BarycentricTriangleTest20");
	BarycentricTriangleTest21("BarycentricTriangleTest21");
	BarycentricTriangleTest22("BarycentricTriangleTest22");
	BarycentricTriangleTest23("BarycentricTriangleTest23");
	BarycentricTriangleTest24("BarycentricTriangleTest24");
	BarycentricTriangleTest25("BarycentricTriangleTest25");
	BarycentricTriangleTest26("BarycentricTriangleTest26");
	BarycentricTriangleTest27("BarycentricTriangleTest27");
	BarycentricTriangleTest28("BarycentricTriangleTest28");
	BarycentricTriangleTest29("BarycentricTriangleTest29");
	BarycentricTriangleTest30("BarycentricTriangleTest30");
	BarycentricTriangleTest31("BarycentricTriangleTest31");
	BarycentricTriangleTest32("BarycentricTriangleTest32");
	BarycentricTriangleTest33("BarycentricTriangleTest33");
	BarycentricTriangleTest34("BarycentricTriangleTest34");
	BarycentricTriangleTest35("BarycentricTriangleTest35");
	BarycentricTriangleTest36("BarycentricTriangleTest36");
	BarycentricTriangleTest37("BarycentricTriangleTest37");
	BarycentricTriangleTest38("BarycentricTriangleTest38");
	BarycentricTriangleTest39("BarycentricTriangleTest39");
	BarycentricTriangleTest40("BarycentricTriangleTest40");
	BarycentricTriangleTest41("BarycentricTriangleTest41");
	BarycentricTriangleTest42("BarycentricTriangleTest42");
	BarycentricTriangleTest43("BarycentricTriangleTest43");
	BarycentricTriangleTest44("BarycentricTriangleTest44");
	BarycentricTriangleTest45("BarycentricTriangleTest45");
	BarycentricTriangleTest46("BarycentricTriangleTest46");
	BarycentricTriangleTest47("BarycentricTriangleTest47");
	BarycentricTriangleTest48("BarycentricTriangleTest48");
	BarycentricTriangleTest49("BarycentricTriangleTest49");
	BarycentricTriangleTest50("BarycentricTriangleTest50");
	BarycentricTriangleTest51("BarycentricTriangleTest51");
	BarycentricTriangleTest52("BarycentricTriangleTest52");
	BarycentricTriangleTest53("BarycentricTriangleTest53");
	BarycentricTriangleTest54("BarycentricTriangleTest54");
	BarycentricTriangleTest55("BarycentricTriangleTest55");
	BarycentricTriangleTest56("BarycentricTriangleTest56");
	BarycentricTriangleTest57("BarycentricTriangleTest57");
	BarycentricTriangleTest58("BarycentricTriangleTest58");
	BarycentricTriangleTest59("BarycentricTriangleTest59");
	BarycentricTriangleTest60("BarycentricTriangleTest60");
	BarycentricTriangleTest61("BarycentricTriangleTest61");
	BarycentricTriangleTest62("BarycentricTriangleTest62");
	BarycentricTriangleTest63("BarycentricTriangleTest63");
	BarycentricTriangleTest64("BarycentricTriangleTest64");

	RayPlaneTest1("RayPlaneTest1");
	RayPlaneTest2("RayPlaneTest2");
	RayPlaneTest3("RayPlaneTest3");
	RayPlaneTest4("RayPlaneTest4");
	RayPlaneTest5("RayPlaneTest5");
	RayPlaneTest6("RayPlaneTest6");
	RayPlaneTest7("RayPlaneTest7");
	RayPlaneTest8("RayPlaneTest8");
	RayPlaneTest9("RayPlaneTest9");
	RayPlaneTest10("RayPlaneTest10");
	RayPlaneTest11("RayPlaneTest11");
	RayPlaneTest12("RayPlaneTest12");
	RayPlaneTest13("RayPlaneTest13");
	RayPlaneTest14("RayPlaneTest14");
	RayPlaneTest15("RayPlaneTest15");
	RayPlaneTest16("RayPlaneTest16");
	RayPlaneTest17("RayPlaneTest17");
	RayPlaneTest18("RayPlaneTest18");
	RayPlaneTest19("RayPlaneTest19");
	RayPlaneTest20("RayPlaneTest20");
	RayPlaneTest21("RayPlaneTest21");
	RayPlaneTest22("RayPlaneTest22");
	RayPlaneTest23("RayPlaneTest23");
	RayPlaneTest24("RayPlaneTest24");
	RayPlaneTest25("RayPlaneTest25");
	RayPlaneTest26("RayPlaneTest26");
	RayPlaneTest27("RayPlaneTest27");
	RayPlaneTest28("RayPlaneTest28");
	RayPlaneTest29("RayPlaneTest29");
	RayPlaneTest30("RayPlaneTest30");
	RayPlaneTest31("RayPlaneTest31");
	RayPlaneTest32("RayPlaneTest32");
	RayPlaneTest33("RayPlaneTest33");
	RayPlaneTest34("RayPlaneTest34");
	RayPlaneTest35("RayPlaneTest35");
	RayPlaneTest36("RayPlaneTest36");
	RayPlaneTest37("RayPlaneTest37");
	RayPlaneTest38("RayPlaneTest38");
	RayPlaneTest39("RayPlaneTest39");
	RayPlaneTest40("RayPlaneTest40");
	RayPlaneTest41("RayPlaneTest41");
	RayPlaneTest42("RayPlaneTest42");
	RayPlaneTest43("RayPlaneTest43");
	RayPlaneTest44("RayPlaneTest44");
	RayPlaneTest45("RayPlaneTest45");
	RayPlaneTest46("RayPlaneTest46");
	RayPlaneTest47("RayPlaneTest47");
	RayPlaneTest48("RayPlaneTest48");
	RayPlaneTest49("RayPlaneTest49");
	RayPlaneTest50("RayPlaneTest50");
	RayPlaneTest51("RayPlaneTest51");
	RayPlaneTest52("RayPlaneTest52");
	RayPlaneTest53("RayPlaneTest53");
	RayPlaneTest54("RayPlaneTest54");
	RayPlaneTest55("RayPlaneTest55");
	RayPlaneTest56("RayPlaneTest56");
	RayPlaneTest57("RayPlaneTest57");
	RayPlaneTest58("RayPlaneTest58");
	RayPlaneTest59("RayPlaneTest59");
	RayPlaneTest60("RayPlaneTest60");
	RayPlaneTest61("RayPlaneTest61");
	RayPlaneTest62("RayPlaneTest62");
	RayPlaneTest63("RayPlaneTest63");
	RayPlaneTest64("RayPlaneTest64");
	RayPlaneTest65("RayPlaneTest65");
	RayPlaneTest66("RayPlaneTest66");
	RayPlaneTest67("RayPlaneTest67");
	RayPlaneTest68("RayPlaneTest68");
	RayPlaneTest69("RayPlaneTest69");
	RayPlaneTest70("RayPlaneTest70");
	RayPlaneTest71("RayPlaneTest71");
	RayPlaneTest72("RayPlaneTest72");

	RayTriangleTest1("RayTriangleTest1");
	RayTriangleTest2("RayTriangleTest2");
	RayTriangleTest3("RayTriangleTest3");
	RayTriangleTest4("RayTriangleTest4");
	RayTriangleTest5("RayTriangleTest5");
	RayTriangleTest6("RayTriangleTest6");
	RayTriangleTest7("RayTriangleTest7");
	RayTriangleTest8("RayTriangleTest8");
	RayTriangleTest9("RayTriangleTest9");
	RayTriangleTest10("RayTriangleTest10");
	RayTriangleTest11("RayTriangleTest11");
	RayTriangleTest12("RayTriangleTest12");
	RayTriangleTest13("RayTriangleTest13");
	RayTriangleTest14("RayTriangleTest14");
	RayTriangleTest15("RayTriangleTest15");
	RayTriangleTest16("RayTriangleTest16");
	RayTriangleTest17("RayTriangleTest17");
	RayTriangleTest18("RayTriangleTest18");
	RayTriangleTest19("RayTriangleTest19");
	RayTriangleTest20("RayTriangleTest20");
	RayTriangleTest21("RayTriangleTest21");
	RayTriangleTest22("RayTriangleTest22");
	RayTriangleTest23("RayTriangleTest23");
	RayTriangleTest24("RayTriangleTest24");
	RayTriangleTest25("RayTriangleTest25");
	RayTriangleTest26("RayTriangleTest26");
	RayTriangleTest27("RayTriangleTest27");
	RayTriangleTest28("RayTriangleTest28");
	RayTriangleTest29("RayTriangleTest29");
	RayTriangleTest30("RayTriangleTest30");

	RaySphereTest1("RaySphereTest1");
	RaySphereTest2("RaySphereTest2");
	RaySphereTest3("RaySphereTest3");
	RaySphereTest4("RaySphereTest4");
	RaySphereTest5("RaySphereTest5");
	RaySphereTest6("RaySphereTest6");
	RaySphereTest7("RaySphereTest7");
	RaySphereTest8("RaySphereTest8");
	RaySphereTest9("RaySphereTest9");
	RaySphereTest10("RaySphereTest10");
	RaySphereTest11("RaySphereTest11");
	RaySphereTest12("RaySphereTest12");
	RaySphereTest13("RaySphereTest13");
	RaySphereTest14("RaySphereTest14");
	RaySphereTest15("RaySphereTest15");
	RaySphereTest16("RaySphereTest16");
	RaySphereTest17("RaySphereTest17");
	RaySphereTest18("RaySphereTest18");
	RaySphereTest19("RaySphereTest19");
	RaySphereTest20("RaySphereTest20");
	RaySphereTest21("RaySphereTest21");
	RaySphereTest22("RaySphereTest22");
	RaySphereTest23("RaySphereTest23");
	RaySphereTest24("RaySphereTest24");
	RaySphereTest25("RaySphereTest25");
	RaySphereTest26("RaySphereTest26");
	RaySphereTest27("RaySphereTest27");
	RaySphereTest28("RaySphereTest28");
	RaySphereTest29("RaySphereTest29");
	RaySphereTest30("RaySphereTest30");
	RaySphereTest31("RaySphereTest31");
	RaySphereTest32("RaySphereTest32");
	RaySphereTest33("RaySphereTest33");
	RaySphereTest34("RaySphereTest34");
	RaySphereTest35("RaySphereTest35");
	RaySphereTest36("RaySphereTest36");
	RaySphereTest37("RaySphereTest37");
	RaySphereTest38("RaySphereTest38");
	RaySphereTest39("RaySphereTest39");
	RaySphereTest40("RaySphereTest40");
	RaySphereTest41("RaySphereTest41");
	RaySphereTest42("RaySphereTest42");
	RaySphereTest43("RaySphereTest43");
	RaySphereTest44("RaySphereTest44");
	RaySphereTest45("RaySphereTest45");
	RaySphereTest46("RaySphereTest46");
	RaySphereTest47("RaySphereTest47");
	RaySphereTest48("RaySphereTest48");
	RaySphereTest49("RaySphereTest49");
	RaySphereTest50("RaySphereTest50");
	RaySphereTest51("RaySphereTest51");
	RaySphereTest52("RaySphereTest52");
	RaySphereTest53("RaySphereTest53");
	RaySphereTest54("RaySphereTest54");
	RaySphereTest55("RaySphereTest55");
	RaySphereTest56("RaySphereTest56");
	RaySphereTest57("RaySphereTest57");
	RaySphereTest58("RaySphereTest58");
	RaySphereTest59("RaySphereTest59");
	RaySphereTest60("RaySphereTest60");
	RaySphereTest61("RaySphereTest61");
	RaySphereTest62("RaySphereTest62");
	RaySphereTest63("RaySphereTest63");
	RaySphereTest64("RaySphereTest64");
	RaySphereTest65("RaySphereTest65");
	RaySphereTest66("RaySphereTest66");
	RaySphereTest67("RaySphereTest67");
	RaySphereTest68("RaySphereTest68");
	RaySphereTest69("RaySphereTest69");
	RaySphereTest70("RaySphereTest70");
	RaySphereTest71("RaySphereTest71");
	RaySphereTest72("RaySphereTest72");
	RaySphereTest73("RaySphereTest73");
	RaySphereTest74("RaySphereTest74");
	RaySphereTest75("RaySphereTest75");
	RaySphereTest76("RaySphereTest76");
	RaySphereTest77("RaySphereTest77");
	RaySphereTest78("RaySphereTest78");
	RaySphereTest79("RaySphereTest79");
	RaySphereTest80("RaySphereTest80");
	RaySphereTest81("RaySphereTest81");
	RaySphereTest82("RaySphereTest82");
	RaySphereTest83("RaySphereTest83");
	RaySphereTest84("RaySphereTest84");
	RaySphereTest85("RaySphereTest85");

	RayAabbTest1("RayAabbTest1");
	RayAabbTest2("RayAabbTest2");
	RayAabbTest3("RayAabbTest3");
	RayAabbTest4("RayAabbTest4");
	RayAabbTest5("RayAabbTest5");
	RayAabbTest6("RayAabbTest6");
	RayAabbTest7("RayAabbTest7");
	RayAabbTest8("RayAabbTest8");
	RayAabbTest9("RayAabbTest9");
	RayAabbTest10("RayAabbTest10");
	RayAabbTest11("RayAabbTest11");
	RayAabbTest12("RayAabbTest12");
	RayAabbTest13("RayAabbTest13");
	RayAabbTest14("RayAabbTest14");
	RayAabbTest15("RayAabbTest15");
	RayAabbTest16("RayAabbTest16");
	RayAabbTest17("RayAabbTest17");
	RayAabbTest18("RayAabbTest18");
	RayAabbTest19("RayAabbTest19");
	RayAabbTest20("RayAabbTest20");
	RayAabbTest21("RayAabbTest21");
	RayAabbTest22("RayAabbTest22");
	RayAabbTest23("RayAabbTest23");
	RayAabbTest24("RayAabbTest24");
	RayAabbTest25("RayAabbTest25");
	RayAabbTest26("RayAabbTest26");
	RayAabbTest27("RayAabbTest27");
	RayAabbTest28("RayAabbTest28");
	RayAabbTest29("RayAabbTest29");
	RayAabbTest30("RayAabbTest30");
	RayAabbTest31("RayAabbTest31");
	RayAabbTest32("RayAabbTest32");
	RayAabbTest33("RayAabbTest33");
	RayAabbTest34("RayAabbTest34");
	RayAabbTest35("RayAabbTest35");
	RayAabbTest36("RayAabbTest36");
	RayAabbTest37("RayAabbTest37");
	RayAabbTest38("RayAabbTest38");
	RayAabbTest39("RayAabbTest39");
	RayAabbTest40("RayAabbTest40");
	RayAabbTest41("RayAabbTest41");
	RayAabbTest42("RayAabbTest42");
	RayAabbTest43("RayAabbTest43");
	RayAabbTest44("RayAabbTest44");
	RayAabbTest45("RayAabbTest45");
	RayAabbTest46("RayAabbTest46");
	RayAabbTest47("RayAabbTest47");
	RayAabbTest48("RayAabbTest48");
	RayAabbTest49("RayAabbTest49");
	RayAabbTest50("RayAabbTest50");
	RayAabbTest51("RayAabbTest51");
	RayAabbTest52("RayAabbTest52");
	RayAabbTest53("RayAabbTest53");
	RayAabbTest54("RayAabbTest54");
	RayAabbTest55("RayAabbTest55");
	RayAabbTest56("RayAabbTest56");
	RayAabbTest57("RayAabbTest57");
	RayAabbTest58("RayAabbTest58");
	RayAabbTest59("RayAabbTest59");
	RayAabbTest60("RayAabbTest60");
	RayAabbTest61("RayAabbTest61");
	RayAabbTest62("RayAabbTest62");
	RayAabbTest63("RayAabbTest63");
	RayAabbTest64("RayAabbTest64");
	RayAabbTest65("RayAabbTest65");
	RayAabbTest66("RayAabbTest66");
	RayAabbTest67("RayAabbTest67");
	RayAabbTest68("RayAabbTest68");
	RayAabbTest69("RayAabbTest69");
	RayAabbTest70("RayAabbTest70");
	RayAabbTest71("RayAabbTest71");
	RayAabbTest72("RayAabbTest72");
	RayAabbTest73("RayAabbTest73");
	RayAabbTest74("RayAabbTest74");
	RayAabbTest75("RayAabbTest75");
	RayAabbTest76("RayAabbTest76");
	RayAabbTest77("RayAabbTest77");
	RayAabbTest78("RayAabbTest78");
	RayAabbTest79("RayAabbTest79");
	RayAabbTest80("RayAabbTest80");
	RayAabbTest81("RayAabbTest81");
	RayAabbTest82("RayAabbTest82");
	RayAabbTest83("RayAabbTest83");
	RayAabbTest84("RayAabbTest84");
	RayAabbTest85("RayAabbTest85");
	RayAabbTest86("RayAabbTest86");
	RayAabbTest87("RayAabbTest87");
	RayAabbTest88("RayAabbTest88");
	RayAabbTest89("RayAabbTest89");
	RayAabbTest90("RayAabbTest90");
	RayAabbTest91("RayAabbTest91");

	PlaneSphereTest1("PlaneSphereTest1");
	PlaneSphereTest2("PlaneSphereTest2");
	PlaneSphereTest3("PlaneSphereTest3");
	PlaneSphereTest4("PlaneSphereTest4");
	PlaneSphereTest5("PlaneSphereTest5");
	PlaneSphereTest6("PlaneSphereTest6");
	PlaneSphereTest7("PlaneSphereTest7");
	PlaneSphereTest8("PlaneSphereTest8");
	PlaneSphereTest9("PlaneSphereTest9");
	PlaneSphereTest10("PlaneSphereTest10");
	PlaneSphereTest11("PlaneSphereTest11");
	PlaneSphereTest12("PlaneSphereTest12");
	PlaneSphereTest13("PlaneSphereTest13");
	PlaneSphereTest14("PlaneSphereTest14");
	PlaneSphereTest15("PlaneSphereTest15");
	PlaneSphereTest16("PlaneSphereTest16");
	PlaneSphereTest17("PlaneSphereTest17");
	PlaneSphereTest18("PlaneSphereTest18");
	PlaneSphereTest19("PlaneSphereTest19");
	PlaneSphereTest20("PlaneSphereTest20");

	PlaneAabbTest1("PlaneAabbTest1");
	PlaneAabbTest2("PlaneAabbTest2");
	PlaneAabbTest3("PlaneAabbTest3");
	PlaneAabbTest4("PlaneAabbTest4");
	PlaneAabbTest5("PlaneAabbTest5");
	PlaneAabbTest6("PlaneAabbTest6");
	PlaneAabbTest7("PlaneAabbTest7");
	PlaneAabbTest8("PlaneAabbTest8");
	PlaneAabbTest9("PlaneAabbTest9");
	PlaneAabbTest10("PlaneAabbTest10");
	PlaneAabbTest11("PlaneAabbTest11");
	PlaneAabbTest12("PlaneAabbTest12");
	PlaneAabbTest13("PlaneAabbTest13");
	PlaneAabbTest14("PlaneAabbTest14");
	PlaneAabbTest15("PlaneAabbTest15");
	PlaneAabbTest16("PlaneAabbTest16");
	PlaneAabbTest17("PlaneAabbTest17");
	PlaneAabbTest18("PlaneAabbTest18");
	PlaneAabbTest19("PlaneAabbTest19");
	PlaneAabbTest20("PlaneAabbTest20");
	PlaneAabbTest21("PlaneAabbTest21");
	PlaneAabbTest22("PlaneAabbTest22");
	PlaneAabbTest23("PlaneAabbTest23");
	PlaneAabbTest24("PlaneAabbTest24");
	PlaneAabbTest25("PlaneAabbTest25");
	PlaneAabbTest26("PlaneAabbTest26");
	PlaneAabbTest27("PlaneAabbTest27");
	PlaneAabbTest28("PlaneAabbTest28");
	PlaneAabbTest29("PlaneAabbTest29");
	PlaneAabbTest30("PlaneAabbTest30");
	PlaneAabbTest31("PlaneAabbTest31");
	PlaneAabbTest32("PlaneAabbTest32");
	PlaneAabbTest33("PlaneAabbTest33");
	PlaneAabbTest34("PlaneAabbTest34");
	PlaneAabbTest35("PlaneAabbTest35");
	PlaneAabbTest36("PlaneAabbTest36");
	PlaneAabbTest37("PlaneAabbTest37");
	PlaneAabbTest38("PlaneAabbTest38");
	PlaneAabbTest39("PlaneAabbTest39");
	PlaneAabbTest40("PlaneAabbTest40");
	PlaneAabbTest41("PlaneAabbTest41");
	PlaneAabbTest42("PlaneAabbTest42");
	PlaneAabbTest43("PlaneAabbTest43");
	PlaneAabbTest44("PlaneAabbTest44");
	PlaneAabbTest45("PlaneAabbTest45");
	PlaneAabbTest46("PlaneAabbTest46");
	PlaneAabbTest47("PlaneAabbTest47");
	PlaneAabbTest48("PlaneAabbTest48");
	PlaneAabbTest49("PlaneAabbTest49");
	PlaneAabbTest50("PlaneAabbTest50");

	return 1;
}



void PrintTestHeader(const std::string& testName)
{
	printf("\n");
	printf("////////////////////////////////////////////////////////////\n");
	printf("%s\n", testName.c_str());
	printf("////////////////////////////////////////////////////////////\n");
}

#pragma region Sphere

/** Sphere Vs Sphere -- 15 tests **/


	// Spheres separating
	void SphereSphereTest1(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(-2, 0, 0), 1);
		Sphere sphere1(Vector3(2, 0, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));		
	}

	// Spheres separating
	void SphereSphereTest2(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(-1.5f, 0, 0), 1);
		Sphere sphere1(Vector3(1.5f, 0, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres just touching
	void SphereSphereTest3(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(-1, 0, 0), 1);
		Sphere sphere1(Vector3(1, 0, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres overlapping a bit
	void SphereSphereTest4(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(-0.5f, 0, 0), 1);
		Sphere sphere1(Vector3(0.5f, 0, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres at the same center
	void SphereSphereTest5(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, 0, 0), 1);
		Sphere sphere1(Vector3(0, 0, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres separating
	void SphereSphereTest6(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, -2, 0), 1);
		Sphere sphere1(Vector3(0, 2, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres separating
	void SphereSphereTest7(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, -1.5f, 0), 1);
		Sphere sphere1(Vector3(0, 1.5f, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres just touching
	void SphereSphereTest8(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, -1, 0), 1);
		Sphere sphere1(Vector3(0, 1, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres overlapping a bit
	void SphereSphereTest9(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, -0.5f, 0), 1);
		Sphere sphere1(Vector3(0, 0.5f, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres at the same center
	void SphereSphereTest10(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, 0, 0), 1);
		Sphere sphere1(Vector3(0, 0, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres separating
	void SphereSphereTest11(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, 0, -2), 1);
		Sphere sphere1(Vector3(0, 0, 2), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres separating
	void SphereSphereTest12(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, 0, -1.5f), 1);
		Sphere sphere1(Vector3(0, 0, 1.5f), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres just touching
	void SphereSphereTest13(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, 0, -1), 1);
		Sphere sphere1(Vector3(0, 0, 1), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres overlapping a bit
	void SphereSphereTest14(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, 0, -0.5f), 1);
		Sphere sphere1(Vector3(0, 0, 0.5f), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

	// Spheres at the same center
	void SphereSphereTest15(const std::string & testName)
	{
		PrintTestHeader(testName);

		Sphere sphere0(Vector3(0, 0, 0), 1);
		Sphere sphere1(Vector3(0, 0, 0), 1);

		PrintResult(TestSphereSphere(sphere0, sphere1));
	}

#pragma endregion

#pragma region AABBAABB


/** AABB Vs AABB : 84 tests **/

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(0.5f, -0.5f, -0.5f), Vector3(1.5f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(0.5f, -0.5f, -0.5f), Vector3(1.5f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-1.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1, -0.5f, -0.5f), Vector3(0, 0.5f, 0.5f));
		Aabb aabb1(Vector3(0, -0.5f, -0.5f), Vector3(1, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(0, -0.5f, -0.5f), Vector3(1, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-1, -0.5f, -0.5f), Vector3(0, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.75f, -0.5f, -0.5f), Vector3(0.25f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.25f, -0.5f, -0.5f), Vector3(0.75f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.25f, -0.5f, -0.5f), Vector3(0.75f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.75f, -0.5f, -0.5f), Vector3(0.25f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -1.5f, -0.5f), Vector3(0.5f, -0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.5f, 1.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.5f, 1.5f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -1.5f, -0.5f), Vector3(0.5f, -0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -1, -0.5f), Vector3(0.5f, 0, 0.5f));
		Aabb aabb1(Vector3(-0.5f, 0, -0.5f), Vector3(0.5f, 1, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, 0, -0.5f), Vector3(0.5f, 1, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -1, -0.5f), Vector3(0.5f, 0, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.75f, -0.5f), Vector3(0.5f, 0.25f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.25f, -0.5f), Vector3(0.5f, 0.75f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.25f, -0.5f), Vector3(0.5f, 0.75f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.75f, -0.5f), Vector3(0.5f, 0.25f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -1.5f), Vector3(0.5f, 0.5f, -0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.5f, 0.5f, 1.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.5f, 0.5f, 1.5f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -1.5f), Vector3(0.5f, 0.5f, -0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -1), Vector3(0.5f, 0.5f, 0));
		Aabb aabb1(Vector3(-0.5f, -0.5f, 0), Vector3(0.5f, 0.5f, 1));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, 0), Vector3(0.5f, 0.5f, 1));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -1), Vector3(0.5f, 0.5f, 0));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.75f), Vector3(0.5f, 0.5f, 0.25f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.25f), Vector3(0.5f, 0.5f, 0.75f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.25f), Vector3(0.5f, 0.5f, 0.75f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.75f), Vector3(0.5f, 0.5f, 0.25f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1.5f, -1.5f, -0.5f), Vector3(-0.5f, -0.5f, 0.5f));
		Aabb aabb1(Vector3(0.5f, 0.5f, -0.5f), Vector3(1.5f, 1.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(0.5f, 0.5f, -0.5f), Vector3(1.5f, 1.5f, 0.5f));
		Aabb aabb1(Vector3(-1.5f, -1.5f, -0.5f), Vector3(-0.5f, -0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1, -1, -0.5f), Vector3(2.98023e-08f, 2.98023e-08f, 0.5f));
		Aabb aabb1(Vector3(-2.98023e-08f, -2.98023e-08f, -0.5f), Vector3(1, 1, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-2.98023e-08f, -2.98023e-08f, -0.5f), Vector3(1, 1, 0.5f));
		Aabb aabb1(Vector3(-1, -1, -0.5f), Vector3(2.98023e-08f, 2.98023e-08f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.75f, -0.75f, -0.5f), Vector3(0.25f, 0.25f, 0.5f));
		Aabb aabb1(Vector3(-0.25f, -0.25f, -0.5f), Vector3(0.75f, 0.75f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.25f, -0.25f, -0.5f), Vector3(0.75f, 0.75f, 0.5f));
		Aabb aabb1(Vector3(-0.75f, -0.75f, -0.5f), Vector3(0.25f, 0.25f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1.5f, -0.5f, -1.5f), Vector3(-0.5f, 0.5f, -0.5f));
		Aabb aabb1(Vector3(0.5f, -0.5f, 0.5f), Vector3(1.5f, 0.5f, 1.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(0.5f, -0.5f, 0.5f), Vector3(1.5f, 0.5f, 1.5f));
		Aabb aabb1(Vector3(-1.5f, -0.5f, -1.5f), Vector3(-0.5f, 0.5f, -0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest31(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1, -0.5f, -1), Vector3(2.98023e-08f, 0.5f, 2.98023e-08f));
		Aabb aabb1(Vector3(-2.98023e-08f, -0.5f, -2.98023e-08f), Vector3(1, 0.5f, 1));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest32(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-2.98023e-08f, -0.5f, -2.98023e-08f), Vector3(1, 0.5f, 1));
		Aabb aabb1(Vector3(-1, -0.5f, -1), Vector3(2.98023e-08f, 0.5f, 2.98023e-08f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest33(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.75f, -0.5f, -0.75f), Vector3(0.25f, 0.5f, 0.25f));
		Aabb aabb1(Vector3(-0.25f, -0.5f, -0.25f), Vector3(0.75f, 0.5f, 0.75f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest34(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.25f, -0.5f, -0.25f), Vector3(0.75f, 0.5f, 0.75f));
		Aabb aabb1(Vector3(-0.75f, -0.5f, -0.75f), Vector3(0.25f, 0.5f, 0.25f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest35(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest36(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -1.5f, -1.5f), Vector3(0.5f, -0.5f, -0.5f));
		Aabb aabb1(Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.5f, 1.5f, 1.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest37(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.5f, 1.5f, 1.5f));
		Aabb aabb1(Vector3(-0.5f, -1.5f, -1.5f), Vector3(0.5f, -0.5f, -0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest38(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -1, -1), Vector3(0.5f, 2.98023e-08f, 2.98023e-08f));
		Aabb aabb1(Vector3(-0.5f, -2.98023e-08f, -2.98023e-08f), Vector3(0.5f, 1, 1));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest39(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -2.98023e-08f, -2.98023e-08f), Vector3(0.5f, 1, 1));
		Aabb aabb1(Vector3(-0.5f, -1, -1), Vector3(0.5f, 2.98023e-08f, 2.98023e-08f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest40(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.75f, -0.75f), Vector3(0.5f, 0.25f, 0.25f));
		Aabb aabb1(Vector3(-0.5f, -0.25f, -0.25f), Vector3(0.5f, 0.75f, 0.75f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest41(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.25f, -0.25f), Vector3(0.5f, 0.75f, 0.75f));
		Aabb aabb1(Vector3(-0.5f, -0.75f, -0.75f), Vector3(0.5f, 0.25f, 0.25f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest42(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest43(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1.5f, -0.5f, -0.1f), Vector3(-0.5f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(0.5f, -0.5f, -0.1f), Vector3(1.5f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest44(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(0.5f, -0.5f, -0.1f), Vector3(1.5f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-1.5f, -0.5f, -0.1f), Vector3(-0.5f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest45(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1, -0.5f, -0.1f), Vector3(0, 0.5f, 0.1f));
		Aabb aabb1(Vector3(0, -0.5f, -0.1f), Vector3(1, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest46(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(0, -0.5f, -0.1f), Vector3(1, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-1, -0.5f, -0.1f), Vector3(0, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest47(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.75f, -0.5f, -0.1f), Vector3(0.25f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.25f, -0.5f, -0.1f), Vector3(0.75f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest48(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.25f, -0.5f, -0.1f), Vector3(0.75f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.75f, -0.5f, -0.1f), Vector3(0.25f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest49(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest50(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -1.5f, -0.1f), Vector3(0.5f, -0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, 0.5f, -0.1f), Vector3(0.5f, 1.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest51(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, 0.5f, -0.1f), Vector3(0.5f, 1.5f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -1.5f, -0.1f), Vector3(0.5f, -0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest52(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -1, -0.1f), Vector3(0.5f, 0, 0.1f));
		Aabb aabb1(Vector3(-0.5f, 0, -0.1f), Vector3(0.5f, 1, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest53(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, 0, -0.1f), Vector3(0.5f, 1, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -1, -0.1f), Vector3(0.5f, 0, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest54(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.75f, -0.1f), Vector3(0.5f, 0.25f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.25f, -0.1f), Vector3(0.5f, 0.75f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest55(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.25f, -0.1f), Vector3(0.5f, 0.75f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.75f, -0.1f), Vector3(0.5f, 0.25f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest56(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest57(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.3f), Vector3(0.5f, 0.5f, -0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, 0.1f), Vector3(0.5f, 0.5f, 0.3f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest58(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, 0.1f), Vector3(0.5f, 0.5f, 0.3f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.3f), Vector3(0.5f, 0.5f, -0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest59(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.2f), Vector3(0.5f, 0.5f, 0));
		Aabb aabb1(Vector3(-0.5f, -0.5f, 0), Vector3(0.5f, 0.5f, 0.2f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest60(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, 0), Vector3(0.5f, 0.5f, 0.2f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.2f), Vector3(0.5f, 0.5f, 0));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest61(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.15f), Vector3(0.5f, 0.5f, 0.05f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.05f), Vector3(0.5f, 0.5f, 0.15f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest62(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.05f), Vector3(0.5f, 0.5f, 0.15f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.15f), Vector3(0.5f, 0.5f, 0.05f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest63(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest64(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1.5f, -1.5f, -0.1f), Vector3(-0.5f, -0.5f, 0.1f));
		Aabb aabb1(Vector3(0.5f, 0.5f, -0.1f), Vector3(1.5f, 1.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest65(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(0.5f, 0.5f, -0.1f), Vector3(1.5f, 1.5f, 0.1f));
		Aabb aabb1(Vector3(-1.5f, -1.5f, -0.1f), Vector3(-0.5f, -0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest66(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1, -1, -0.1f), Vector3(2.98023e-08f, 2.98023e-08f, 0.1f));
		Aabb aabb1(Vector3(-2.98023e-08f, -2.98023e-08f, -0.1f), Vector3(1, 1, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest67(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-2.98023e-08f, -2.98023e-08f, -0.1f), Vector3(1, 1, 0.1f));
		Aabb aabb1(Vector3(-1, -1, -0.1f), Vector3(2.98023e-08f, 2.98023e-08f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest68(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.75f, -0.75f, -0.1f), Vector3(0.25f, 0.25f, 0.1f));
		Aabb aabb1(Vector3(-0.25f, -0.25f, -0.1f), Vector3(0.75f, 0.75f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest69(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.25f, -0.25f, -0.1f), Vector3(0.75f, 0.75f, 0.1f));
		Aabb aabb1(Vector3(-0.75f, -0.75f, -0.1f), Vector3(0.25f, 0.25f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest70(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest71(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-1.1f, -0.5f, -0.7f), Vector3(-0.1f, 0.5f, -0.5f));
		Aabb aabb1(Vector3(0.1f, -0.5f, 0.5f), Vector3(1.1f, 0.5f, 0.7f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest72(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(0.1f, -0.5f, 0.5f), Vector3(1.1f, 0.5f, 0.7f));
		Aabb aabb1(Vector3(-1.1f, -0.5f, -0.7f), Vector3(-0.1f, 0.5f, -0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest73(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.8f, -0.5f, -0.4f), Vector3(0.2f, 0.5f, -0.2f));
		Aabb aabb1(Vector3(-0.2f, -0.5f, 0.2f), Vector3(0.8f, 0.5f, 0.4f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest74(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.2f, -0.5f, 0.2f), Vector3(0.8f, 0.5f, 0.4f));
		Aabb aabb1(Vector3(-0.8f, -0.5f, -0.4f), Vector3(0.2f, 0.5f, -0.2f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest75(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.65f, -0.5f, -0.25f), Vector3(0.35f, 0.5f, -0.05f));
		Aabb aabb1(Vector3(-0.35f, -0.5f, 0.05f), Vector3(0.65f, 0.5f, 0.25f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest76(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.35f, -0.5f, 0.05f), Vector3(0.65f, 0.5f, 0.25f));
		Aabb aabb1(Vector3(-0.65f, -0.5f, -0.25f), Vector3(0.35f, 0.5f, -0.05f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest77(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb0 behind aabb1)
	void AabbAabbTest78(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -1.1f, -0.7f), Vector3(0.5f, -0.1f, -0.5f));
		Aabb aabb1(Vector3(-0.5f, 0.1f, 0.5f), Vector3(0.5f, 1.1f, 0.7f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs separating (with aabb1 behind aabb0)
	void AabbAabbTest79(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, 0.1f, 0.5f), Vector3(0.5f, 1.1f, 0.7f));
		Aabb aabb1(Vector3(-0.5f, -1.1f, -0.7f), Vector3(0.5f, -0.1f, -0.5f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb0 behind aabb1)
	void AabbAabbTest80(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.8f, -0.4f), Vector3(0.5f, 0.2f, -0.2f));
		Aabb aabb1(Vector3(-0.5f, -0.2f, 0.2f), Vector3(0.5f, 0.8f, 0.4f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs just touching (with aabb1 behind aabb0)
	void AabbAabbTest81(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.2f, 0.2f), Vector3(0.5f, 0.8f, 0.4f));
		Aabb aabb1(Vector3(-0.5f, -0.8f, -0.4f), Vector3(0.5f, 0.2f, -0.2f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb0 behind aabb1)
	void AabbAabbTest82(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.65f, -0.25f), Vector3(0.5f, 0.35f, -0.05f));
		Aabb aabb1(Vector3(-0.5f, -0.35f, 0.05f), Vector3(0.5f, 0.65f, 0.25f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs overlapping (with aabb1 behind aabb0)
	void AabbAabbTest83(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.35f, 0.05f), Vector3(0.5f, 0.65f, 0.25f));
		Aabb aabb1(Vector3(-0.5f, -0.65f, -0.25f), Vector3(0.5f, 0.35f, -0.05f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}

	// Aabbs at the same center
	void AabbAabbTest84(const std::string& testName)
	{
		PrintTestHeader(testName);

		Aabb aabb0(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));
		Aabb aabb1(Vector3(-0.5f, -0.5f, -0.1f), Vector3(0.5f, 0.5f, 0.1f));

		PrintResult(TestAabbAabb(aabb0, aabb1));
	}
#pragma endregion

#pragma region PointPlane


/** Point Vs Plane - 40 tests **/

	// In front of plane
	void PointPlaneTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, 1, 2);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane
	void PointPlaneTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(-1, 0, 2);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind Plane
	void PointPlaneTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, 1, 0);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind Plane
	void PointPlaneTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(-1, 0, 0);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Directly on Plane
	void PointPlaneTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, 1, 1);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Directly on Plane
	void PointPlaneTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(-1, 0, 1);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane within epsilon
	void PointPlaneTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, 1, 1.25f);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane within epsilon
	void PointPlaneTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, 1, 0.75f);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane
	void PointPlaneTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, -1, 2);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane
	void PointPlaneTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, 0, 2);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind Plane
	void PointPlaneTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, -1, 0);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind Plane
	void PointPlaneTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, 0, 0);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Directly on Plane
	void PointPlaneTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, -1, 1);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Directly on Plane
	void PointPlaneTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, 0, 1);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind of plane within epsilon
	void PointPlaneTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, -1, 1.25f);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind of plane within epsilon
	void PointPlaneTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0, -1, 0.75f);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane within epsilon
	void PointPlaneTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(-1, 0, 1.25f);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane within epsilon
	void PointPlaneTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(-1, 0, 0.75f);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind of plane within epsilon
	void PointPlaneTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, 0, 1.25f);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind of plane within epsilon
	void PointPlaneTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, 0, 0.75f);
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 1));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane
	void PointPlaneTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, 2.41421f, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane
	void PointPlaneTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1.70711f, 1.70711f, 1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind Plane
	void PointPlaneTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(-0.414214f, 1, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind Plane
	void PointPlaneTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0.292893f, 0.292893f, 1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Directly on Plane
	void PointPlaneTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0.292893f, 1.70711f, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Directly on Plane
	void PointPlaneTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, 1, 1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane within epsilon
	void PointPlaneTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0.46967f, 1.88388f, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane within epsilon
	void PointPlaneTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0.116116f, 1.53033f, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane
	void PointPlaneTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(2.41421f, 1, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane
	void PointPlaneTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1.70711f, 1.70711f, -1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind Plane
	void PointPlaneTest31(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, -0.414214f, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind Plane
	void PointPlaneTest32(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0.292893f, 0.292893f, -1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Directly on Plane
	void PointPlaneTest33(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1.70711f, 0.292893f, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Directly on Plane
	void PointPlaneTest34(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1, 1, -1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind of plane within epsilon
	void PointPlaneTest35(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1.88388f, 0.46967f, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind of plane within epsilon
	void PointPlaneTest36(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1.53033f, 0.116116f, 0);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane within epsilon
	void PointPlaneTest37(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1.17678f, 1.17678f, 1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// In front of plane within epsilon
	void PointPlaneTest38(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0.823223f, 0.823223f, 1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind of plane within epsilon
	void PointPlaneTest39(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(1.17678f, 1.17678f, -1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

	// Behind of plane within epsilon
	void PointPlaneTest40(const std::string& testName)
	{
		PrintTestHeader(testName);

		float epsilon = 0.5f;
		Vector3 point = Vector3(0.823223f, 0.823223f, -1);
		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(1, 1, 0));

		PrintResultPlane(TestPointPlane(point, plane),epsilon);
	}

#pragma endregion

#pragma region PointSphere


/** Point Vs Sphere : 26 tests **/


	void PointSphereTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(4, 2, 3);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(2.5f, 2, 3);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.75f, 2, 3);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 5, 3);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 3.5f, 3);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2.75f, 3);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 6);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 4.5f);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 3.75f);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(2.73205f, 3.73205f, 4.73205f);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.86603f, 2.86603f, 3.86603f);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.43301f, 2.43301f, 3.43301f);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 3);
		Sphere sphere(Vector3(1, 2, 3), 1.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 0, 0);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.5f, 0, 0);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.25f, 0, 0);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 1, 0);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0.5f, 0);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0.25f, 0);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, 1);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, 0.5f);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, 0.25f);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.57735f, 0.57735f, 0.57735f);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.288675f, 0.288675f, 0.288675f);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.144338f, 0.144338f, 0.144338f);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}

	void PointSphereTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, 0);
		Sphere sphere(Vector3(0, 0, 0), 0.5f);

		PrintResult(TestPointSphere(point, sphere));
	}


#pragma endregion

#pragma region PointAABB
/** Point Vs AABB : 84 tests  **/


	void PointAabbTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 0, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.5f, 0, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.25f, 0, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.25f, 0, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.5f, 0, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-1, 0, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 1, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0.5f, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0.25f, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, -0.25f, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, -0.5f, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, -1, 0);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, 1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, 0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, 0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, -0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, -0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0, 0, -1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 1, 1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.5f, 0.5f, 0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.25f, 0.25f, 0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.25f, -0.25f, -0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.5f, -0.5f, -0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-1, -1, -1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 1, -1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.5f, 0.5f, -0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.25f, 0.25f, -0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.25f, -0.25f, 0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.5f, -0.5f, 0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-1, -1, 1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest31(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, -1, -1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest32(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.5f, -0.5f, -0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest33(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.25f, -0.25f, -0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest34(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.25f, 0.25f, 0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest35(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.5f, 0.5f, 0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest36(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-1, 1, 1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest37(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, -1, 1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest38(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.5f, -0.5f, 0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest39(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.25f, -0.25f, 0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest40(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.25f, 0.25f, -0.25f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest41(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-0.5f, 0.5f, -0.5f);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest42(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(-1, 1, -1);
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest43(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.2f, 2, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest44(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.1f, 2, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest45(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.05f, 2, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest46(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.95f, 2, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest47(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.9f, 2, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest48(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.8f, 2, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest49(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 6, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest50(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 4, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest51(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 3, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest52(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 1, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest53(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 0, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest54(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, -2, 3);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest55(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 4.4f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest56(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 3.7f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest57(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 3.35f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest58(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 2.65f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest59(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 2.3f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest60(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1, 2, 1.6f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest61(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.2f, 6, 4.4f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest62(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.1f, 4, 3.7f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest63(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.05f, 3, 3.35f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest64(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.95f, 1, 2.65f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest65(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.9f, 0, 2.3f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest66(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.8f, -2, 1.6f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest67(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.2f, 6, 1.6f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest68(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.1f, 4, 2.3f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest69(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.05f, 3, 2.65f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest70(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.95f, 1, 3.35f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest71(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.9f, 0, 3.7f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest72(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.8f, -2, 4.4f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest73(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.2f, -2, 1.6f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest74(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.1f, 0, 2.3f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest75(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.05f, 1, 2.65f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest76(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.95f, 3, 3.35f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest77(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.9f, 4, 3.7f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest78(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.8f, 6, 4.4f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest79(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.2f, -2, 4.4f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest80(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.1f, 0, 3.7f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest81(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(1.05f, 1, 3.35f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest82(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.95f, 3, 2.65f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest83(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.9f, 4, 2.3f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

	void PointAabbTest84(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 point = Vector3(0.8f, 6, 1.6f);
		Aabb aabb(Vector3(0.9f, 0, 2.3f), Vector3(1.1f, 4, 3.7f));

		PrintResult(TestPointAabb(point, aabb));
	}

#pragma endregion

#pragma region BarycentricTriangle

/** Point Vs Triangle : 64 tests **/

	void BarycentricTriangleTest0(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.33f;
		float v = 0.33f;
		float w = 0.33f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Halfway in-between p0 and p1
	void BarycentricTriangleTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.5f;
		float v = 0.5f;
		float w = 0;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Halfway in-between p0 and p2
	void BarycentricTriangleTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.5f;
		float v = 0;
		float w = 0.5f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Halfway in-between p1 and p2
	void BarycentricTriangleTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0;
		float v = 0.5f;
		float w = 0.5f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Coordinate at p0
	void BarycentricTriangleTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 1;
		float v = 0;
		float w = 0;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Coordinate at p1
	void BarycentricTriangleTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0;
		float v = 1;
		float w = 0;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Coordinate at p2
	void BarycentricTriangleTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0;
		float v = 0;
		float w = 1;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.25f;
		float v = 0.5f;
		float w = 0.25f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.5f;
		float v = 0.25f;
		float w = 0.25f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.25f;
		float v = 0.25f;
		float w = 0.5f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.25f;
		float v = 0.25f;
		float w = 0.5f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.5f;
		float v = 0.25f;
		float w = 0.25f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.25f;
		float v = 0.5f;
		float w = 0.25f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.1f;
		float v = 0.3f;
		float w = 0.6f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.3f;
		float v = 0.6f;
		float w = 0.1f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.6f;
		float v = 0.1f;
		float w = 0.3f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.1f;
		float v = 0.6f;
		float w = 0.3f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.3f;
		float v = 0.1f;
		float w = 0.6f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.6f;
		float v = 0.3f;
		float w = 0.1f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = -0.1f;
		float v = 0.45f;
		float w = 0.65f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.45f;
		float v = 0.65f;
		float w = -0.1f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.65f;
		float v = -0.1f;
		float w = 0.45f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = -0.1f;
		float v = 0.65f;
		float w = 0.45f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.45f;
		float v = -0.1f;
		float w = 0.65f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 0.65f;
		float v = 0.45f;
		float w = -0.1f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = -0.15f;
		float v = -0.05f;
		float w = 1.2f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = -0.05f;
		float v = 1.2f;
		float w = -0.15f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 1.2f;
		float v = -0.15f;
		float w = -0.05f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = -0.15f;
		float v = 1.2f;
		float w = -0.05f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = -0.05f;
		float v = -0.15f;
		float w = 1.2f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(0, 0, 0);
		Vector3 p1 = Vector3(1, 0, 0);
		Vector3 p2 = Vector3(0, 1, 0);
		float u = 1.2f;
		float v = -0.05f;
		float w = -0.15f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest31(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.33f;
		float v = 0.33f;
		float w = 0.33f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Halfway in-between p0 and p1
	void BarycentricTriangleTest32(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.5f;
		float v = 0.5f;
		float w = 0;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Halfway in-between p0 and p2
	void BarycentricTriangleTest33(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.5f;
		float v = 0;
		float w = 0.5f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Halfway in-between p1 and p2
	void BarycentricTriangleTest34(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0;
		float v = 0.5f;
		float w = 0.5f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Coordinate at p0
	void BarycentricTriangleTest35(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 1;
		float v = 0;
		float w = 0;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Coordinate at p1
	void BarycentricTriangleTest36(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0;
		float v = 1;
		float w = 0;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Coordinate at p2
	void BarycentricTriangleTest37(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0;
		float v = 0;
		float w = 1;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest38(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.25f;
		float v = 0.5f;
		float w = 0.25f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest39(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.5f;
		float v = 0.25f;
		float w = 0.25f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest40(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.25f;
		float v = 0.25f;
		float w = 0.5f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest41(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.25f;
		float v = 0.25f;
		float w = 0.5f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest42(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.5f;
		float v = 0.25f;
		float w = 0.25f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest43(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.25f;
		float v = 0.5f;
		float w = 0.25f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest44(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.1f;
		float v = 0.3f;
		float w = 0.6f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest45(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.3f;
		float v = 0.6f;
		float w = 0.1f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest46(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.6f;
		float v = 0.1f;
		float w = 0.3f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest47(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.1f;
		float v = 0.6f;
		float w = 0.3f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest48(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.3f;
		float v = 0.1f;
		float w = 0.6f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest49(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.6f;
		float v = 0.3f;
		float w = 0.1f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest50(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = -0.1f;
		float v = 0.45f;
		float w = 0.65f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest51(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.45f;
		float v = 0.65f;
		float w = -0.1f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest52(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.65f;
		float v = -0.1f;
		float w = 0.45f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest53(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = -0.1f;
		float v = 0.65f;
		float w = 0.45f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest54(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.45f;
		float v = -0.1f;
		float w = 0.65f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest55(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.65f;
		float v = 0.45f;
		float w = -0.1f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest56(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = -0.15f;
		float v = -0.05f;
		float w = 1.2f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest57(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = -0.05f;
		float v = 1.2f;
		float w = -0.15f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest58(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 1.2f;
		float v = -0.15f;
		float w = -0.05f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest59(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = -0.15f;
		float v = 1.2f;
		float w = -0.05f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest60(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = -0.05f;
		float v = -0.15f;
		float w = 1.2f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	void BarycentricTriangleTest61(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 1.2f;
		float v = -0.05f;
		float w = -0.15f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: false. Testing a degenerate triangle
	void BarycentricTriangleTest62(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(-1, -2, -3);
		Vector3 p1 = Vector3(-1, -2, -3);
		Vector3 p2 = Vector3(0, 0, 0);
		float u = 0;
		float v = 1;
		float w = 0;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Testing the use of epsilon
	void BarycentricTriangleTest63(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = -0.025f;
		float v = -0.025f;
		float w = 1.05f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

	// Expected Result: true. Testing the use of epsilon
	void BarycentricTriangleTest64(const std::string& testName)
	{
		PrintTestHeader(testName);

		Vector3 p0 = Vector3(1, 0, -1);
		Vector3 p1 = Vector3(1, 0, 1);
		Vector3 p2 = Vector3(-1, 0, 0);
		float u = 0.52f;
		float v = 0.52f;
		float w = -0.04f;

		PrintResultUVW(TestBarycentricTriangle(p0, p1, p2, u, v, w),u, v, w);
	}

#pragma endregion

#pragma region RayPlane
/** Ray Vs Plane : 72 tests **/

	// Ray in front of plane pointing forward
	void RayPlaneTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 1, 0), Vector3(1, 0, 0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 1, 0), Vector3(1, 0, 0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 1, 0), Vector3(-1, -0, -0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 1, 0), Vector3(-1, -0, -0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 1, 0), Vector3(-0, 1, 0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 1, 0), Vector3(-0, 1, 0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 1, 0), Vector3(0, -0, 1));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 1, 0), Vector3(0, -0, 1));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing forward
	void RayPlaneTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -1), Vector3(0, 1, 0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -1, -1), Vector3(0, 1, 0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -1), Vector3(-0, -1, -0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -1, -1), Vector3(-0, -1, -0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -1), Vector3(0, 0, -1));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -1, -1), Vector3(0, 0, -1));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -1), Vector3(-1, 0, 0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -1, -1), Vector3(-1, 0, 0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing forward
	void RayPlaneTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 1), Vector3(0, 0, 1));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -1), Vector3(0, 0, 1));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 1), Vector3(-0, -0, -1));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -1), Vector3(-0, -0, -1));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 1), Vector3(0, 1, -0));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -1), Vector3(0, 1, -0));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 1), Vector3(-1, 0, 0));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -1), Vector3(-1, 0, 0));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing forward
	void RayPlaneTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3, 1, 0), Vector3(1, 0, 0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3, 1, 0), Vector3(1, 0, 0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3, 1, 0), Vector3(-1, -0, -0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3, 1, 0), Vector3(-1, -0, -0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3, 1, 0), Vector3(-0, 1, 0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3, 1, 0), Vector3(-0, 1, 0));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest31(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3, 1, 0), Vector3(0, -0, 1));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest32(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3, 1, 0), Vector3(0, -0, 1));
		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing forward
	void RayPlaneTest33(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 3, -1), Vector3(0, 1, 0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest34(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -3, -1), Vector3(0, 1, 0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest35(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 3, -1), Vector3(-0, -1, -0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest36(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -3, -1), Vector3(-0, -1, -0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest37(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 3, -1), Vector3(0, 0, -1));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest38(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -3, -1), Vector3(0, 0, -1));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest39(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 3, -1), Vector3(-1, 0, 0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest40(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -3, -1), Vector3(-1, 0, 0));
		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing forward
	void RayPlaneTest41(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 3), Vector3(0, 0, 1));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest42(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -3), Vector3(0, 0, 1));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest43(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 3), Vector3(-0, -0, -1));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest44(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -3), Vector3(-0, -0, -1));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest45(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 3), Vector3(0, 1, -0));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest46(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -3), Vector3(0, 1, -0));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest47(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 3), Vector3(-1, 0, 0));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest48(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, -3), Vector3(-1, 0, 0));
		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing forward
	void RayPlaneTest49(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.29289f, 3.70711f, 3), Vector3(1, 1, 1));
		Plane plane(Vector3(1, 1, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest50(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3.70711f, -2.29289f, -3), Vector3(1, 1, 1));
		Plane plane(Vector3(1, 1, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest51(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.29289f, 3.70711f, 3), Vector3(-1, -1, -1));
		Plane plane(Vector3(1, 1, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest52(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3.70711f, -2.29289f, -3), Vector3(-1, -1, -1));
		Plane plane(Vector3(1, 1, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest53(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.29289f, 3.70711f, 3), Vector3(-0.707107f, 0.707107f, 0));
		Plane plane(Vector3(1, 1, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest54(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3.70711f, -2.29289f, -3), Vector3(-0.707107f, 0.707107f, 0));
		Plane plane(Vector3(1, 1, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest55(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.29289f, 3.70711f, 3), Vector3(-0.408248f, -0.408248f, 0.816497f));
		Plane plane(Vector3(1, 1, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest56(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3.70711f, -2.29289f, -3), Vector3(-0.408248f, -0.408248f, 0.816497f));
		Plane plane(Vector3(1, 1, 1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing forward
	void RayPlaneTest57(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2.29289f, -3.70711f, -3), Vector3(-1, -1, -1));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest58(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.70711f, 2.29289f, 3), Vector3(-1, -1, -1));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest59(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2.29289f, -3.70711f, -3), Vector3(1, 1, 1));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest60(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.70711f, 2.29289f, 3), Vector3(1, 1, 1));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest61(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2.29289f, -3.70711f, -3), Vector3(0.707107f, -0.707107f, 0));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest62(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.70711f, 2.29289f, 3), Vector3(0.707107f, -0.707107f, 0));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest63(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2.29289f, -3.70711f, -3), Vector3(-0.408248f, -0.408248f, 0.816497f));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest64(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.70711f, 2.29289f, 3), Vector3(-0.408248f, -0.408248f, 0.816497f));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 0, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing forward
	void RayPlaneTest65(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2.29289f, -2.70711f, -3), Vector3(-1, -1, -1));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 1, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing forward
	void RayPlaneTest66(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.70711f, 3.29289f, 3), Vector3(-1, -1, -1));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 1, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray in front of plane pointing backwards
	void RayPlaneTest67(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2.29289f, -2.70711f, -3), Vector3(1, 1, 1));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 1, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray behind plane pointing backwards
	void RayPlaneTest68(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.70711f, 3.29289f, 3), Vector3(1, 1, 1));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 1, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest69(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2.29289f, -2.70711f, -3), Vector3(0.707107f, -0.707107f, 0));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 1, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest70(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.70711f, 3.29289f, 3), Vector3(0.707107f, -0.707107f, 0));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 1, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest71(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2.29289f, -2.70711f, -3), Vector3(-0.408248f, -0.408248f, 0.816497f));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 1, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

	// Ray parallel to plane
	void RayPlaneTest72(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.70711f, 3.29289f, 3), Vector3(-0.408248f, -0.408248f, 0.816497f));
		Plane plane(Vector3(-1, -1, -1), Vector3(0, 1, 0));

		float t{FLT_MAX};
		PrintResultRay(TestRayPlane(ray, plane,t),t);
	}

#pragma endregion

#pragma region RayTriangle
/** Ray Vs Triangle : 30 tests **/

	// Ray behind triangle pointing forwards
	void RayTriangleTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -1, 0), Vector3(0, 1, 0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind triangle pointing backwards
	void RayTriangleTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -1, 0), Vector3(-0, -1, -0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray in front of triangle pointing backwards
	void RayTriangleTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 0), Vector3(-0, -1, -0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray in front of triangle pointing forwards
	void RayTriangleTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 1, 0), Vector3(0, 1, 0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 1
	void RayTriangleTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1.1f, -1, -1.1f), Vector3(0, 1, 0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 2
	void RayTriangleTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1.1f, -1, 1.1f), Vector3(0, 1, 0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 3
	void RayTriangleTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1.1f, -1, 0), Vector3(0, 1, 0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge12
	void RayTriangleTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1.1f, -1, 0), Vector3(0, 1, 0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge23
	void RayTriangleTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.22f, -1, 0.44f), Vector3(0, 1, 0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge31
	void RayTriangleTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.22f, -1, -0.44f), Vector3(0, 1, 0));
		Triangle tri(Vector3(1, 0, -1), Vector3(1, 0, 1), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind triangle pointing forwards
	void RayTriangleTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 0, 0), Vector3(1, 0, 0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind triangle pointing backwards
	void RayTriangleTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 0, 0), Vector3(-1, -0, -0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray in front of triangle pointing backwards
	void RayTriangleTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 0, 0), Vector3(-1, -0, -0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray in front of triangle pointing forwards
	void RayTriangleTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 0, 0), Vector3(1, 0, 0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 1
	void RayTriangleTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 1.1f, -1.1f), Vector3(1, 0, 0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 2
	void RayTriangleTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, -1.1f, -1.1f), Vector3(1, 0, 0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 3
	void RayTriangleTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 0, 1.1f), Vector3(1, 0, 0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge12
	void RayTriangleTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 0, -1.1f), Vector3(1, 0, 0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge23
	void RayTriangleTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, -0.44f, 0.22f), Vector3(1, 0, 0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge31
	void RayTriangleTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1, 0.44f, 0.22f), Vector3(1, 0, 0));
		Triangle tri(Vector3(0, 1, -1), Vector3(0, -1, -1), Vector3(0, 0, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind triangle pointing forwards
	void RayTriangleTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, -1), Vector3(0, 0, 1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind triangle pointing backwards
	void RayTriangleTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, -1), Vector3(-0, -0, -1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray in front of triangle pointing backwards
	void RayTriangleTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 1), Vector3(-0, -0, -1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray in front of triangle pointing forwards
	void RayTriangleTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 1), Vector3(0, 0, 1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 1
	void RayTriangleTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1.1f, 1.1f, -1), Vector3(0, 0, 1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 2
	void RayTriangleTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1.1f, -1.1f, -1), Vector3(0, 0, 1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray behind point 3
	void RayTriangleTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-1.1f, 0, -1), Vector3(0, 0, 1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge12
	void RayTriangleTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1.1f, 0, -1), Vector3(0, 0, 1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge23
	void RayTriangleTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.22f, -0.44f, -1), Vector3(0, 0, 1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

	// Ray outside edge31
	void RayTriangleTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.22f, 0.44f, -1), Vector3(0, 0, 1));
		Triangle tri(Vector3(1, 1, 0), Vector3(1, -1, 0), Vector3(-1, 0, 0));

		float t{ FLT_MAX };
		PrintResultRay(TestRayTriangle(ray, tri, t), t);
	}

#pragma endregion

#pragma region RaySphere
/** Ray Vs Sphere : 85 tests **/

	// Ray behind sphere pointing backwards
	void RaySphereTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-9, -1, 0), Vector3(-1, -0, -0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing forwards
	void RaySphereTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-9, -1, 0), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing but not going through the sphere center
	void RaySphereTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-9, -0.333333f, 0), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray at sphere center
	void RaySphereTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, 0), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing backwards
	void RaySphereTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(11, -1, 0), Vector3(-1, -0, -0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing forwards
	void RaySphereTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(11, -1, 0), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far behind sphere pointing forwards
	void RaySphereTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-9, -1, 0), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far in front of sphere pointing backwards
	void RaySphereTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(11, -1, 0), Vector3(-1, -0, -0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-9, 0.98f, 0), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-9, -2.98f, 0), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-9, -1, 1.98f), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-9, -1, -1.98f), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-29, -1, 1.98f), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(31, -1, 1.98f), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-29, -1, -1.98f), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(31, -1, -1.98f), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray misses sphere
	void RaySphereTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, -4), Vector3(1, 0, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing backwards
	void RaySphereTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -11, 0), Vector3(-0, -1, -0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing forwards
	void RaySphereTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -11, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing but not going through the sphere center
	void RaySphereTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -11, -0.666667f), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray at sphere center
	void RaySphereTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing backwards
	void RaySphereTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 9, 0), Vector3(-0, -1, -0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing forwards
	void RaySphereTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 9, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far behind sphere pointing forwards
	void RaySphereTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -11, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far in front of sphere pointing backwards
	void RaySphereTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 9, 0), Vector3(-0, -1, -0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -11, -1.98f), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -11, 1.98f), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.98f, -11, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.98f, -11, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.98f, -31, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest31(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.98f, 29, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest32(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.98f, -31, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest33(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.98f, 29, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray misses sphere
	void RaySphereTest34(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(5, -1, 0), Vector3(0, 1, 0));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing backwards
	void RaySphereTest35(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, -10), Vector3(-0, -0, -1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing forwards
	void RaySphereTest36(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, -10), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing but not going through the sphere center
	void RaySphereTest37(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -0.333333f, -10), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray at sphere center
	void RaySphereTest38(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, 0), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing backwards
	void RaySphereTest39(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, 10), Vector3(-0, -0, -1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing forwards
	void RaySphereTest40(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, 10), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far behind sphere pointing forwards
	void RaySphereTest41(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, -10), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far in front of sphere pointing backwards
	void RaySphereTest42(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, 10), Vector3(-0, -0, -1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest43(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, 0.98f, -10), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest44(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -2.98f, -10), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest45(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.98f, -1, -10), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest46(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.98f, -1, -10), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest47(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.98f, -1, -30), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest48(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.98f, -1, 30), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest49(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.98f, -1, -30), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest50(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.98f, -1, 30), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray misses sphere
	void RaySphereTest51(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(5, -1, 0), Vector3(0, 0, 1));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing backwards
	void RaySphereTest52(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4.7735f, -6.7735f, -5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing forwards
	void RaySphereTest53(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4.7735f, -6.7735f, -5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing but not going through the sphere center
	void RaySphereTest54(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-5.24491f, -6.3021f, -5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray at sphere center
	void RaySphereTest55(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, 0), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing backwards
	void RaySphereTest56(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(6.7735f, 4.7735f, 5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing forwards
	void RaySphereTest57(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(6.7735f, 4.7735f, 5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far behind sphere pointing forwards
	void RaySphereTest58(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4.7735f, -6.7735f, -5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far in front of sphere pointing backwards
	void RaySphereTest59(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(6.7735f, 4.7735f, 5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest60(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-6.17357f, -5.37343f, -5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest61(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3.37343f, -8.17357f, -5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest62(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-5.58183f, -7.58183f, -4.15684f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest63(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3.96517f, -5.96517f, -7.39017f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest64(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-17.1288f, -19.1288f, -15.7038f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest65(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(17.5122f, 15.5122f, 18.9372f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest66(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-15.5122f, -17.5122f, -18.9372f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest67(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(19.1288f, 17.1288f, 15.7038f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray misses sphere
	void RaySphereTest68(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.63299f, 0.632993f, -3.26599f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing backwards
	void RaySphereTest69(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(6.7735f, 4.7735f, 5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing forwards
	void RaySphereTest70(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(6.7735f, 4.7735f, 5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray behind sphere pointing but not going through the sphere center
	void RaySphereTest71(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(7.24491f, 4.3021f, 5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray at sphere center
	void RaySphereTest72(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(1, -1, 0), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing backwards
	void RaySphereTest73(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4.7735f, -6.7735f, -5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray in front of sphere pointing forwards
	void RaySphereTest74(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4.7735f, -6.7735f, -5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far behind sphere pointing forwards
	void RaySphereTest75(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(6.7735f, 4.7735f, 5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray far in front of sphere pointing backwards
	void RaySphereTest76(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4.7735f, -6.7735f, -5.7735f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest77(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(8.17357f, 3.37343f, 5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest78(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(5.37343f, 6.17357f, 5.7735f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest79(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(5.96517f, 3.96517f, 7.39017f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest80(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(7.58183f, 5.58183f, 4.15684f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest81(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(17.5122f, 15.5122f, 18.9372f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest82(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-17.1288f, -19.1288f, -15.7038f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest83(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(19.1288f, 17.1288f, 15.7038f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray Tangent to sphere
	void RaySphereTest84(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-15.5122f, -17.5122f, -18.9372f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

	// Ray misses sphere
	void RaySphereTest85(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2.63299f, 0.632993f, -3.26599f), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Sphere sphere(Vector3(1, -1, 0), 2);

		float t{ FLT_MAX };
		PrintResultRay(TestRaySphere(ray, sphere,t),t);
	}

#pragma endregion

#pragma region RayAABB
	/** Ray Vs AABB : 91 tests **/

		// Ray behind aabb pointing backwards
	void RayAabbTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 0, 0), Vector3(-1, -0, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 0, 0), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray at aabb center
	void RayAabbTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 0), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 0, 0), Vector3(-1, -0, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 0, 0), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(4, 0, 0), Vector3(-1, -0, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(4, 0, 0), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, 0, 0), Vector3(-1, -0, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, 0, 0), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 2, 0), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, 0), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 0, 2), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 0, -2), Vector3(1, 0, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -2, 0), Vector3(-0, -1, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -2, 0), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray at aabb center
	void RayAabbTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 0), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 2, 0), Vector3(-0, -1, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 2, 0), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 4, 0), Vector3(-0, -1, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 4, 0), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -4, 0), Vector3(-0, -1, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -4, 0), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -2, -2), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -2, 2), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, 0), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, -2, 0), Vector3(0, 1, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, -2), Vector3(-0, -0, -1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, -2), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray at aabb center
	void RayAabbTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 0), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 2), Vector3(-0, -0, -1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest31(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 2), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest32(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 4), Vector3(-0, -0, -1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest33(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 4), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest34(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, -4), Vector3(-0, -0, -1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest35(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, -4), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest36(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 2, -2), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest37(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -2, -2), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest38(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 0, -2), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest39(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 0, -2), Vector3(0, 0, 1));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest40(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, 0), Vector3(-0.707107f, -0.707107f, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest41(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, 0), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray at aabb center
	void RayAabbTest42(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 0), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest43(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 2, 0), Vector3(-0.707107f, -0.707107f, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest44(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 2, 0), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest45(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(4, 4, 0), Vector3(-0.707107f, -0.707107f, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest46(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(4, 4, 0), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest47(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, -4, 0), Vector3(-0.707107f, -0.707107f, -0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest48(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, -4, 0), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest49(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, 3.57628e-07f, 0), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest50(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.57628e-07f, -4, 0), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest51(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, 2), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest52(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, -2), Vector3(0.707107f, 0.707107f, 0));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest53(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 0, -2), Vector3(-0.707107f, -0, -0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest54(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 0, -2), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray at aabb center
	void RayAabbTest55(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 0), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest56(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 0, 2), Vector3(-0.707107f, -0, -0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest57(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 0, 2), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest58(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(4, 0, 4), Vector3(-0.707107f, -0, -0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest59(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(4, 0, 4), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest60(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, 0, -4), Vector3(-0.707107f, -0, -0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest61(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, 0, -4), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest62(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, 2, -2), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest63(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, -2), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest64(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, 0, 3.57628e-07f), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest65(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(3.57628e-07f, 0, -4), Vector3(0.707107f, 0, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest66(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -2, -2), Vector3(-0, -0.707107f, -0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest67(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -2, -2), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray at aabb center
	void RayAabbTest68(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 0), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest69(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 2, 2), Vector3(-0, -0.707107f, -0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest70(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 2, 2), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest71(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 4, 4), Vector3(-0, -0.707107f, -0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest72(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 4, 4), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest73(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -4, -4), Vector3(-0, -0.707107f, -0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest74(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -4, -4), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest75(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 3.57628e-07f, -4), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest76(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -4, 3.57628e-07f), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest77(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, -2), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest78(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, -2, -2), Vector3(0, 0.707107f, 0.707107f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest79(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, -2), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest80(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-2, -2, -2), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray at aabb center
	void RayAabbTest81(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, 0, 0), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest82(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 2, 2), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest83(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(2, 2, 2), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing backwards
	void RayAabbTest84(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(4, 4, 4), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray in front of aabb pointing forwards
	void RayAabbTest85(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(4, 4, 4), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing backwards
	void RayAabbTest86(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, -4, -4), Vector3(-0.57735f, -0.57735f, -0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	// Ray behind aabb pointing forwards
	void RayAabbTest87(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, -4, -4), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest88(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-4, 0, -2), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest89(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(0, -4, -2), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest90(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-3.33333f, -3.33333f, 0.666667f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

	void RayAabbTest91(const std::string& testName)
	{
		PrintTestHeader(testName);

		Ray ray(Vector3(-0.666667f, -0.666667f, -4.66667f), Vector3(0.57735f, 0.57735f, 0.57735f));
		Aabb aabb(Vector3(-1, -1, -1), Vector3(1, 1, 1));

		float t{ FLT_MAX };
		PrintResultRay(TestRayAabb(ray, aabb, t), t);
	}

#pragma endregion

#pragma region PlaneSphere
/** Plane Vs Sphere : 20 tests **/

	// Sphere in front of plane
	void PlaneSphereTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(2, 0, 0), 1);
		
		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere just touching plane (on the front)
	void PlaneSphereTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(1, 0, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere centered at plane
	void PlaneSphereTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 0, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere just touching plane (on the back)
	void PlaneSphereTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(-1, 0, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere behind plane
	void PlaneSphereTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(-2, 0, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere in front of plane
	void PlaneSphereTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 2, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere just touching plane (on the front)
	void PlaneSphereTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 1, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere centered at plane
	void PlaneSphereTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 0, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere just touching plane (on the back)
	void PlaneSphereTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, -1, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere behind plane
	void PlaneSphereTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, -2, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere in front of plane
	void PlaneSphereTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 0, 2), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere just touching plane (on the front)
	void PlaneSphereTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 0, 1), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere centered at plane
	void PlaneSphereTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 0, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere just touching plane (on the back)
	void PlaneSphereTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 0, -1), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere behind plane
	void PlaneSphereTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 0, -2), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere in front of plane
	void PlaneSphereTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.57735f, 0.57735f, 0.57735f), Vector3(0, 0, 0));
		Sphere sphere(Vector3(1.1547f, 1.1547f, 1.1547f), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere just touching plane (on the front)
	void PlaneSphereTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.57735f, 0.57735f, 0.57735f), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0.57735f, 0.57735f, 0.57735f), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere centered at plane
	void PlaneSphereTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.57735f, 0.57735f, 0.57735f), Vector3(0, 0, 0));
		Sphere sphere(Vector3(0, 0, 0), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere just touching plane (on the back)
	void PlaneSphereTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.57735f, 0.57735f, 0.57735f), Vector3(0, 0, 0));
		Sphere sphere(Vector3(-0.57735f, -0.57735f, -0.57735f), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

	// Sphere behind plane
	void PlaneSphereTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.57735f, 0.57735f, 0.57735f), Vector3(0, 0, 0));
		Sphere sphere(Vector3(-1.1547f, -1.1547f, -1.1547f), 1);

		float t{0.0f};
		PrintResultPlane(TestPlaneSphere(plane, sphere, t) ,t);
	}

#pragma endregion

#pragma region PlaneAABB
/** Plane Vs AABB : 50 tests **/

	// Aabb in front of plane
	void PlaneAabbTest1(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(0.5f, -0.5f, -0.5f), Vector3(1.5f, 0.5f, 0.5f));

		float t{ 0.0f };
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely in front of plane
	void PlaneAabbTest2(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(0.05f, -0.5f, -0.5f), Vector3(1.05f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb on front)
	void PlaneAabbTest3(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.05f, -0.5f, -0.5f), Vector3(0.95f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb slightly touching plane (aabb on front)
	void PlaneAabbTest4(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.125f, -0.5f, -0.5f), Vector3(0.875f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb centered on plane
	void PlaneAabbTest5(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb signtly touching plane (aabb in back)
	void PlaneAabbTest6(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.875f, -0.5f, -0.5f), Vector3(0.125f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb in back)
	void PlaneAabbTest7(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.95f, -0.5f, -0.5f), Vector3(0.05f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely behind plane
	void PlaneAabbTest8(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-1.05f, -0.5f, -0.5f), Vector3(-0.05f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb behind plane
	void PlaneAabbTest9(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(1, 0, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-1.5f, -0.5f, -0.5f), Vector3(-0.5f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb in front of plane
	void PlaneAabbTest10(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.5f, 1.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely in front of plane
	void PlaneAabbTest11(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, 0.05f, -0.5f), Vector3(0.5f, 1.05f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb on front)
	void PlaneAabbTest12(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.05f, -0.5f), Vector3(0.5f, 0.95f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb slightly touching plane (aabb on front)
	void PlaneAabbTest13(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.125f, -0.5f), Vector3(0.5f, 0.875f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb centered on plane
	void PlaneAabbTest14(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb signtly touching plane (aabb in back)
	void PlaneAabbTest15(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.875f, -0.5f), Vector3(0.5f, 0.125f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb in back)
	void PlaneAabbTest16(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.95f, -0.5f), Vector3(0.5f, 0.05f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely behind plane
	void PlaneAabbTest17(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -1.05f, -0.5f), Vector3(0.5f, -0.05f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb behind plane
	void PlaneAabbTest18(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 1, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -1.5f, -0.5f), Vector3(0.5f, -0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb in front of plane
	void PlaneAabbTest19(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.5f, 0.5f, 1.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely in front of plane
	void PlaneAabbTest20(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, 0.05f), Vector3(0.5f, 0.5f, 1.05f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb on front)
	void PlaneAabbTest21(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.05f), Vector3(0.5f, 0.5f, 0.95f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb slightly touching plane (aabb on front)
	void PlaneAabbTest22(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.125f), Vector3(0.5f, 0.5f, 0.875f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb centered on plane
	void PlaneAabbTest23(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb signtly touching plane (aabb in back)
	void PlaneAabbTest24(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.875f), Vector3(0.5f, 0.5f, 0.125f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb in back)
	void PlaneAabbTest25(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.95f), Vector3(0.5f, 0.5f, 0.05f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely behind plane
	void PlaneAabbTest26(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -1.05f), Vector3(0.5f, 0.5f, -0.05f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb behind plane
	void PlaneAabbTest27(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0, 1), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -1.5f), Vector3(0.5f, 0.5f, -0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb in front of plane
	void PlaneAabbTest28(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(0.5f, 0.5f, -0.5f), Vector3(1.5f, 1.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely in front of plane
	void PlaneAabbTest29(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(0.05f, 0.05f, -0.5f), Vector3(1.05f, 1.05f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb on front)
	void PlaneAabbTest30(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.05f, -0.05f, -0.5f), Vector3(0.95f, 0.95f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb slightly touching plane (aabb on front)
	void PlaneAabbTest31(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.125f, -0.125f, -0.5f), Vector3(0.875f, 0.875f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb centered on plane
	void PlaneAabbTest32(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb signtly touching plane (aabb in back)
	void PlaneAabbTest33(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.875f, -0.875f, -0.5f), Vector3(0.125f, 0.125f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb in back)
	void PlaneAabbTest34(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.95f, -0.95f, -0.5f), Vector3(0.05f, 0.05f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely behind plane
	void PlaneAabbTest35(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-1.05f, -1.05f, -0.5f), Vector3(-0.05f, -0.05f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb behind plane
	void PlaneAabbTest36(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0.707107f, 0), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-1.5f, -1.5f, -0.5f), Vector3(-0.5f, -0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb in front of plane
	void PlaneAabbTest37(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(0.5f, -0.5f, 0.5f), Vector3(1.5f, 0.5f, 1.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely in front of plane
	void PlaneAabbTest38(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(0.05f, -0.5f, 0.05f), Vector3(1.05f, 0.5f, 1.05f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb on front)
	void PlaneAabbTest39(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.05f, -0.5f, -0.05f), Vector3(0.95f, 0.5f, 0.95f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb slightly touching plane (aabb on front)
	void PlaneAabbTest40(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.125f, -0.5f, -0.125f), Vector3(0.875f, 0.5f, 0.875f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb centered on plane
	void PlaneAabbTest41(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb signtly touching plane (aabb in back)
	void PlaneAabbTest42(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.875f, -0.5f, -0.875f), Vector3(0.125f, 0.5f, 0.125f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb in back)
	void PlaneAabbTest43(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.95f, -0.5f, -0.95f), Vector3(0.05f, 0.5f, 0.05f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely behind plane
	void PlaneAabbTest44(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-1.05f, -0.5f, -1.05f), Vector3(-0.05f, 0.5f, -0.05f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb behind plane
	void PlaneAabbTest45(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0.707107f, 0, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-1.5f, -0.5f, -1.5f), Vector3(-0.5f, 0.5f, -0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb in front of plane
	void PlaneAabbTest46(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0.707107f, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.5f, 1.5f, 1.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely in front of plane
	void PlaneAabbTest47(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0.707107f, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, 0.05f, 0.05f), Vector3(0.5f, 1.05f, 1.05f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb just barely touching plane (aabb on front)
	void PlaneAabbTest48(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0.707107f, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.05f, -0.05f), Vector3(0.5f, 0.95f, 0.95f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb slightly touching plane (aabb on front)
	void PlaneAabbTest49(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0.707107f, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.125f, -0.125f), Vector3(0.5f, 0.875f, 0.875f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

	// Aabb centered on plane
	void PlaneAabbTest50(const std::string& testName)
	{
		PrintTestHeader(testName);

		Plane plane(Vector3(0, 0.707107f, 0.707107f), Vector3(0, 0, 0));
		Aabb aabb(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		float t{ 0.0f }; 
		PrintResultPlane(TestPlaneAabb(plane, aabb, t), t);
	}

#pragma endregion

