#include "Scene.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <boost/filesystem.hpp>
#include "ThreadPool.h"

namespace fs = boost::filesystem;

using namespace std;
using namespace bpScene;

void Scene::load(Options& options)
{
	vector<string> fileNames;

	ThreadPool pool{thread::hardware_concurrency()};

	if (options.list)
	{
		ifstream file(options.path);
		copy(istream_iterator<string>(file), istream_iterator<string>(),
		     back_inserter(fileNames));
	} else if (options.directory)
	{
		fs::path path{options.path};
		for (fs::directory_iterator i{path}; i != fs::directory_iterator{}; ++i)
		{
			if (fs::is_regular_file(i->path()) && i->path().extension() == ".obj")
			{
				fileNames.push_back(i->path().string());
			}
		}
	} else
	{
		fileNames.push_back(options.path);
	}

	if (options.maxObjCount > 0 && fileNames.size() > options.maxObjCount)
	{
		fileNames.resize(options.maxObjCount);
	}

	if (options.basic)
	{
		meshes.resize(fileNames.size());
		auto loadFlags = Mesh::LoadFlags{};
		if (!options.generateNormals)
		{
			loadFlags << Mesh::NORMAL;
		}

		vector<future<void>> futures;

		for (auto i = 0; i < fileNames.size(); i++)
		{
			loadMessageEvent("Loading \"" + fileNames[i] + "\"...");
			futures.push_back(pool.enqueue([this, i, &fileNames, &loadFlags]{
				meshes[i].loadObj(fileNames[i], loadFlags);
			}));
		}

		for (auto i = 0; i < fileNames.size(); i++)
		{
			futures[i].wait();
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
		models.resize(fileNames.size());
		auto loadFlags = Mesh::LoadFlags{} << Mesh::TEXTURE_COORDINATE;
		if (!options.generateNormals)
		{
			loadFlags << Mesh::NORMAL;
		}
		for (auto i = 0; i < fileNames.size(); i++)
		{
			loadMessageEvent("Loading \"" + fileNames[i] + "\"...");
			models[i].loadObj(fileNames[i], loadFlags);
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