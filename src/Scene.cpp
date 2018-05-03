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

static void loadMesh(Mesh& mesh, const string& file, const Mesh::LoadFlags& flags)
{
	mesh.loadObj(file, flags);
}

void Scene::load(Options& options)
{
	vector<string> filesNames;

	ThreadPool pool{thread::hardware_concurrency()};

	if (options.list)
	{
		ifstream file(options.path);
		copy(istream_iterator<string>(file), istream_iterator<string>(),
		     back_inserter(filesNames));
	} else if (options.directory)
	{
		fs::path path{options.path};
		for (fs::directory_iterator i{path}; i != fs::directory_iterator{}; ++i)
		{
			if (fs::is_regular_file(i->path()) && i->path().extension() == ".obj")
			{
				filesNames.push_back(i->path().string());
			}
		}
	} else
	{
		filesNames.push_back(options.path);
	}

	if (options.maxObjCount > 0 && filesNames.size() > options.maxObjCount)
	{
		filesNames.resize(options.maxObjCount);
	}

	if (options.basic)
	{
		meshes.resize(filesNames.size());
		auto loadFlags = Mesh::LoadFlags{};
		if (!options.generateNormals)
		{
			loadFlags << Mesh::NORMAL;
		}

		vector<future<void>> futures;

		for (auto i = 0; i < filesNames.size(); i++)
		{
			loadMessageEvent("Loading \"" + filesNames[i] + "\"...");
			futures.push_back(pool.enqueue(loadMesh, meshes[i], filesNames[i],
						       loadFlags));
		}

		for (auto i = 0; i < filesNames.size(); i++)
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
		models.resize(filesNames.size());
		auto loadFlags = Mesh::LoadFlags{} << Mesh::TEXTURE_COORDINATE;
		if (!options.generateNormals)
		{
			loadFlags << Mesh::NORMAL;
		}
		for (auto i = 0; i < filesNames.size(); i++)
		{
			loadMessageEvent("Loading \"" + filesNames[i] + "\"...");
			models[i].loadObj(filesNames[i], loadFlags);
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