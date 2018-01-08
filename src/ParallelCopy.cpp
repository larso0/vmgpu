#include "ParallelCopy.h"
#include <future>
#include <vector>
#include <cstring>

using namespace std;

void parallelCopy(initializer_list<CopyJob> copyJobs, size_t chunkSize)
{
	vector<future<void>> futures;

	for (auto job : copyJobs)
	{
		size_t remaining = job.size;
		while (remaining > 0)
		{
			size_t copySize;
			if (remaining < chunkSize) copySize = remaining;
			else copySize = chunkSize;

			futures.push_back(async(launch::async, [job, copySize]{
				memmove(job.dst, job.src, copySize);
			}));

			remaining -= copySize;
		}
	}

	for (auto& f : futures) f.wait();
}