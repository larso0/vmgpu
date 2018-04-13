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

	bpScene::Mesh mesh;
	bpScene::Model model;
	bpScene::Node node;
};

#endif
