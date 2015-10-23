#include "bitmap.h"

#include <cstring> // std::memset
#include <fstream> // std::ofstream
#include <algorithm> // std::min

#include <iostream>

void writeBitmap(const char *filename, char *data, const size_t width, const size_t height)
{
	BitmapFileHeader bmpFileHeader;
	BitmapInfoHeader bmpInfoHeader;

	bmpFileHeader.bfType[0] = 'B';
	bmpFileHeader.bfType[1] = 'M';

	bmpFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader); // size of the header for 24-bit bitmap

	bmpInfoHeader.biSize = sizeof(BitmapInfoHeader); // size of info portion of the header for 24-bit bitmap

	bmpInfoHeader.biWidth = width;
	bmpInfoHeader.biHeight = height;

	// settings for 24-bit bitmap
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;

	bmpInfoHeader.biSizeImage = height * width * 3;

	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + bmpInfoHeader.biSizeImage;

	BitmapHeader bmpHeader;
	bmpHeader.bmpFileHeader = bmpFileHeader;
	bmpHeader.bmpInfoHeader = bmpInfoHeader;

	// open a file stream and write header out to file
	std::ofstream ofs(filename, std::ios::out | std::ios::binary);

	ofs.write(static_cast<char*>(static_cast<void*>(&bmpHeader)), sizeof(BitmapHeader));

#if 0
	//Backwards!
	// write pixel data pixel at a time
	for (unsigned i = width * height * 3; i > 0 ; i-=3) {
		ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i + 2]) * 255);
		ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i + 1]) * 255);
		ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i]) * 255);
	}
#endif

	int k = 0;
	for (size_t j = height; j > 0; --j)
	{
		size_t line = j - 1;
		for (size_t i = (line * width * 3); i < (3 * width) + (line * width * 3); i += 3, k += 3)
		{
			ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i + 2]) * 255);
			ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i + 1]) * 255);
			ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i]) * 255);
		}
	}
}
