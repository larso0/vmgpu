#include "Scene.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>

using namespace std;

void Scene::load(Options& options)
{
	vector<string> filesNames;

	if (options.objList)
	{
		ifstream file(options.objPath);
		copy(istream_iterator<string>(file), istream_iterator<string>(),
		     back_inserter(filesNames));
	} else
	{
		filesNames.push_back(options.objPath);
	}

	if (options.basic)
	{
		meshes.resize(filesNames.size());
		for (auto i = 0; i < filesNames.size(); i++)
		{
			loadMessageEvent("Loading \"" + filesNames[i] + "\"...");
			meshes[i].loadObj(filesNames[i]);
			const auto& minV = meshes[i].getMinVertex();
			const auto& maxV = meshes[i].getMaxVertex();
			if (minV.x < minVertex.x) minVertex.x = minV.x;
			if (maxV.x > maxVertex.x) maxVertex.x = maxV.x;
			if (minV.y < minVertex.y) minVertex.y = minV.y;
			if (maxV.y > maxVertex.y) maxVertex.y = maxV.y;
			if (minV.z < minVertex.z) minVertex.z = minV.z;
			if (maxV.z > maxVertex.z) maxVertex.z = maxV.z;
		}
	} else
	{
		models.resize(filesNames.size());
		for (auto i = 0; i < filesNames.size(); i++)
		{
			loadMessageEvent("Loading \"" + filesNames[i] + "\"...");
			models[i].loadObj(filesNames[i]);
			const auto& minV = models[i].getMinVertex();
			const auto& maxV = models[i].getMaxVertex();
			if (minV.x < minVertex.x) minVertex.x = minV.x;
			if (maxV.x > maxVertex.x) maxVertex.x = maxV.x;
			if (minV.y < minVertex.y) minVertex.y = minV.y;
			if (maxV.y > maxVertex.y) maxVertex.y = maxV.y;
			if (minV.z < minVertex.z) minVertex.z = minV.z;
			if (maxV.z > maxVertex.z) maxVertex.z = maxV.z;
		}
	}

	if (options.zUp)
	{
		node.rotate(glm::radians(-90.f), {1.f, 0.f, 0.f});
		node.update();
	}
}