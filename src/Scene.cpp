#include "Scene.h"
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
			meshes[i].loadObj(filesNames[i]);
		}
	} else
	{
		models.resize(filesNames.size());
		for (auto i = 0; i < filesNames.size(); i++)
		{
			models[i].loadObj(filesNames[i]);
		}
	}
}