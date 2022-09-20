#pragma once

#include "MathCommon.h"
#include "Geometry.h"
#include "GfxTypes.h"

#include <vector>
#include <array>

// Debug draw interface, to decouple with the "god class".
struct DebugDraw
{
	static void AddLine(const glm::vec3& p0, const glm::vec3& p1, const oGFX::Color& col = oGFX::Colors::WHITE);
	static void AddAABB(const AABB& aabb, const oGFX::Color& col = oGFX::Colors::WHITE);
	static void AddSphere(const Sphere& sphere, const oGFX::Color& col = oGFX::Colors::WHITE);
	static void AddTriangle(const Triangle& tri, const oGFX::Color& col = oGFX::Colors::WHITE);

	// Debug draws a circular disc, spanned by the 2 given basis vectors
	static void AddDisc(const glm::vec3& center, float radius, const glm::vec3& basis0, const glm::vec3& basis1, const oGFX::Color& color = oGFX::Colors::WHITE);
	// Debug draws a circular disc, using a direction/normal. The 2 basis vectors will be implicitly generated internally.
	static void AddDisc(const glm::vec3& center, float radius, const glm::vec3& normal, const oGFX::Color& color = oGFX::Colors::WHITE);
	
	// Add more as needed...
};
