#ifndef VMGPU_SCENE_H
#define VMGPU_SCENE_H

#include "Options.h"
#include <bpScene/Node.h>
#include <bpScene/Mesh.h>
#include <bpScene/Model.h>

class Scene
{
public:
	void load(Options& options);

	std::vector<bpScene::Mesh> meshes;
	std::vector<bpScene::Model> models;
	bpScene::Node node;
};

#endif
