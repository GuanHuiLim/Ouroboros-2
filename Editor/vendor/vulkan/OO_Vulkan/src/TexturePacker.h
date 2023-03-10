#pragma once

#include "MathCommon.h"
#include <vector>
#include <memory>

namespace oGFX {


struct TextureNode
{
	TextureNode(glm::ivec2 o, glm::ivec2 s) :origin{ o }, size{ s }, empty{ true } {}
	glm::ivec2 origin; // Top left of the rectangle this node represents
	glm::ivec2 size;   // Size of the rectangle this node represents
	bool empty;        // true if this node is a leaf and is filled

	std::unique_ptr<TextureNode> left;  // Left (or top) subdivision
	std::unique_ptr<TextureNode> right; // Right (or bottom) subdivision
};
class TexturePacker
{
public:
	TexturePacker(glm::ivec2 texSize = { 512,512 });

	glm::ivec2 textureSize;
	std::unique_ptr<TextureNode> rootNode;
	std::vector<uint32_t> buffer;

	void resizeBuffer(const glm::ivec2& newSize);
	TextureNode* pack(TextureNode* node, const glm::ivec2& imageSize, uint16_t c);
	glm::ivec2 packTexture(uint32_t* textureBuffer, const glm::ivec2& imageSize, uint16_t c);

};

}// end namespace oGFX
