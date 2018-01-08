#ifndef VMGPU_PARALLELCOPY_H
#define VMGPU_PARALLELCOPY_H

#include <initializer_list>
#include <thread>

struct CopyJob
{
	CopyJob() :
		src{nullptr},
		dst{nullptr},
		size{0} {}
	CopyJob(const void* src, void* dst, size_t size) :
		src{src},
		dst{dst},
		size{size} {}

	const void* src;
	void* dst;
	size_t size;
};

void parallelCopy(std::initializer_list<CopyJob> copyJobs, size_t chunkSize = 8192);

#endif