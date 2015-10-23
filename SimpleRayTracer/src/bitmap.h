#pragma once

#include <cstdint> // uint16_t, uint32_t, ...

#pragma pack(2)
struct BitmapFileHeader
{
	char bfType[1];
	uint32_t bfSize = 0;
	uint16_t bfReserved1 = 0;
	uint16_t bfReserved2 = 0;
	uint32_t bfOffBits = 0;
};

struct BitmapInfoHeader
{
	uint32_t biSize = 0;
	int32_t biWidth = 0;
	int32_t biHeight = 0;
	uint16_t biPlanes = 0;
	uint16_t biBitCount = 0;
	uint32_t biCompression = 0;
	uint32_t biSizeImage = 0;
	int32_t biXPelsPerMeter = 0;
	int32_t biYPelsPerMeter = 0;
	uint32_t biClrUsed = 0;
	uint32_t biClrImportant = 0;
};

struct BitmapHeader
{
	BitmapFileHeader bmpFileHeader;
	BitmapInfoHeader bmpInfoHeader;
};
#pragma pack()

void writeBitmap(const char *filename, char *data, const size_t width, const size_t height);
