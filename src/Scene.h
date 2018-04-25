#ifndef VMGPU_SCENE_H
#define VMGPU_SCENE_H

#include "Options.h"
#include <bpScene/Node.h>
#include <bpScene/Mesh.h>
#include <bpScene/Model.h>
#include <bpUtil/Event.h>

class Scene
{
public:
	void load(Options& options);

	std::vector<bpScene::Mesh> meshes;
	std::vector<bpScene::Model> models;
	bpScene::Node root;
	bpScene::Node node{&root};
	glm::vec3 minVertex, maxVertex;

	bpUtil::Event<const std::string&> loadMessageEvent;
};

#endif
