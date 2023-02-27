#include "TexturePacker.h"

namespace oGFX {

TexturePacker::TexturePacker(glm::ivec2 texSize)
	:textureSize{ texSize }
	, rootNode{ std::make_unique<TextureNode>(glm::ivec2{0.0f,0.0f}, glm::ivec2{ INT_MAX ,INT_MAX }) }
	, buffer(static_cast<size_t>(texSize.x)* static_cast<size_t>(texSize.y))
{
}

glm::ivec2 textureSize;
std::unique_ptr<TextureNode> rootNode;
std::vector<uint32_t> buffer;

// Add object to the tree
TextureNode* TexturePacker::pack(TextureNode* node, const glm::ivec2& imageSize, uint16_t c)
{
	if (!node->empty)
	{
		// The node is filled, not gonna fit anything else here
		assert(!node->left && !node->right);
		return NULL;
	}
	else if (node->left && node->right)
	{
		// Non-leaf, try inserting to the left ..
		TextureNode* retval = pack(node->left.get(), imageSize, c);
		if (retval != NULL)
		{
			return retval;
		}
		// .. and then to the right
		return pack(node->right.get(), imageSize, c);
	}
	else
	{
		// This is an unfilled leaf - let's see if we can fill it
		glm::ivec2 realSize(node->size.x, node->size.y); //unfilled box

														 // If we're along a boundary, calculate the actual imageSize
		if (node->origin.x + node->size.x == INT_MAX)
		{
			realSize.x = textureSize.x - node->origin.x;
		}
		if (node->origin.y + node->size.y == INT_MAX)
		{
			realSize.y = textureSize.y - node->origin.y;
		}

		if (node->size.x == imageSize.x && node->size.y == imageSize.y)
		{
			// Perfect imageSize - just pack into this node
			node->empty = false;
			return node;
		}
		else if (realSize.x < imageSize.x || realSize.y < imageSize.y)
		{
			// Not big enough
			return NULL;
		}
		else
		{
			// Large enough - split until we get a perfect fit

			// Determine how much space we'll have left if we split each way
			int remainX = realSize.x - imageSize.x;
			int remainY = realSize.y - imageSize.y;

			// Split the way that will leave the most room
			bool verticalSplit = remainX < remainY;
			if (remainX == 0 && remainY == 0)
			{
				// Edge case - we are are going to hit the border of
				// the textureID atlas perfectly, split at the border instead
				if (node->size.x > node->size.y)
				{
					verticalSplit = false;
				}
				else
				{
					verticalSplit = true;
				}
			}

			if (verticalSplit)
			{
				// Split vertically (left is top)
				node->left = std::make_unique<TextureNode>(node->origin, glm::ivec2(node->size.x, imageSize.y));
				node->right = std::make_unique<TextureNode>(glm::ivec2(node->origin.x, node->origin.y + imageSize.y),
					glm::ivec2(node->size.x, node->size.y - imageSize.y));
			}
			else
			{
				// Split horizontally
				node->left = std::make_unique<TextureNode>(node->origin, glm::ivec2(imageSize.x, node->size.y));
				node->right = std::make_unique<TextureNode>(glm::ivec2(node->origin.x + imageSize.x, node->origin.y), glm::ivec2(node->size.x - imageSize.x, node->size.y));
			}

			return pack(node->left.get(), imageSize, c);
		}
	}
}

// Query space
glm::ivec2 TexturePacker::packTexture(uint32_t* textureBuffer, const glm::ivec2& imageSize, uint16_t c)
{
	auto bufferedImageSize = imageSize + glm::ivec2{ 1,1 }; // pack slightly loosely to avoid artefacting

	TextureNode* node = pack(rootNode.get(), bufferedImageSize, c);
	if (node == NULL)
	{
		this->resizeBuffer(glm::ivec2(textureSize.x * 2, textureSize.y * 2));
		node = pack(rootNode.get(), bufferedImageSize, c);

		// Note: this assert will be hit if we try to pack a texture larger
		// than the current imageSize of the texture
		assert(node != NULL);
	}

	assert(bufferedImageSize.x == node->size.x);
	assert(bufferedImageSize.y == node->size.y);

	// Copy the texture to the texture atlas' buffer
	for (size_t ly = 0; ly < imageSize.y; ly++)
	{
		for (size_t lx = 0; lx < imageSize.x; lx++)
		{
			size_t y = node->origin.y + ly;
			size_t x = node->origin.x + lx;
			this->buffer[y * textureSize.x + x] = textureBuffer[ly * imageSize.x + lx];
		}
	}

	return node->origin;
}


void TexturePacker::resizeBuffer(const glm::ivec2& newSize)
{
	std::vector<uint32_t> newBuffer;
	newBuffer.resize(static_cast<size_t>(newSize.y) * newSize.x);
	if (newBuffer.size() > buffer.size())
	{
		for (size_t y = 0; y < textureSize.y; y++)
		{
			for (size_t x = 0; x < textureSize.x; x++)
			{
				newBuffer[y * newSize.x + x] = buffer[y * textureSize.x + x];
			}
		}
	}
	rootNode->size = { newSize };
	textureSize = newSize;
	buffer = std::move(newBuffer);
}


}// end namespace oGFX