#include "Options.h"
#include "Vmgpu.h"
#include <QGuiApplication>
#include <QVulkanInstance>
#include <QLoggingCategory>
#include <bpScene/Mesh.h>

using bpScene::Mesh;

int main(int argc, char** argv)
{
	QGuiApplication app{argc, argv};

	Options options;
	try { options = parseOptions(argc, argv); } catch (int e) { return e; }

	Mesh mesh;
	qInfo() << "Loading '" << options.objPath.c_str() << "'";
	mesh.loadObj(options.objPath, Mesh::LoadFlags() << Mesh::POSITION << Mesh::NORMAL);
	qInfo() << "Initializing renderer";

	QVulkanInstance instance;

#ifndef NDEBUG
	QLoggingCategory::setFilterRules(QStringLiteral("qt.vulkan=true"));
	instance.setLayers(QByteArrayList() << "VK_LAYER_LUNARG_standard_validation");
#endif

	if (!instance.create())
		qFatal("Failed to create instance.");

	Vmgpu vmgpu{instance};
	vmgpu.resize(options.width, options.height);
	vmgpu.show();

	return app.exec();
}