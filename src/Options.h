#ifndef VMGPU_OPTIONS_H
#define VMGPU_OPTIONS_H

#include <string>
#include <cstdint>
#include <bpScene/Math.h>

enum class Strategy
{
	Single,
	SortFirst,
	SortLast
};

struct Options
{
	Strategy strategy;
	bool simulateMultiGPU;
	bool basic;
	bool zUp;
	bool objList;
	std::string objPath;
	uint32_t width, height;
	uint32_t deviceCount;
};

Options parseOptions(int argc, char** argv);

#endif