#ifndef VMGPU_RENDERER_H
#define VMGPU_RENDERER_H

#include <bp/Instance.h>
#include <bpScene/Mesh.h>
#include <bpScene/Camera.h>

class Renderer
{
public:
	virtual ~Renderer() = default;
	virtual void init(bp::Instance& instance, uint32_t width, uint32_t height,
			  bpScene::Mesh& mesh) = 0;
	virtual void render() = 0;
	virtual void update(float delta) {}
	virtual bool shouldClose() = 0;
};

#endif