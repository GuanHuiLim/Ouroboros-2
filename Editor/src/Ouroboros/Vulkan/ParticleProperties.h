/************************************************************************************//*!
\file          ParticleProperties.h
\project       Ouroboros
\author        Jamie Kong, j.kong , 390004720 | code contribution (100%)
\par           email: j.kong\@digipen.edu
\date          December 1, 2021
\brief         File contains particle properties data.

Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <glm/common.hpp>
#include "OO_Vulkan/src/GfxTypes.h"
#include "OO_Vulkan/src/Mesh.h"
#include "Ouroboros/Asset/Asset.h"
#include "Ouroboros/Vulkan/MeshInfo.h"


#include <rttr/type>
#include <vector>

namespace oo
{

enum class ParticleShape 
{
	Cone=0,
	Circle=1,
};

struct InterpolatedValues
{
	oGFX::Color col;
	float speed;
	float rotation;
	glm::vec3 direction;
	glm::vec3 size;
};

struct ParticleProperties
{
	ParticleProperties();

	float lifetime = 0.0f;
	float maxLifetime = 5.0f;

	//glm::vec3 direction{0.0f};
	glm::vec3 scale{1.0f};

	float startSpeed;

	std::vector<float> p_colours;
	std::vector<oGFX::Color> colours;

	std::vector<float> p_speeds;
	std::vector<float> speeds;

	std::vector<float> p_rotations;
	std::vector<float> rotations;

	std::vector<float> p_directions;
	std::vector<glm::vec3> directions;

	std::vector<float> p_sizes;
	std::vector<glm::vec3> sizes;

	bool localSpace = false;

	// TODO : Make this better..
	//std::vector<Colour>& GetColourList() ;
	//void SetColourList(std::vector<Colour>& col);
	//
	//Colour GetColour_B() const ;
	//void SetColour_B(Colour col);
	//

	bool GetLocalSpace() const ;
	void SetLocalSpace(bool b);

	RTTR_ENABLE();
};

struct ParticlePropsRenderer
{

	enum ParticleType
	{
		BILLBOARD,
		MESH
	}m_renderType{BILLBOARD};

	// no need to serialize
	uint32_t ModelHandle{ 0 };

	MeshInfo MeshInformation;

	Asset AlbedoHandle;
	uint32_t AlbedoID = 0xFFFFFFFF;

	Asset NormalHandle;
	uint32_t NormalID = 0xFFFFFFFF;

	Asset MetallicHandle;
	uint32_t MetallicID = 0xFFFFFFFF;

	Asset RoughnessHandle;
	uint32_t RoughnessID = 0xFFFFFFFF;

	ParticleType GetParticleType();
	void SetParticleType(ParticleType ty);

	// TODO
	//AssetHandle emission;

	MeshInfo GetMeshInfo();
    /*********************************************************************************//*!
    \brief      this function will only set the submeshbits
    *//**********************************************************************************/
    void SetMeshInfo(MeshInfo info);
    
    Asset GetMesh();
    void SetMesh(Asset _asset);


    //set a single model and asset
    void SetModelHandle(Asset _asset, uint32_t _submodel_id);
    
    void SetAlbedoMap(Asset albedoMap);
    Asset GetAlbedoMap() const;

    void SetNormalMap(Asset normalMap);
    Asset GetNormalMap() const;

    void SetMetallicMap(Asset metallicMap);
    Asset GetMetallicMap() const;

    void SetRoughnessMap(Asset roughnessMap);
    Asset GetRoughnessMap() const;

	//AssetHandle GetEmission() { return emission; }
	//void SetEmission(AssetHandle hdl) { emission = hdl; }

	RTTR_ENABLE();
};

struct ParticlePropsShape
{
	ParticleShape shape = ParticleShape::Cone;

	float angle{30.0f};
	float size{1.0f};

	float GetAngle() { return angle; }
	void SetAngle(float a) { angle = a; }

	float GetSize() { return size; }
	void SetSize(float s) { size = std::max(s, 0.0f); }

	ParticleShape GetShape() { return shape; }
	void SetShape(ParticleShape sh) { 
		shape = sh; 
	}

	RTTR_ENABLE();
};

} // end namespace oo