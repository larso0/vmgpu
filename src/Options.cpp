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
		("list,l", "specifies that the passed file is a list of obj files to load")
		("file,f", po::value<string>(), "file to load")
		("count,c", po::value<uint32_t>()->default_value(2),
		 "device count (how many devices/gpus to use)")
		("simulate-mgpu", "similate the use of more GPUs than available")
		("basic,b", "use basic rendering of mesh (will not load materials)");
	po::variables_map arguments;
	po::store(po::parse_command_line(argc, argv, options), arguments);

	if (arguments.count("help"))
	{
		cout << options << endl;
		throw 1;
	}

	result.basic = arguments.count("basic") > 0;
	result.simulateMultiGPU = arguments.count("simulate-mgpu") > 0;
	result.objList = arguments.count("list") > 0;

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
			result.strategy = Strategy::SortFirst;
		} else if (strategy == "sort-last")
		{
			result.strategy = Strategy::SortLast;
		} else
		{
			cerr << "Unknown rendering strategy, using single as fallback strategy."
			     << endl;
			result.strategy = Strategy::Single;
		}
	}

	if (result.strategy != Strategy::Single && arguments.count("count"))
	{
		result.deviceCount = arguments["count"].as<uint32_t>();
		if (result.deviceCount < 2) result.strategy = Strategy::Single;
	}

	if (result.strategy == Strategy::Single) result.deviceCount = 1;

	if (arguments.count("file"))
	{
		result.objPath = arguments["file"].as<string>();
	} else
	{
		cerr << "No file was specified.\n";
		cout << options << endl;
		throw 2;
	}

	return result;
}
