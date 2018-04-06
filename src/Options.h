#ifndef VMGPU_OPTIONS_H
#define VMGPU_OPTIONS_H

#include <string>
#include <cstdint>

enum class Strategy
{
	Single,
	SortFirst,
	SortFirstBorderlessWindowCompositing,
	SortLast
};

struct Options
{
	Strategy strategy;
	bool simulateMultiGPU;
	std::string objPath;
	uint32_t width, height;
	uint32_t deviceCount;
};

Options parseOptions(int argc, char** argv);

#endif