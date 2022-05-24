#ifndef COMPOSITE_SPRITE_HPP
#define COMPOSITE_SPRITE_HPP
#include "GoblEngine.hpp"

namespace cSpr
{
	class CompositeSprite 
	{
	private:
		gobl::Sprite* sprites = nullptr;
		unsigned char sprLen = 0;
		gobl::GoblEngine* ge = nullptr;
		Vec2* offsets = nullptr;

		bool reverseRender = false;

	public:
		// Constructor & Destructor
		CompositeSprite() = default;
		~CompositeSprite() 
		{
			delete[] sprites;
		}

		void CreateSprites(std::string* dirs, unsigned char dirLen);

		void Draw();
		void DrawRelative(gobl::Camera* cam);

		CompositeSprite& SetPosition(Vec2 pos);
		CompositeSprite& SetFlipped(bool flipped);

		void SetDimensions(IntVec2 dim);
		void SetOffsets(Vec2* offsets) { this->offsets = offsets; }
		void SetSprites(int* sprIndexs);
		void SetEngine(gobl::GoblEngine* ge) { this->ge = ge; }
		unsigned char GetSprLen() { return sprLen; }

		void SetReverseRenderOrder(bool reverse) { reverseRender = reverse; }
	};
}

#endif // !COMPOSITE_SPRITE_HPP