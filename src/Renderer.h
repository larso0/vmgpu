#ifndef VMGPU_RENDERER_H
#define VMGPU_RENDERER_H

#include <bp/Instance.h>
#include <bp/Pointer.h>
#include <bpScene/Mesh.h>
#include <bpScene/Camera.h>

class Renderer
{
public:
	virtual ~Renderer() = default;
	virtual void init(bp::NotNull<bp::Instance> instance, uint32_t width, uint32_t height,
			  bp::NotNull<bpScene::Mesh> mesh, bp::NotNull<bpScene::Node> meshNode,
			  bp::NotNull<bpScene::Camera> camera) = 0;
	virtual void render() = 0;
	virtual bool shouldClose() = 0;
};

#endif