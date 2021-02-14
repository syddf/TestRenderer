#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Vulkan/Device.h"
#include "Vulkan/Window.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/ResourceCreator.h"
#include "Vulkan/RenderingPipeline.h"
#include "Vulkan/PresentEngine.h"
#include "Vulkan/Scene.h"
#include "Vulkan/ComputePass.h"

#include <stdio.h>
#include <direct.h>
#include <iostream>
#define MAXPATH  1024  

bool gNeedDebugLayer = true;
UInt32 gScreenWidth = 800;
UInt32 gScreenHeight = 600;

VulkanPresentEngine* presentEngine;

void test(VkQueue graphicsQueue, VulkanWindow* window)
{
	//auto anistroMaterial = ResourceCreator::CreateMaterial("test4", "D:\\OfflineRenderer\\Asset\\anisoMipMapVolumeComp.data");
	auto computeMaterial = ResourceCreator::CreateMaterial("test4", "D:\\OfflineRenderer\\Asset\\clearVoxelMapComp.data");
	//auto ttmaterial = ResourceCreator::CreateMaterial("test3", "D:\\OfflineRenderer\\Asset\\voxelizationVert.data", "D:\\OfflineRenderer\\Asset\\voxelizationFrag.data", "D:\\OfflineRenderer\\Asset\\voxelizationGeom.data");
	auto tmaterial = ResourceCreator::CreateMaterial("test2", MaterialMode::Normal, "D:\\OfflineRenderer\\Asset\\renderVoxelVert.data", "D:\\OfflineRenderer\\Asset\\renderVoxelFrag.data", "D:\\OfflineRenderer\\Asset\\renderVoxelGeom.data");
	//auto material = ResourceCreator::CreateMaterial("test1", "D:\\OfflineRenderer\\Asset\\lightVert.data", "D:\\OfflineRenderer\\Asset\\lightFrag.data");
	
	VulkanSceneData * scene = new VulkanSceneData("D:\\Resource\\res\\sponza.data");
	ResourceCreator::CreateDepthStencilAttachment("DepthStencilAttachment", gScreenWidth, gScreenHeight);

	RenderingPipelineNodeDesc voxelizationPass = {};
	voxelizationPass.NodeName = "voxelization";
	voxelizationPass.BindPoint = PipelineBindPoint::BP_GRAPHICS;
	voxelizationPass.AttachToWindowNode = false;
	voxelizationPass.RenderingNodeDescVec = scene->ExportAllRenderingNodeByMaterial("D:\\OfflineRenderer\\Asset\\voxelizationVert.data", "D:\\OfflineRenderer\\Asset\\voxelizationFrag.data", "D:\\OfflineRenderer\\Asset\\voxelizationGeom.data", "Sponza", MaterialMode::NoAttachment);
	
	RenderingPipelineNodeDesc renderVoxelPass = {};
	renderVoxelPass.NodeName = "renderVoxel";
	renderVoxelPass.BindPoint = PipelineBindPoint::BP_GRAPHICS;
	renderVoxelPass.AttachToWindowNode = true;
	renderVoxelPass.FrameBufferDesc.Width = gScreenWidth;
	renderVoxelPass.FrameBufferDesc.Height = gScreenHeight;
	renderVoxelPass.FrameBufferDesc.AttachmentName.push_back("SwapChainImage");
	renderVoxelPass.FrameBufferDesc.AttachmentName.push_back("DepthStencilAttachment");
	AttachmentDesc attachDesc;
	attachDesc.Usage = TextureUsageBits::TU_COLOR_ATTACHMENT;
	attachDesc.LoadOp = AttachmentOperator::AO_CLEAR;
	attachDesc.StoreOp = AttachmentOperator::AO_STORE;
	attachDesc.Format = TextureFormat::TF_B8G8R8A8SRGB;
	renderVoxelPass.FrameBufferLayoutDesc.AttachmentDesc.push_back(attachDesc);

	attachDesc.Usage = TextureUsageBits::TU_DEPTH_STENCIL;
	attachDesc.Format = TextureFormat::TF_D24US8;
	renderVoxelPass.FrameBufferLayoutDesc.AttachmentDesc.push_back(attachDesc);

	RenderingNodeDesc voxelNode = {};
	voxelNode.EmptyVertexCount = 256 * 256 * 256;
	voxelNode.MaterialAddr = (char*)(tmaterial.get());
	voxelNode.World = scene->GetWorldData().get();
	renderVoxelPass.RenderingNodeDescVec.push_back(voxelNode);
	renderVoxelPass.DependingNodeIndex.push_back(0);

	ComputeNodeDesc cnd = {};
	cnd.Invocation = Vec3(256, 256, 256);
	cnd.MaterialAddr = (char*)(computeMaterial.get());
	cnd.World = scene->GetWorldData().get();
	RenderingPipelineNodeDesc clearVoxelPass = {};
	clearVoxelPass.NodeName = "clearVoxel";
	clearVoxelPass.BindPoint = PipelineBindPoint::BP_COMPUTE;
	clearVoxelPass.AttachToWindowNode = false;
	clearVoxelPass.ComputeNodeDescVec.push_back(cnd);
	clearVoxelPass.DependingNodeIndex.push_back(1);

	scene->AddUpdateMaterial("test2");
	scene->AddUpdateMaterial("test4");

	std::vector<RenderingPipelineNodeDesc> pipelineNodesVec;
	pipelineNodesVec.push_back(voxelizationPass);
	pipelineNodesVec.push_back(renderVoxelPass);
	pipelineNodesVec.push_back(clearVoxelPass);

	auto vulkanPipeline = new VulkanRenderingPipeline();
	vulkanPipeline->GenerateRenderingGraph(pipelineNodesVec);

	while (true)
	{
		int imageIndex = presentEngine->AcquireImage();
		scene->UpdateSceneData(imageIndex);
		auto semaphore = vulkanPipeline->SubmitRenderingCommands(imageIndex, graphicsQueue, presentEngine->GetCurrentFrameRenderFinishFence());
		presentEngine->PresentFrame(imageIndex, semaphore);
		if (window->MainLoop())
		{
			vkQueueWaitIdle(graphicsQueue);
			delete vulkanPipeline;
			break;
		}
	}

}

void compileShader()
{
	std::vector<std::string> shaderFiles = 
	{
		"voxelization.vert",
		"voxelization.frag",
		"voxelization.geom",
		"renderVoxel.vert",
		"renderVoxel.frag",
		"renderVoxel.geom",
		"clearVoxelMap.comp",
		"averageVoxelMap.comp",
		"anisoMipMapVolume.comp"
	};
	std::vector<std::string> newShaderPathVec;
	char buffer[MAXPATH];
	_getcwd(buffer, MAXPATH);
	std::string path = buffer;
	path += "\\.\\..\\..\\Asset\\";

	std::string batchFile = path + "shaderCompile.bat";
	std::ofstream outFile;
	outFile.open(batchFile);
	std::string compilerFile = path + "glslc.exe";
	for (auto str : shaderFiles)
	{
		std::string shaderPath = path + str;
		std::string newShaderPath = path + str.substr(0, str.find('.'));
		newShaderPath += str.substr(str.find('.')+1, 4);
		newShaderPath[newShaderPath.length() - 4] = newShaderPath[newShaderPath.length() - 4] + ('A' - 'a');
		newShaderPath += ".spv";
		outFile << compilerFile << " " << shaderPath << " -o " << newShaderPath << std::endl;
		newShaderPathVec.push_back(newShaderPath);
	}
	outFile << "pause";
	std::string batchFileRunCommand = std::string("cmd.exe /c \"") + batchFile + "\"";
	system(batchFileRunCommand.c_str());

	std::string assetImporterFile =std::string(buffer) + "\\.\\..\\AssetImporter\\Debug\\importer.exe";
	for (auto shaderStr : newShaderPathVec)
	{
		std::string importFilePath = shaderStr;
		importFilePath[importFilePath.length() - 4] = '.';
		importFilePath[importFilePath.length() - 3] = 'd';
		importFilePath[importFilePath.length() - 2] = 'a';
		importFilePath[importFilePath.length() - 1] = 't';
		importFilePath.push_back('a');
		std::string importShaderCommand = std::string("cmd.exe /c \"") + assetImporterFile + std::string(" ") + shaderStr + std::string(" ") + importFilePath;
		system(importShaderCommand.c_str());
	}

}

int main() 
{	
	compileShader();
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	IDevice* device = new VulkanDevice();
	IWindow* window = new VulkanWindow(true, static_cast<VulkanDevice*>(device)->GetInstance());
	if (static_cast<VulkanWindow*>(window)->InitPresentFamily(static_cast<VulkanDevice*>(device)->GetPhysicalDevice()))
	{
		ISwapChain* swapChain = new VulkanSwapChain
		(
			static_cast<VulkanDevice*>(device)->GetPhysicalDevice(),
			static_cast<VulkanWindow*>(window)->GetVulkanSurface(),
			static_cast<VulkanDevice*>(device)->GetDevice(),
			static_cast<VulkanDevice*>(device)->GetGraphicsFamily(),
			static_cast<VulkanWindow*>(window)->GetPresentFamily()
		);
		
		presentEngine = new VulkanPresentEngine(static_cast<VulkanSwapChain*>(swapChain)->GetSwapChainKHR(), static_cast<VulkanDevice*>(device)->GetGraphicsQueue());
		test(static_cast<VulkanDevice*>(device)->GetGraphicsQueue(), static_cast<VulkanWindow*>(window));
		static_cast<VulkanDevice*>(device)->FreeCommandPool();
		delete presentEngine;
		ResourceCreator::DestroyCachingResource();
		delete swapChain;
	}
	delete window;
	delete device;

	return 0;
}