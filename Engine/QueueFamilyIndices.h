#pragma once
#include <optional>

struct QueueFamilyIndices
{
public:
	bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
	uint32_t GetGraphicsFamilyValue() const
	{
		return graphicsFamily.value();
	}
	uint32_t GetPresentFamilyValue() const
	{
		return presentFamily.value();
	}
public:
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
};