#pragma once

#include "string.h"

struct Image;

class ImageLoader {
public:
	ImageLoader();
	~ImageLoader();

	bool IsLoaded() const;
	Image* LoadTga(const String& path) const;
	void FreeTga(Image* image) const;

private:
	bool ContainsTGAFooter(struct binary_info* pBinary) const;

};