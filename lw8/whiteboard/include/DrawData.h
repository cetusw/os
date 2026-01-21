#ifndef WHITEBOARD_DRAWDATA_H
#define WHITEBOARD_DRAWDATA_H

#include <cstdint>
// TODO размеры полей. static_assert.
#pragma pack(push, 1)

struct DrawData
{
	float startX;
	float startY;
	float endX;
	float endY;
	uint32_t color;

	DrawData() = default;

	DrawData(const float sx, const float sy, const float ex, const float ey, const uint32_t c)
		: startX(sx)
		, startY(sy)
		, endX(ex)
		, endY(ey)
		, color(c)
	{
	}
};

#pragma pack(pop)

static_assert(sizeof(DrawData) == 20, "DrawData size must be 20 bytes");

#endif // WHITEBOARD_DRAWDATA_H