#include <bpView/bpView.h>
#include <bpView/Window.h>
#include <bp/Instance.h>
#include <bp/Device.h>
#include <bp/Swapchain.h>
#include <bp/RenderPass.h>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <stdexcept>
#include <cstring>

using namespace bp;
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
		("file,f", po::value<string>(), "obj file to load 3D mesh from");
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

	if (arguments.count("file"))
	{
		result.objPath = arguments["file"].as<string>();
	} else
	{
		cerr << "Can't load mesh, no obj file was specified.\n";
		throw 2;
	}

	return result;
}

int main(int argc, char** argv)
{
	Options options;
	try { options = parseOptions(argc, argv); } catch (int e) { return e; }

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

	static const char* SWAPCHAIN_EXTENSION = "VK_KHR_swapchain";

	Window window{instance, options.width, options.height, "vmgpu", nullptr,
		      FlagSet<Window::Flags>() << Window::Flags::VISIBLE
					       << Window::Flags::DECORATED
					       << Window::Flags::AUTO_ICONIFY};

	DeviceRequirements requirements;
	requirements.queues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.features.samplerAnisotropy = VK_TRUE;
	requirements.features.geometryShader = VK_TRUE;
	requirements.surface = window.getSurface();
	requirements.extensions.push_back(SWAPCHAIN_EXTENSION);

	Device device{instance, requirements};

	Swapchain swapchain;
	swapchain.setClearEnabled(true);
	swapchain.setClearValue({0.2f, 0.2f, 0.2f, 1.f});
	swapchain.init(&device, window, options.width, options.height, false);

	return 0;
}