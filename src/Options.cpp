#include "Options.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
using namespace std;

Options parseOptions(int argc, char** argv)
{
	Options result;

	po::options_description options;
	options.add_options()
		("help", "print help message")
		("resolution,r", po::value<string>()->default_value("1024x768"),
		 "resolution of window: <width>x<height>")
		("strategy,s", po::value<string>()->default_value("single"),
		 "strategy for rendering: single, sort-first, or sort-last")
		("borderless,b", "use borderless windows for compositing with sort-first method")
		("file,f", po::value<string>(), "obj file to load 3D mesh from")
		("count,c", po::value<uint32_t>()->default_value(2),
		 "device count (how many devices/gpus to use)");
	po::variables_map arguments;
	po::store(po::parse_command_line(argc, argv, options), arguments);

	if (arguments.count("help"))
	{
		cout << options << endl;
		throw 1;
	}

	{
		string res = arguments["resolution"].as<string>();
		int n = sscanf(res.c_str(), "%ux%u", &result.width, &result.height);
		if (n != 2)
		{
			cerr << "Could not parse resolution, "
			     << "using fallback resolution of 1024x768." << endl;
			result.width = 1024;
			result.height = 768;
		}
	}

	{
		string strategy = arguments["strategy"].as<string>();
		if (strategy == "single") result.strategy = Strategy::Single;
		else if (strategy == "sort-first")
		{
			if (arguments.count("borderless"))
				result.strategy = Strategy::SortFirstBorderlessWindowCompositing;
			else
				result.strategy = Strategy::SortFirst;
		} else if (strategy == "sort-last")
		{
			result.strategy = Strategy::SortLast;
		} else
		{
			cerr << "Unknown rendering strategy, using single as fallback strategy." << endl;
			result.strategy = Strategy::Single;
		}
	}

	if (result.strategy != Strategy::Single && arguments.count("count"))
	{
		result.deviceCount = arguments["count"].as<uint32_t>();
		if (result.deviceCount < 2) result.strategy = Strategy::Single;
	}

	if (arguments.count("file"))
	{
		result.objPath = arguments["file"].as<string>();
	}

	return result;
}