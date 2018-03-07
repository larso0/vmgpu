#ifndef VMGPU_PIPELINESTEP_H
#define VMGPU_PIPELINESTEP_H

#include <vector>

template <typename Input, typename Output>
class PipelineStep
{
public:
	virtual ~PipelineStep() = default;

	void init(unsigned outputCount)
	{
		outputs.resize(outputCount);
		for (unsigned i = 0; i < outputCount; i++) prepare(outputs[i]);
	}

	Output& process(Input& input, unsigned outputIdx)
	{
		process(input, outputs[outputIdx], outputIdx);
		return outputs[outputIdx];
	}

	unsigned getOutputCount() const { return outputs.size(); }

protected:
	virtual void prepare(Output&) {}
	virtual void process(Input& input, Output& output, unsigned outputIndx) = 0;

private:
	std::vector<Output> outputs;
};

#endif
