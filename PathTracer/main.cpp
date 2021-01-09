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
	auto material = ResourceCreator::CreateMaterial("test1", "D:\\OfflineRenderer\\Asset\\lightVert.data", "D:\\OfflineRenderer\\Asset\\lightFrag.data");
	ResourceCreator::CreateImageFromFile("D:\\OfflineRenderer\\Asset\\Dst\\red.data");
	VulkanSceneData * scene = new VulkanSceneData("C:\\Users\\syddfyuan\\Downloads\\VCTRenderer-master\\VCTRenderer-master\\engine\\assets\\models\\crytek-sponza\\res\\sponza.data");
	RenderingPipelineNodeDesc nodeDesc = {};
	nodeDesc.NodeName = "testNode";
	nodeDesc.BindPoint = PipelineBindPoint::BP_GRAPHICS;
	nodeDesc.AttachToWindowNode = true;
	nodeDesc.FrameBufferDesc.Width = gScreenWidth;
	nodeDesc.FrameBufferDesc.Height = gScreenHeight;
	nodeDesc.FrameBufferDesc.AttachmentName.push_back("SwapChainImage");

	AttachmentDesc attachDesc;
	attachDesc.Usage = TextureUsageBits::TU_COLOR_ATTACHMENT;
	attachDesc.LoadOp = AttachmentOperator::AO_CLEAR;
	attachDesc.StoreOp = AttachmentOperator::AO_STORE;
	attachDesc.Format = TextureFormat::TF_B8G8R8A8SRGB;
	nodeDesc.FrameBufferLayoutDesc.AttachmentDesc.push_back(attachDesc);

	nodeDesc.RenderingNodeDescVec = scene->ExportAllRenderingNodeByMaterial("D:\\OfflineRenderer\\Asset\\lightVert.data", "D:\\OfflineRenderer\\Asset\\lightFrag.data", "Sponza");

	std::vector<RenderingPipelineNodeDesc> pipelineNodesVec;
	pipelineNodesVec.push_back(nodeDesc);

	auto vulkanPipeline = new VulkanRenderingPipeline();
	vulkanPipeline->GenerateRenderingGraph(pipelineNodesVec);
	float theta = 0.0f;
	while (true)
	{
		theta += 0.01f;
		glm::vec3 position = glm::vec3(-500 * sinf(theta), 0, -500 * cosf(theta));
		glm::vec3 viewDir = glm::normalize(glm::vec3(0, 0, 0) - position);
		glm::mat4x4 view = glm::lookAtLH(position, position + viewDir, glm::vec3(0.0f, 1.0f, 0.0f));
		material->SetMatrix("view", view);
		glm::mat4x4 world = glm::identity<glm::mat4x4>();
		glm::mat4x4 proj = glm::perspectiveFovLH(90.0f, (float)gScreenWidth, (float)gScreenHeight, 0.1f, 1000.0f);
		material->SetMatrix("model", world);
		material->SetMatrix("proj", proj);
		int imageIndex = presentEngine->AcquireImage();
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
		"light.vert",
		"light.frag"
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
		ResourceCreator::CreateImageFromFile("./../../Asset/Dst/red.data");
		
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