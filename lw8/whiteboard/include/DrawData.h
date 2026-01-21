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
};
#pragma pack(pop)

static_assert(sizeof(DrawData) == 20, "DrawData size must be 20 bytes");

#endif // WHITEBOARD_DRAWDATA_H