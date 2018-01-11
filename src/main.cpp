#include <bpView/bpView.h>
#include <bpView/Window.h>
#include <bp/Instance.h>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include "Renderer.h"
#include "SingleRenderer.h"
#include "MultiRenderer.h"

using namespace bp;
using namespace bpScene;
using namespace std;
namespace po = boost::program_options;

enum class Mode
{
	Single,
	SortFirst,
	SortFirstBorderlessWindowCompositing,
	SortLast
};

struct Options
{
	Mode mode;
	string objPath;
	uint32_t width, height;
	uint32_t deviceCount;
};

Options parseOptions(int argc, char** argv)
{
	Options result;

	po::options_description options;
	options.add_options()
		("help", "print help message")
		("resolution,r", po::value<string>()->default_value("1024x768"),
		 "resolution of window: <width>x<height>")
		("mode,m", po::value<string>()->default_value("single"),
		 "mode for rendering: single, sort-first, or sort-last")
		("borderless-window-compositing",
		 "use borderless windows for compositing with sort-last method")
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
		string m = arguments["mode"].as<string>();
		if (m == "single") result.mode = Mode::Single;
		else if (m == "sort-first")
		{
			if (arguments.count("borderless-window-compositing"))
				result.mode = Mode::SortFirstBorderlessWindowCompositing;
			else
				result.mode = Mode::SortFirst;
		} else if (m == "sort-last")
		{
			result.mode = Mode::SortLast;
		} else
		{
			cerr << "Unknown rendering mode, using single as fallback mode." << endl;
			result.mode = Mode::Single;
		}
	}

	if (result.mode != Mode::Single && arguments.count("count"))
	{
		result.deviceCount = arguments["count"].as<uint32_t>();
		if (result.deviceCount < 2) result.mode = Mode::Single;
	}

	if (arguments.count("file"))
	{
		result.objPath = arguments["file"].as<string>();
	} else
	{
		cerr << "Can't load mesh, no obj file was specified.\n";
		cout << options << endl;
		throw 2;
	}

	return result;
}

int main(int argc, char** argv)
{
	Options options;
	try { options = parseOptions(argc, argv); } catch (int e) { return e; }

	Mesh mesh;
	mesh.loadObj(options.objPath, FlagSet<Mesh::LoadFlags>()
		<< Mesh::LoadFlags::POSITION
		<< Mesh::LoadFlags::NORMAL);

	bpView::init();

#ifdef NDEBUG
	bool debugEnabled = false;
#else
	bool debugEnabled = true;
#endif

	Instance instance;

	instance.enableExtensions(bpView::requiredInstanceExtensions.begin(),
				  bpView::requiredInstanceExtensions.end());
	instance.init(debugEnabled);

	auto print = [](const string& s) { cout << s << endl; };
	auto printErr = [](const string& s) { cerr << s << endl; };

	connect(instance.infoEvent, print);
	connect(instance.warningEvent, printErr);
	connect(instance.errorEvent, printErr);
	connect(bpView::errorEvent, instance.errorEvent);

	Renderer* renderer = nullptr;

	switch (options.mode)
	{
	case Mode::Single:
		renderer = new SingleRenderer();
		break;
	case Mode::SortFirst:
	{
		MultiRenderer* slr = new MultiRenderer();
		slr->setStrategy(MultiRenderer::Strategy::SORT_FIRST);
		slr->setDeviceCount(options.deviceCount);
		renderer = slr;
		break;
	}
	case Mode::SortLast:
	{
		MultiRenderer* slr = new MultiRenderer();
		slr->setStrategy(MultiRenderer::Strategy::SORT_LAST);
		slr->setDeviceCount(options.deviceCount);
		renderer = slr;
		break;
	}
	default:
		cerr << "Mode not implemented." << endl;
		return 3;
	}

	renderer->init(instance, options.width, options.height, mesh);

	if (options.mode == Mode::SortFirst || options.mode == Mode::SortLast)
	{
		MultiRenderer* slr = static_cast<MultiRenderer*>(renderer);
		slr->setColor(1, {0.f, 1.f, 0.f});
		if (slr->getDeviceCount() > 2)
			slr->setColor(2, {0.f, 0.f, 1.f});
	}

	double seconds = glfwGetTime();
	double frametimeAccumulator = seconds;
	unsigned frameCounter = 0;
	while (!renderer->shouldClose())
	{
		bpView::pollEvents();
		renderer->render();

		double time = glfwGetTime();
		float delta = static_cast<float>(time - seconds);
		seconds = time;
		if (++frameCounter % 50 == 0)
		{
			double diff = time - frametimeAccumulator;
			frametimeAccumulator = time;
			double fps = 50.0 / diff;
			cout << '\r' << setprecision(4) << fps << "FPS";
		}

		renderer->update(delta);
	}

	delete renderer;

	return 0;
}