#pragma once

class Image
{
public:
	Image();
	~Image();

public:
	void load(const std::string& file);
	static VkFormat getDefaultFormatFromChannelCount(int channels);

private:
	uint32_t width, height;
	VkImage image;
};