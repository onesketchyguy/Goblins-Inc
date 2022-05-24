#include "CompositeSprite.hpp"

namespace cSpr 
{
	// Initialization
	void CompositeSprite::CreateSprites(std::string* dirs, unsigned char dirLen)
	{
		sprites = new gobl::Sprite[dirLen];
		offsets = new Vec2[dirLen];

		for (unsigned char i = 0; i < dirLen; i++)
		{
			ge->CreateSpriteObject(sprites[i], dirs[i].c_str());
		}

		sprLen = dirLen;
	}

	// Draw functions
	void CompositeSprite::Draw()
	{
		if (reverseRender) 
		{
			for (int i = sprLen - 1; i >= 0; i--) sprites[i].Draw();
		}
		else 
		{
			for (unsigned char i = 0; i < sprLen; i++) sprites[i].Draw();
		}
	}

	void CompositeSprite::DrawRelative(gobl::Camera* cam)
	{
		if (reverseRender)
		{
			for (int i = sprLen - 1; i >= 0; i--) sprites[i].DrawRelative(cam);
		}
		else
		{
			for (unsigned char i = 0; i < sprLen; i++) sprites[i].DrawRelative(cam);
		}
	}
	
	// Mutators
	void CompositeSprite::SetSprites(int* sprIndexs)
	{
		for (unsigned char i = 0; i < sprLen; i++) 
			sprites[i].SetSpriteIndex(sprIndexs[i]);
	}

	CompositeSprite& CompositeSprite::SetPosition(Vec2 pos)
	{
		for (unsigned char i = 0; i < sprLen; i++)
			sprites[i].SetPosition(pos + offsets[i]);

		return *this;
	}

	CompositeSprite& CompositeSprite::SetFlipped(bool flipped)
	{
		for (unsigned char i = 0; i < sprLen; i++)
			sprites[i].SetFlipped(flipped);

		return *this;
	}

	void CompositeSprite::SetDimensions(IntVec2 dim)
	{
		for (unsigned char i = 0; i < sprLen; i++)
			sprites[i].SetDimensions(dim);
	}
}