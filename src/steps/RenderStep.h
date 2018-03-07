#ifndef VMGPU_RENDERSTEP_H
#define VMGPU_RENDERSTEP_H

#include "PipelineStep.h"
#include "../Scene.h"
#include <bp/Texture.h>

class RenderStep : public PipelineStep<Scene, bp::Texture>
{
public:

private:
	void prepare(bp::Texture& output) override;
	void process(Scene& input, bp::Texture& output, unsigned outputIndx) override;
};


#endif
