#include "bitmap.h"

#include <fstream> // std::ofstream
#include <algorithm> // std::min

#include <iostream>

void writeBitmap(const char *filename, const char *data, const size_t width, const size_t height)
{
	BitmapFileHeader bmpFileHeader;
	BitmapInfoHeader bmpInfoHeader;

	// bitmap files always begin with "BM"
	bmpFileHeader.bfType[0] = 'B';
	bmpFileHeader.bfType[1] = 'M';

	bmpFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader); // size of the header

	bmpInfoHeader.biSize = sizeof(BitmapInfoHeader); // size of info portion of the header

	bmpInfoHeader.biWidth = width;
	bmpInfoHeader.biHeight = height;

	// settings for 24-bit bitmap
	bmpInfoHeader.biPlanes = 1;
	bmpInfoHeader.biBitCount = 24;

	bmpInfoHeader.biSizeImage = height * width * 3; // size of the actual color values, 24 bits per pixel

	bmpFileHeader.bfSize = bmpFileHeader.bfOffBits + bmpInfoHeader.biSizeImage; // total size of the file

	BitmapHeader bmpHeader;
	bmpHeader.bmpFileHeader = bmpFileHeader;
	bmpHeader.bmpInfoHeader = bmpInfoHeader;

	// open a file stream and write header out to file
	std::ofstream ofs(filename, std::ios::out | std::ios::binary);
	ofs.write(static_cast<char*>(static_cast<void*>(&bmpHeader)), sizeof(BitmapHeader));

	// write out color values to file
	for (size_t j = height; j > 0; --j)
	{
		// for whatever reason, bitmap lines are stored in reverse order, so we start at the bottom of the image
		size_t line = j - 1;
		for (size_t i = (line * width * 3); i < (3 * width) + (line * width * 3); i += 3)
		{
			// data is in floating point form, needs to be converted to 8 bit color values for our bitmap
			// colors are stored in BGR order, so we need to store them in reverse order
			ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i + 2]) * 255);
			ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i + 1]) * 255);
			ofs << (unsigned char)(std::min(1.0f, ((float*)data)[i]) * 255);
		}
	}
}
