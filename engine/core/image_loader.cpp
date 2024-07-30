#include "image_loader.h"

#include "image.h"
#include "platform/platform.h"
#include "core/logger.h"

struct BGR {
	uint8_t b;
	uint8_t g;
	uint8_t r;
};

struct RGB {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct BGRA {
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
};

struct ARGB {
	uint8_t a;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

namespace TGA {

#pragma pack(push, 1)
/**
 * @ref http://www.paulbourke.net/dataformats/tga/ 
 */
struct TGAHeader {
	/* idlength is the length of a string located located after the header. */
	uint8_t idLength;
	char colorMapType;
	/* targa format */
	char dataTypeCode;
	int16_t colorMapOrigin;
	int16_t colorMapLength;
	char colorMapEntrySize;
	int16_t originX;
	int16_t originY;
	int16_t width;
	int16_t height;
	/* self-explanatory name. we will accept only 24 or 32 bits (3 rgb) (4 rgba)*/
	char bitsPerPixel;
	char attributeBitsPerPixel : 4;
	char bottomBit : 1;
	char topBit : 1;
	char reserved : 2;
};

struct TGAFooter {
	uint32_t extensionAreaOffset;
	uint32_t developerDirectoryOffset;
	char signature[16];
};

#pragma pack(pop)

enum TGAImageType {
	NoImage,
	UncompressedColorMapped,
	UncompressedTrueColor,
	UncompressedBlackAndWhite,
	RunLengthEncodedColorMapped = 9,
	RunLengthEncodedTrueColor,
	RunLengthEncodedBlackAndWhite
};

TGAImageType FindImageType(TGAHeader* header);

void DecodeUncompressedTrueColor(TGAHeader* header, uint64_t imageDataOffset, BGRA** outPixels);

} // namespace TGA

#define DEC_POINTER(pointer, offset_bytes) ((reinterpret_cast<uint8_t*>(pointer)) - offset_bytes)
#define INC_POINTER(pointer, offset_bytes) ((reinterpret_cast<uint8_t*>(pointer)) + offset_bytes)

ImageLoader::ImageLoader() {
	
}

ImageLoader::~ImageLoader() {
}

Image* ImageLoader::LoadTga(const String& path) const {
	static_assert(sizeof(TGA::TGAHeader) == 18, "size of TGAHeader is not 18 bytes, you're compiler is probably aligning it.");

	String fileExtension = path.GetFileExtension();

	binary_info imageBinary = Platform::OpenBinary(path.CStr());

	TGA::TGAHeader* header = (TGA::TGAHeader*)imageBinary.binary;

	// Extract TGAFooter, if available
	TGA::TGAFooter* footerPtr = (TGA::TGAFooter*)INC_POINTER(imageBinary.binary, imageBinary.size - 26);

	if (strncmp(footerPtr->signature, "TRUEVISION-XFILE", 16)) {
		// if footer signature doesn't match, it means we don't
		// have a footer.
		footerPtr = nullptr;
	}

	HANDLE pImageInformation = nullptr;
	uint64_t currentOffset = 0;

	currentOffset += sizeof(TGA::TGAHeader);

	if (header->idLength > 0) {
		pImageInformation = INC_POINTER(imageBinary.binary, currentOffset);
	}

	currentOffset += header->idLength;

	HANDLE pColorMapData = nullptr;
	int16_t colorMapDataSize = header->colorMapEntrySize * header->colorMapLength;

	if (colorMapDataSize > 0) {
		currentOffset += colorMapDataSize;
		pColorMapData = INC_POINTER(imageBinary.binary, currentOffset);
	}

	currentOffset += colorMapDataSize;

	TGA::TGAImageType type = TGA::FindImageType(header);

	int64_t imageDataSize = header->width * header->height * header->bitsPerPixel;

	BGRA* pPixels = nullptr;

	switch (type) {
		case TGA::TGAImageType::UncompressedColorMapped: {
			break;
		}
		case TGA::TGAImageType::UncompressedTrueColor: {
			TGA::DecodeUncompressedTrueColor(header, currentOffset, &pPixels);
			break;
		}
		case TGA::TGAImageType::UncompressedBlackAndWhite: {
			break;
		}
		case TGA::TGAImageType::RunLengthEncodedColorMapped: {
			break;
		}
		case TGA::TGAImageType::RunLengthEncodedTrueColor: {
			break;
		}
		case TGA::TGAImageType::RunLengthEncodedBlackAndWhite: {
			break;
		}
		default: {
			Logger::debug("Invalid TGA Format");
			return nullptr;
		}
	}

	Image* image = nullptr;

	if (pPixels != nullptr) {
		image = new Image(ImageFormat::TGA, pPixels, 4, header->width, header->height);
	}

	Platform::CloseBinary(&imageBinary);

	Logger::debug("");

	return image;
}

void ImageLoader::FreeTga(Image* image) const {
	Platform::UFree(image->pImage);
	memset(image, 0, sizeof(*image));
}

namespace TGA {

TGAImageType FindImageType(TGAHeader* header) {
	if (header->colorMapType == 1 && header->dataTypeCode == 1) {
		return TGAImageType::UncompressedColorMapped;
	} else if (header->colorMapType == 0 && header->dataTypeCode == 2) {
		return TGAImageType::UncompressedTrueColor;
	} else if (header->colorMapType == 0 && header->dataTypeCode == 3) {
		return TGAImageType::UncompressedBlackAndWhite;
	} else if (header->colorMapType == 1 && header->dataTypeCode == 9) {
		return TGAImageType::RunLengthEncodedColorMapped;
	} else if (header->colorMapType == 0 && header->dataTypeCode == 10) {
		return TGAImageType::RunLengthEncodedTrueColor;
	} else if (header->colorMapType == 0 && header->dataTypeCode == 11) {
		return TGAImageType::RunLengthEncodedBlackAndWhite;
	}
	return TGAImageType::NoImage;
}

void DecodeUncompressedTrueColor(TGAHeader* header, uint64_t imageDataOffset, BGRA** outPixels) {
	uint64_t outPixelsSize = header->width * header->height * 4;
	*outPixels = (BGRA*)Platform::UAlloc(outPixelsSize);
	
	if (header->bitsPerPixel == 32) {
		ARGB* sourcePixelArray = (ARGB*)INC_POINTER(header, imageDataOffset);
		for (uint64_t i = header->width * header->height; i > 0; i--) {
			unsigned char* sPixel = (unsigned char*)&sourcePixelArray[i];
			BGRA& dPixel = (*outPixels)[i];

			dPixel.a = 255;
			dPixel.r = sPixel[2];
			dPixel.g = sPixel[1];
			dPixel.b = sPixel[0];

			(*outPixels)[i].a = sPixel[3];
			(*outPixels)[i].r = sPixel[2];
			(*outPixels)[i].g = sPixel[1];
			(*outPixels)[i].b = sPixel[0];
		}
	} else if (header->bitsPerPixel == 24) {
		RGB* sourcePixelArray = (RGB*)INC_POINTER(header, imageDataOffset);
		uint64_t iDest = 0;
		for (uint64_t i = header->width * header->height; i > 0; i--) {
			unsigned char* sPixel = (unsigned char*)&sourcePixelArray[iDest++];
			BGRA& dPixel = (*outPixels)[i];

			dPixel.a = 255;
			dPixel.r = sPixel[2];
			dPixel.g = sPixel[1];
			dPixel.b = sPixel[0];

			(*outPixels)[i].a = 255;
			(*outPixels)[i].r = sPixel[2];
			(*outPixels)[i].g = sPixel[1];
			(*outPixels)[i].b = sPixel[0];
		}
	} else if (header->bitsPerPixel == 16) {
		uint16_t* sourcePixelArray = (uint16_t*)INC_POINTER(header, imageDataOffset);
		for (uint64_t i = header->width * header->height; i > 0; i--) {
			unsigned char* sPixel = (unsigned char*)&sourcePixelArray[i];
			BGRA& dPixel = (*outPixels)[i];

			(*outPixels)[i].a = (sPixel[1] & 0x80);
			(*outPixels)[i].r = (sPixel[1] & 0x7c) << 1;
			(*outPixels)[i].g = ((sPixel[1] & 0x03) << 6) | ((sPixel[0] & 0xe0) >> 2);
			(*outPixels)[i].b = (sPixel[0] & 0x1f) << 3;
		}
	}
}

} // namespace TGA