#include "Scene.h"

void Scene::load(Options& options)
{
	if (options.basic)
	{
		mesh.loadObj(options.objPath);
	} else
	{
		model.loadObj(options.objPath);
	}
}