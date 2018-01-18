#include <bpView/bpView.h>
#include <bpView/Window.h>
#include <bp/Instance.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include "Options.h"
#include "Renderer.h"
#include "SingleRenderer.h"
#include "MultiRenderer.h"
#include "SortFirstBorderlessRenderer.h"

using namespace bp;
using namespace bpScene;
using namespace std;

int main(int argc, char** argv)
{
	Options options;
	try { options = parseOptions(argc, argv); } catch (int e) { return e; }

	Mesh mesh;
	cout << "Loading '" << options.objPath << "'" << endl;
	mesh.loadObj(options.objPath, FlagSet<Mesh::LoadFlags>()
		<< Mesh::LoadFlags::POSITION
		<< Mesh::LoadFlags::NORMAL);
	cout << "Initializing renderer" << endl;

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

	switch (options.strategy)
	{
	case Strategy::Single:
		renderer = new SingleRenderer();
		break;
	case Strategy::SortFirst:
	{
		MultiRenderer* slr = new MultiRenderer();
		slr->setStrategy(MultiRenderer::Strategy::SORT_FIRST);
		slr->setDeviceCount(options.deviceCount);
		renderer = slr;
		break;
	}
	case Strategy::SortLast:
	{
		MultiRenderer* slr = new MultiRenderer();
		slr->setStrategy(MultiRenderer::Strategy::SORT_LAST);
		slr->setDeviceCount(options.deviceCount);
		renderer = slr;
		break;
	}
	case Strategy::SortFirstBorderlessWindowCompositing:
	{
		SortFirstBorderlessRenderer* r = new SortFirstBorderlessRenderer();
		r->setDeviceCount(options.deviceCount);
		renderer = r;
		break;
	}
	}

	renderer->init(instance, options.width, options.height, mesh);

	if (options.strategy == Strategy::SortFirst || options.strategy == Strategy::SortLast)
	{
		MultiRenderer* slr = static_cast<MultiRenderer*>(renderer);
		slr->setColor(1, {0.f, 1.f, 0.f});
		if (slr->getDeviceCount() > 2)
			slr->setColor(2, {0.f, 0.f, 1.f});
	}

	cout << "Rendering" << endl;

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
			cout << '\r' << setprecision(4) << fps << " FPS" << flush;
		}

		renderer->update(delta);
	}

	delete renderer;

	cout << endl;

	return 0;
}