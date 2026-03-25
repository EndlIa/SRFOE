#pragma once

#include <string>
#include "Vector.hpp"
#include <cmath>
#include <algorithm>
#include "stb_image.h"

class Texture {
public:
	Texture(const std::string &filename)
		: data(nullptr), width(0), height(0), channels(0)
	{
		data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
	}

	~Texture()
	{
		if (data) stbi_image_free(data);
	}

	bool isValid() const { return data != nullptr; }

	Vector3f getColor(float u, float v) const
	{
		if (!data || width <= 0 || height <= 0) return Vector3f(0.0f);
		// wrap
		u = u - floorf(u);
		v = v - floorf(v);
		// flip v to image coords
		v = 1.0f - v;
		int x = std::min(width - 1, std::max(0, int(u * width)));
		int y = std::min(height - 1, std::max(0, int(v * height)));
		int idx = (y * width + x) * channels;
		float r = 0, g = 0, b = 0;
		if (channels >= 1) r = data[idx] / 255.0f;
		if (channels >= 2) g = data[idx + 1] / 255.0f;
		if (channels >= 3) b = data[idx + 2] / 255.0f;
		return Vector3f(r, g, b);
	}

	int width;
	int height;
	int channels;
private:
	unsigned char *data;
};
