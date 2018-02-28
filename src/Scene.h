#ifndef VMGPU_SCENE_H
#define VMGPU_SCENE_H

#include <bpScene/Node.h>
#include <bpScene/Camera.h>

class Scene
{
public:
	Scene() :
		cameraNode{&sceneRoot},
		camera{&cameraNode}
	{
		cameraNode.translate(0.f, 0.f, 2.f);
		nodes.reserve(2);
		nodes.emplace_back(&sceneRoot);
		nodes.emplace_back(&sceneRoot);
		nodes[0].translate(-0.5f, 0.f, 0.f);
		nodes[1].translate(0.5f, 0.f, 0.f);
		sceneRoot.update();
		camera.update();
	}

	void update(float delta)
	{
		nodes[0].rotate(delta, {0.f, 1.f, 0.f});
		nodes[1].rotate(delta, {0.f, 1.f, 0.f});
		sceneRoot.update();
	}

	bpScene::Node sceneRoot;
	bpScene::Node cameraNode;
	bpScene::Camera camera;
	std::vector<bpScene::Node> nodes;
};

#endif