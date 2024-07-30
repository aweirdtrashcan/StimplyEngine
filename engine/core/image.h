#pragma once

#include "defines.h"

enum class ImageFormat {
	JPG,
	TGA,
	BMP,
	PNG
};

struct Image {
	const ImageFormat format;
	HANDLE pImage;
	const uint32_t channelCount;
	const uint32_t width;
	const uint32_t height;

	Image(ImageFormat format, HANDLE pImage, uint32_t channelCount, uint32_t width, uint32_t height)
		:
	format(format),
	pImage(pImage),
	channelCount(channelCount),
	width(width),
	height(height) {}
};