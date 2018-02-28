#ifndef VMGPU_RENDERER_H
#define VMGPU_RENDERER_H

#include <bp/Instance.h>
#include <bpScene/Mesh.h>
#include <bpScene/Camera.h>
#include "Scene.h"

class Renderer
{
public:
	virtual ~Renderer() = default;
	virtual void init(bp::Instance& instance, uint32_t width, uint32_t height,
			  bpScene::Mesh& mesh, Scene& scene) = 0;
	virtual void render() = 0;
	virtual bool shouldClose() = 0;
};

#endif