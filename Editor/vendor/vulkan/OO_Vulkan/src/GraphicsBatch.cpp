/************************************************************************************//*!
\file           GraphicsBatch.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines GraphicsBatch, a generator for command lists for objects that require to be rendered.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "GraphicsBatch.h"

#include "VulkanRenderer.h"
#include "MathCommon.h"
#include "GraphicsWorld.h"
#include "gpuCommon.h"
#include <cassert>
#include "Profiling.h"

//#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
//#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
//#include <locale>
//#include <codecvt>

#include <sstream>
#include <numeric>

GraphicsBatch GraphicsBatch::Init(GraphicsWorld* gw, VulkanRenderer* renderer, size_t maxObjects)
{
	assert(gw != nullptr);
	assert(renderer != nullptr);

	GraphicsBatch gb;
	gb.m_world = gw;
	gb.m_renderer = renderer;

	for (auto& batch : gb.m_batches)
	{
		batch.reserve(maxObjects);
	}
	s_scratchBuffer.reserve(maxObjects);

	return gb;
}

void AppendBatch(std::vector<oGFX::IndirectCommand>& dest, std::vector<oGFX::IndirectCommand>& src)
{
	dest.insert(dest.end(), src.begin(), src.end());
}

void GraphicsBatch::GenerateBatches()
{
	PROFILE_SCOPED("Generate graphics batch");

	// clear old batches
	for (auto& batch : m_batches)
	{
		batch.clear();
	}

	ProcessGeometry();

	for (auto& batch : m_batches)
	{
		// set up first instance index
		//std::for_each(batch.begin(), batch.end(),
		//	[x = uint32_t{ 0 }](oGFX::IndirectCommand& c) mutable { 
		//	c.firstInstance = c.firstInstance == 0 ? x++ : x - 1;
		//});
	}

	m_uiVertices.clear();
	ProcessUI();

	ProcessLights();

	ProcessParticleEmitters();

}

void GraphicsBatch::ProcessLights()
{
	for (auto& light : m_world->GetAllOmniLightInstances())
	{
		constexpr glm::vec3 up{ 0.0f,1.0f,0.0f };
		constexpr glm::vec3 right{ 1.0f,0.0f,0.0f };
		constexpr glm::vec3 forward{ 0.0f,0.0f,-1.0f };

		light.view[0] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position) + -up, glm::vec3{ 0.0f, 0.0f,-1.0f });
		light.view[1] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position) + up, glm::vec3{ 0.0f, 0.0f, 1.0f });
		light.view[2] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position) + -right, glm::vec3{ 0.0f,1.0f, 0.0f });
		light.view[3] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position) + right, glm::vec3{ 0.0f,1.0f, 0.0f });
		light.view[4] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position) + -forward, glm::vec3{ 0.0f,-1.0f, 0.0f });
		light.view[5] = glm::lookAt(glm::vec3(light.position), glm::vec3(light.position) + forward, glm::vec3{ 0.0f,-1.0f, 0.0f });

		light.projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, light.radius.x);
	}
}

void GraphicsBatch::ProcessGeometry()
{
	using Batch = GraphicsBatch::DrawBatch;
	using Flags = ObjectInstanceFlags;
	auto& entities = m_world->GetAllObjectInstances();
	int32_t currModelID{ -1 };
	int32_t cnt{ 0 };
	for (auto& ent : entities)
	{
		auto& model = m_renderer->g_globalModels[ent.modelID];

		// skip entities dont want to render
		//if (ent.isRenderable() == false)
		//{
		//	// still increment instance
		//	++cnt;
		//	continue;
		//}

		if (ent.modelID != currModelID) // check if we are using the same model
		{
			s_scratchBuffer.clear();
			for (size_t i = 0; i < model.m_subMeshes.size(); i++)
			{
				if (ent.submesh[i] == true)
				{
					const auto& subMesh = model.m_subMeshes[i];
					// clear the buffer to prepare for this model
					oGFX::IndirectCommand indirectCmd{};
					indirectCmd.instanceCount = ent.isRenderable();

					// this is the number invoked by the graphics pipeline as the instance id (location = 15) etc..
					// the number represents the index into the InstanceData array see VulkanRenderer::UploadInstanceData();
					indirectCmd.firstInstance = cnt++; 

					indirectCmd.firstIndex = model.baseIndices + subMesh.baseIndices;
					indirectCmd.indexCount = subMesh.indicesCount;
					indirectCmd.vertexOffset = model.baseVertex + subMesh.baseVertex;

					auto& s = model.m_subMeshes[i].boundingSphere;
					indirectCmd.sphere = glm::vec4(s.center,s.radius);

					s_scratchBuffer.emplace_back(indirectCmd);
				}
			}
		}

		if (ent.isShadowEnabled())
		{
			AppendBatch(m_batches[Batch::SHADOW_CAST], s_scratchBuffer);
		}

		if (ent.isDynamic())
		{
			if (ent.isTransparent())
			{
				AppendBatch(m_batches[Batch::FORWARD_DYNAMIC], s_scratchBuffer);
			}
			else
			{
				AppendBatch(m_batches[Batch::GBUFFER_DYNAMIC], s_scratchBuffer);
			}
		}

		//if (ent.flags & Flags::ENABLE_ZPREPASS)
		//{
		//	AppendBatch(m_batches[Batch::ZPREPASS], s_scratchBuffer);
		//}
		//
		//if (ent.flags & Flags::EMITTER)
		//{
		//	AppendBatch(m_batches[Batch::LIGHT_SPOT], s_scratchBuffer);
		//}

		if (ent.isShadowEnabled())
		{
			// get shadow enabled lights
			AppendBatch(m_batches[Batch::SHADOW_LIGHT], s_scratchBuffer);
		}

		// append to the batches
		AppendBatch(m_batches[Batch::ALL_OBJECTS], s_scratchBuffer);

	}
}

void GraphicsBatch::ProcessUI()
{
	using Flags = UIInstanceFlags;
	auto& allUI = m_world->GetAllUIInstances();

	for (auto& ui: allUI)
	{
		if (static_cast<bool>(ui.flags & Flags::RENDER_ENABLED) == false)
		{
			continue;
		}

		if (static_cast<bool>(ui.flags & Flags::SCREEN_SPACE) == true)
		{
			// skip non depth
			continue;
		}

		if (static_cast<bool>(ui.flags & Flags::TEXT_INSTANCE))
		{
			GenerateTextGeometry(ui);
		}
		else
		{
			GenerateSpriteGeometry(ui);
		}
	}

	// dirty way of doing screenspace
	m_SSVertOffset = m_uiVertices.size();
	for (auto& ui: allUI)
	{
		if (static_cast<bool>(ui.flags & Flags::RENDER_ENABLED) == false)
		{
			continue;
		}

		if (static_cast<bool>(ui.flags & Flags::SCREEN_SPACE) == false)
		{
			// skip depth
			continue;
		}

		if (static_cast<bool>(ui.flags & Flags::TEXT_INSTANCE))
		{
			GenerateTextGeometry(ui);
		}
		else
		{
			GenerateSpriteGeometry(ui);
		}
	}

}

void GraphicsBatch::ProcessParticleEmitters()
{
	auto& allEmitters = m_world->GetAllEmitterInstances();
	m_particleList.clear();
	m_particleCommands.clear();
	/// Create parciles batch
	uint32_t emitterCnt = 0;
	auto* vr = VulkanRenderer::get();
	for (auto& emitter : allEmitters)
	{
		// note to support multiple textures permesh we have to do this per submesh 
		//setup instance data	
		// TODO: this is really bad fix this
		// This is per entity. Should be per material.
		uint32_t albedo = emitter.bindlessGlobalTextureIndex_Albedo;
		uint32_t normal = emitter.bindlessGlobalTextureIndex_Normal;
		uint32_t roughness = emitter.bindlessGlobalTextureIndex_Roughness;
		uint32_t metallic = emitter.bindlessGlobalTextureIndex_Metallic;
		constexpr uint32_t invalidIndex = 0xFFFFFFFF;
		if (albedo == invalidIndex)
			albedo = vr->whiteTextureID;
		if (normal == invalidIndex)
			normal = vr->blackTextureID;
		if (roughness == invalidIndex)
			roughness = vr->whiteTextureID;
		if (metallic == invalidIndex)
			metallic = vr->blackTextureID;

		// Important: Make sure this index packing matches the unpacking in the shader
		const uint32_t albedo_normal = albedo << 16 | (normal & 0xFFFF);
		const uint32_t roughness_metallic = roughness << 16 | (metallic & 0xFFFF);
		for (auto& pd : emitter.particles)
		{
			pd.instanceData.z = albedo_normal;
			pd.instanceData.w = roughness_metallic;
		}

		// copy list
		m_particleList.insert(m_particleList.end(), emitter.particles.begin(), emitter.particles.end());

		auto& model = m_renderer->g_globalModels[emitter.modelID];
		// set up the commands and number of particles
		oGFX::IndirectCommand cmd{};

		cmd.instanceCount = static_cast<uint32_t>(emitter.particles.size());
		// this is the number invoked by the graphics pipeline as the instance id (location = 15) etc..
		// the number represents the index into the InstanceData array see VulkanRenderer::UploadInstanceData();
		cmd.firstInstance = emitterCnt;
		for (size_t i = 0; i < emitter.submesh.size(); i++)
		{
			// create a draw call for each submesh using the same instance data
			if (emitter.submesh[i] == true)
			{
				const auto& subMesh = model.m_subMeshes[i];
				cmd.firstIndex = model.baseIndices + subMesh.baseIndices;
				cmd.indexCount = subMesh.indicesCount;
				cmd.vertexOffset = model.baseVertex + subMesh.baseVertex;
				m_particleCommands.push_back(cmd);
			}
		}
		//increment instance data
		emitterCnt += cmd.instanceCount;

		// clear this so next draw we dont care
		emitter.particles.clear();
	}
}

const std::vector<oGFX::IndirectCommand>& GraphicsBatch::GetBatch(int32_t batchIdx)
{
	assert(batchIdx > -1 && batchIdx < GraphicsBatch::MAX_NUM);

	return m_batches[batchIdx];
}

const std::vector<oGFX::IndirectCommand>& GraphicsBatch::GetParticlesBatch()
{
	return m_particleCommands;
}

const std::vector<ParticleData>& GraphicsBatch::GetParticlesData()
{
	return m_particleList;
}

const std::vector<oGFX::UIVertex>& GraphicsBatch::GetUIVertices()
{
	return m_uiVertices;
}

size_t GraphicsBatch::GetScreenSpaceUIOffset() const
{
	return m_SSVertOffset;
}

void GraphicsBatch::GenerateSpriteGeometry(const UIInstance& ui) 
{

	const auto& mdl_xform = ui.localToWorld;

	// hardcode for now
	constexpr size_t quadVertexCount = 4;
	oGFX::AABB2D texCoords{ glm::vec2{0.0f}, glm::vec2{1.0f} };
	glm::vec2 textureCoords[quadVertexCount] = {
		{ texCoords.min.x, texCoords.max.y },
		{ texCoords.min.x, texCoords.min.y },
		{ texCoords.max.x, texCoords.min.y },
		{ texCoords.max.x, texCoords.max.y },
	};

	
	constexpr glm::vec4 verts[quadVertexCount] = {
		{ 0.5f,-0.5f, 0.0f, 1.0f},
		{ 0.5f, 0.5f, 0.0f, 1.0f},
		{-0.5f, 0.5f, 0.0f, 1.0f},
		{-0.5f,-0.5f, 0.0f, 1.0f},
	};

	auto invalidIndex = 0xFFFFFFFF;
	auto& vr = *VulkanRenderer::get();
	auto albedo = ui.bindlessGlobalTextureIndex_Albedo;
	if (albedo == invalidIndex || vr.g_Textures[albedo].isValid == false)
		albedo = vr.whiteTextureID; // TODO: Dont hardcode this bindless texture index

	for (size_t i = 0; i < quadVertexCount; i++)
	{
		oGFX::UIVertex vert;
		vert.pos = mdl_xform * verts[i];
		vert.pos.w = 1.0; // positive is sprite
		vert.col = ui.colour;
		vert.tex = glm::vec4(textureCoords[i], albedo, ui.entityID);
		m_uiVertices.push_back(vert);
	}

}

void GraphicsBatch::GenerateTextGeometry(const UIInstance& ui)
{
	using FontFormatting = oGFX::FontFormatting;
	using FontAlignment = oGFX::FontAlignment;
	PROFILE_SCOPED("Generate text geom");



	auto* fontAtlas = ui.fontAsset;
	if (!fontAtlas)
	{
		fontAtlas = VulkanRenderer::get()->GetDefaultFont();
	}

	const auto& text = ui.textData;
	const auto& mdl_xform = ui.localToWorld;

	//float fontScale = format.fontSize / fontAtlas->m_pixelSize;
	float fontScale = ui.format.fontSize;


	float boxPixelSizeX = fabsf(ui.format.box.max.x - ui.format.box.min.x);
	float halfBoxX = boxPixelSizeX / 2.0f;
	float boxPixelSizeY = fabsf(ui.format.box.max.y - ui.format.box.min.y);
	float halfBoxY = boxPixelSizeY / 2.0f;

	//static std::wstring_convert<std::codecvt_utf8_utf16<std::wstring::value_type>> converter;
	//std::wstring unicode = converter.from_bytes(text);

	//std::stringstream ss(unicode);
	std::stringstream ss(text);
	std::vector<std::string> tokens;
	std::string temp;
	// Firstly lets tokenize the entire string
	while (std::getline(ss, temp, ' '))
	{
		if (temp.empty())
		{
			// if we have 2 spaces in a row, the user meant to concatenate spaces
			tokens.emplace_back(std::string{ ' ' });
		}
		else
		{
			size_t offset = 0;
			std::stringstream line(temp);
			std::string cleaned;
			// now we have to clean any user entered new line tokens
			while (std::getline(line, cleaned, '\n'))
			{
				if (line.eof() == true)
				{
					// if we have no more text, we can assume that the entire string is completed
					tokens.emplace_back(std::move(cleaned));
					tokens.emplace_back(std::string{ ' ' });
				}
				else
				{
					// add the cleaned text and push in a new line character that the user entered
					tokens.emplace_back(std::move(cleaned));
					tokens.emplace_back(std::string{ '\n' });
				}
			}
		}
	}

	if (tokens.size() && tokens.back().front() == ' ')
	{
		tokens.pop_back();
	}

	int numLines = 1;
	std::vector<float> xStartingOffsets;

	float sizeTaken = 0.0f;
	for (auto token = tokens.begin(); token != tokens.end(); ++token)
	{
		if (token->compare("\n") == 0)
		{
			// if we have a manually entered newline token after cleaning,
			// it means user wants a new line, calculate one with current values and reset

			// handle having spaces at the end of a sentence from the previous iterator			
			if ((&tokens.front() - 1) < (&*token - 1) && std::prev(token)->compare(" ") == 0)
			{
				const auto& gly = fontAtlas->m_characterInfos[L' '];
				float value = (gly.Advance.x) * fontScale;
				sizeTaken -= value;
			}

			++numLines;
			if (ui.format.alignment & (FontAlignment::Centre | FontAlignment::Top_Centre | FontAlignment::Bottom_Centre))
			{
				xStartingOffsets.push_back(sizeTaken / 2.0f);
			}
			else
			{
				xStartingOffsets.push_back(halfBoxX - sizeTaken);
			}
			sizeTaken = 0.0f;
			continue; // go next
		}

		// grab the with of the token
		float textSize = std::accumulate(token->begin(), token->end(), 0.0f, [&](float x, const std::wstring::value_type c)->float
			{
				const auto& gly = fontAtlas->m_characterInfos[c];
				float value = (gly.Advance.x) * fontScale;
				return x + value;
			}
		);

		// now process the token
		if (textSize > boxPixelSizeX)
		{
			//text is much bigger than box, no choice we will just fit it accordingly
			if (sizeTaken == 0.0f)
			{
				// we have a fresh line, just start a new line here
				if (ui.format.alignment & (FontAlignment::Centre | FontAlignment::Top_Centre | FontAlignment::Bottom_Centre))
				{
					xStartingOffsets.push_back(textSize / 2.0f);
				}
				else
				{
					xStartingOffsets.push_back(halfBoxX - textSize);
				}

				if (tokens.size() != 1)
				{
					token->push_back('\n');
					++numLines;
				}
			}
			else
			{
				// we have a line in progress, we must :
				// 1 : clean up the old one and 
				// 2 : start a fresh new line
				if (ui.format.alignment & (FontAlignment::Centre | FontAlignment::Top_Centre | FontAlignment::Bottom_Centre))
				{
					xStartingOffsets.push_back(sizeTaken / 2.0f);
					xStartingOffsets.push_back(textSize / 2.0f);
				}
				else
				{
					xStartingOffsets.push_back(halfBoxX - sizeTaken);
					xStartingOffsets.push_back(halfBoxX - textSize);
				}
				*token = '\n' + *token + '\n';
				numLines += 2;
				sizeTaken = 0.0f;
			}
		}
		else
		{
			// Line is still in progress
			if (textSize + sizeTaken > boxPixelSizeX)
			{
				// We are expected to overflow, so we need to start a new line and continue from there
				if (ui.format.alignment & (FontAlignment::Centre | FontAlignment::Top_Centre | FontAlignment::Bottom_Centre))
				{
					xStartingOffsets.push_back(sizeTaken / 2.0f);
				}
				else
				{
					xStartingOffsets.push_back(halfBoxX - sizeTaken);
				}

				*token = '\n' + *token;
				++numLines;
				// we store the length of the current string as the next starting point
				sizeTaken = textSize;
			}
			else
			{
				// just keep going ...
				sizeTaken += textSize;
			}
		}
	}


	// we set the remaining starting offset
	xStartingOffsets.push_back(sizeTaken);
	if (ui.format.alignment & (FontAlignment::Centre_Right| FontAlignment::Top_Right| FontAlignment::Bottom_Right))
	{
		xStartingOffsets.back() = halfBoxX - xStartingOffsets.back();
	}
	else
	{
		xStartingOffsets.back() /= 2.0f;
	}

	// process starting offsets to get to the right cursor positions
	for (auto& x : xStartingOffsets)
	{
		// old code
		//auto position = ui.position;
		if (ui.format.alignment & (FontAlignment::Centre | FontAlignment::Top_Centre | FontAlignment::Bottom_Centre))
		{
			x = x;
		}
		else
		{
			x = -x;
		}
	}

	float startY{};
	float startX{};
	int xStartIndex = 0;

	// Select formatting along X axis
	if (ui.format.alignment & (FontAlignment::Bottom_Left | FontAlignment::Centre_Left| FontAlignment::Top_Left))
	{
		startX = halfBoxX;
	}
	else
	{
		startX = xStartingOffsets[xStartIndex];
	}

	// Select formatting along Y axis
	if (ui.format.alignment & (FontAlignment::Top_Centre | FontAlignment::Top_Left | FontAlignment::Top_Right))
	{
		// downwards growth is handled for us...
		startY = /*ui.position.y*/ + halfBoxY - fontAtlas->m_characterInfos['L'].Size.y * fontScale;
	}
	else if (ui.format.alignment & (FontAlignment::Bottom_Centre | FontAlignment::Bottom_Left | FontAlignment::Bottom_Right))
	{
		// whereas.. needs to take into account vertical line space to handle upwards growth
		startY = /*ui.position.y*/ - halfBoxY + (std::max(0, numLines - 1) * fontAtlas->m_characterInfos['L'].Size.y * fontScale * ui.format.verticalLineSpace);
	}
	else
	{
		// centre alignment takes into account everything
		const float fullFontSize = fontAtlas->m_characterInfos['L'].Size.y * fontScale;
		const float halfFontSize = fontAtlas->m_characterInfos['L'].Size.y * fontScale / 2.0f;
		const float halfLines = std::max(0.0f,float(numLines-1) / 2);
		startY = /*ui.position.y*/ -halfFontSize + halfLines * fullFontSize * ui.format.verticalLineSpace;
	}


	glm::vec2 cursorPos{ startX, startY };
	for (const auto& token : tokens)
	{
		// go through all our strings and fill the font buffer
		for (const auto& c : token)
		{
			//get our glyph of this char
			const oGFX::Font::Glyph& glyph = fontAtlas->m_characterInfos[c];

			if (c == '\n')
			{
				if (ui.format.alignment & (FontAlignment::Centre_Left| FontAlignment::Top_Left| FontAlignment::Bottom_Left))
				{
					// provide left alignment which is default
					cursorPos.x = startX;
				}
				else
				{
					// provide custom alignment
					cursorPos.x = xStartingOffsets[++xStartIndex];
				}

				// start new line
				cursorPos.y -= glyph.Size.y * /*ui.scale.y * */ ui.format.verticalLineSpace * fontScale;
				continue;
			}


			// calculating glyph positions..
			float xpos = cursorPos.x - glyph.Bearing.x * /*ui.scale.x * */ fontScale;
			float ypos = cursorPos.y - (glyph.Size.y - glyph.Bearing.y) * /*ui.scale.y * */ fontScale;
			ypos = cursorPos.y + (glyph.Bearing.y) * /*ui.scale.y * */ fontScale;

			float w = glyph.Size.x * /*ui.scale.x * */ fontScale;
			float h = glyph.Size.y * /*ui.scale.y * */ fontScale;

			//note position plus scale is already done here
			std::array<glm::vec4, 4> verts = {
				glm::vec4{xpos,    ypos,     0.0f, 1.0f},
				glm::vec4{xpos,    ypos + h, 0.0f, 1.0f},
				glm::vec4{xpos- w ,ypos + h, 0.0f, 1.0f},
				glm::vec4{xpos- w ,ypos,     0.0f, 1.0f},
			};
			cursorPos.x -= (glyph.Advance.x) * /*ui.scale.x * */ fontScale;  // bitshift by 6 to get value in pixels (2^6 = 64)

																		 // Reference : constexpr glm::vec2 textureCoords[] = { { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } };
			std::array<glm::vec2, 4> textureCoords = {
				glm::vec2{ glyph.textureCoordinates.x, glyph.textureCoordinates.y},
				glm::vec2{ glyph.textureCoordinates.x, glyph.textureCoordinates.w},
				glm::vec2{ glyph.textureCoordinates.z, glyph.textureCoordinates.w},
				glm::vec2{ glyph.textureCoordinates.z, glyph.textureCoordinates.y},
			};

			constexpr size_t quadVertexCount = 4;
			for (size_t i = 0; i < quadVertexCount; i++)
			{
				oGFX::UIVertex vert;
				vert.pos = mdl_xform * verts[i];
				vert.pos.w = -1.0; // neagtive is font
				vert.col = ui.colour;
				vert.tex = glm::vec4(textureCoords[i], fontAtlas->m_atlasID, ui.entityID);
				m_uiVertices.push_back(vert);
			}

		}
	}


}
