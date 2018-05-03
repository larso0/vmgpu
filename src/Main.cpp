#include "Options.h"
#include "Vmgpu.h"
#include <QGuiApplication>
#include <QVulkanInstance>
#include <QLoggingCategory>
#include <bpScene/Mesh.h>
#include <iostream>
#include <iomanip>

using namespace std;
using bpScene::Mesh;

int main(int argc, char** argv)
{
	QGuiApplication app{argc, argv};

	Options options;
	try { options = parseOptions(argc, argv); } catch (int e) { return e; }

	QVulkanInstance instance;

#ifndef NDEBUG
	QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));
	instance.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");
#endif

	if (!instance.create())
		qFatal("Failed to create instance.");

	Vmgpu vmgpu{instance, options};
	vmgpu.setVSync(false);
	bpUtil::connect(vmgpu.framerateEvent, [](float fps){
		cout << "\rFramerate = " << setprecision(4) << fps << " FPS" << flush;
	});
	bpUtil::connect(vmgpu.loadMessageEvent, [](const string& msg){
		cout << msg << endl;
	});
	vmgpu.resize(options.width, options.height);
	vmgpu.show();

	return app.exec();
}
