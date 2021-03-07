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
#include "CustomNode.h"

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
	auto gaussianBlurMaterial = ResourceCreator::CreateMaterial("blur", MaterialMode::Normal, "D:\\OfflineRenderer\\Asset\\quadVert.data", "D:\\OfflineRenderer\\Asset\\gaussianBlurFrag.data", "");
	auto anistroVolumeMaterial = ResourceCreator::CreateMaterial("test5", "D:\\OfflineRenderer\\Asset\\anisoMipMapVolumeComp.data");
	auto anistroBaseMaterial = ResourceCreator::CreateMaterial("test6", "D:\\OfflineRenderer\\Asset\\anisoMipMapBaseComp.data");
	auto computeMaterial = ResourceCreator::CreateMaterial("test4", "D:\\OfflineRenderer\\Asset\\clearVoxelMapComp.data");
	auto injectRadianceMaterial = ResourceCreator::CreateMaterial("test7", "D:\\OfflineRenderer\\Asset\\injectRadianceComp.data");
	//auto ttmaterial = ResourceCreator::CreateMaterial("test3", "D:\\OfflineRenderer\\Asset\\voxelizationVert.data", "D:\\OfflineRenderer\\Asset\\voxelizationFrag.data", "D:\\OfflineRenderer\\Asset\\voxelizationGeom.data");
	auto tmaterial = ResourceCreator::CreateMaterial("test2", MaterialMode::Normal, "D:\\OfflineRenderer\\Asset\\renderVoxelVert.data", "D:\\OfflineRenderer\\Asset\\renderVoxelFrag.data", "D:\\OfflineRenderer\\Asset\\renderVoxelGeom.data");
	//auto material = ResourceCreator::CreateMaterial("test1", "D:\\OfflineRenderer\\Asset\\lightVert.data", "D:\\OfflineRenderer\\Asset\\lightFrag.data");

	VulkanSceneData * scene = new VulkanSceneData("D:\\crytek-sponza-noflag\\res\\sponza.data");

	ResourceCreator::CreateDepthStencilAttachment("DepthStencilAttachment", gScreenWidth, gScreenHeight);
	ResourceCreator::CreateColorAttachment("ShadowMap", 1024, 1024, TextureFormat::TF_R32G32B32A32SFloat);
	ResourceCreator::CreateColorAttachment("ShadowMapBlur", 1024, 1024, TextureFormat::TF_R32G32B32A32SFloat);
	ResourceCreator::CreateDepthStencilAttachment("ShadowDepthStencilAttachment", 1024, 1024);

	auto shadowAttachment = ResourceCreator::GetAttachment("ShadowMap");
	auto shadowBlurAttachment = ResourceCreator::GetAttachment("ShadowMapBlur");
	std::vector<IImage::ImagePtr> shadowMapImage = { shadowAttachment->GetImage(0),  shadowAttachment->GetImage(1), shadowAttachment->GetImage(2) };
	std::vector<IImage::ImagePtr> shadowMapBlurImage = { shadowBlurAttachment->GetImage(0), shadowBlurAttachment->GetImage(1), shadowBlurAttachment->GetImage(2) };
	VulkanRenderingCustomNode::CustomNodePtr blurVertNode = std::make_shared<BlurCustomNode>(gaussianBlurMaterial, scene->GetWorldData().get(), shadowMapImage, shadowMapBlurImage, Vec2(1.0f / 1024.0f, 0.0f));
	VulkanRenderingCustomNode::CustomNodePtr blurHoriNode = std::make_shared<BlurCustomNode>(gaussianBlurMaterial, scene->GetWorldData().get(), shadowMapBlurImage, shadowMapImage, Vec2(0.0f, 1.0f / 1024.0f));

	scene->AddUpdateMaterial("test2");
	scene->AddUpdateMaterial("test4");
	scene->AddUpdateMaterial("test5");
	scene->AddUpdateMaterial("test6");
	scene->AddUpdateMaterial("test7");
	scene->AddUpdateMaterial("blur");


	RenderingPipelineNodeDesc voxelizationPass = {};
	voxelizationPass.NodeName = "voxelization";
	voxelizationPass.BindPoint = PipelineBindPoint::BP_GRAPHICS;
	voxelizationPass.AttachToWindowNode = false;
	voxelizationPass.RenderingNodeDescVec = scene->ExportAllRenderingNodeByMaterial("D:\\OfflineRenderer\\Asset\\voxelizationVert.data", "D:\\OfflineRenderer\\Asset\\voxelizationFrag.data", "D:\\OfflineRenderer\\Asset\\voxelizationGeom.data", "Sponza", MaterialMode::NoAttachment);
	voxelizationPass.DependingNodeIndex.push_back(2);
	
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
	renderVoxelPass.DependingNodeIndex.push_back(6);
	

	ComputeNodeDesc cnd = {};
	cnd.Invocation = Vec3(256, 256, 256);
	cnd.MaterialAddr = (char*)(computeMaterial.get());
	cnd.World = scene->GetWorldData().get();
	RenderingPipelineNodeDesc clearVoxelPass = {};
	clearVoxelPass.NodeName = "clearVoxel";
	clearVoxelPass.BindPoint = PipelineBindPoint::BP_COMPUTE;
	clearVoxelPass.AttachToWindowNode = false;
	clearVoxelPass.ComputeNodeDescVec.push_back(cnd);
	clearVoxelPass.DependingNodeIndex.push_back(7);

	ComputeNodeDesc mipBaseCnd = {};
	mipBaseCnd.Invocation = Vec3(256, 256, 256);
	mipBaseCnd.MaterialAddr = (char*)(anistroBaseMaterial.get());
	mipBaseCnd.World = scene->GetWorldData().get();
	RenderingPipelineNodeDesc mipBasePass = {};
	mipBasePass.NodeName = "mipBase";
	mipBasePass.BindPoint = PipelineBindPoint::BP_COMPUTE;
	mipBasePass.AttachToWindowNode = false;
	mipBasePass.ComputeNodeDescVec.push_back(mipBaseCnd);
	mipBasePass.DependingNodeIndex.push_back(4);

	ComputeNodeDesc mipVolumeCnd = {};
	mipVolumeCnd.Invocation = Vec3(256, 256, 256);
	mipVolumeCnd.MaterialAddr = (char*)(anistroVolumeMaterial.get());
	mipVolumeCnd.World = scene->GetWorldData().get();
	mipVolumeCnd.ComputeNode = std::make_shared<VoxelMipMapCustomNode>(mipVolumeCnd);
	RenderingPipelineNodeDesc mipVolumePass = {};
	mipVolumePass.NodeName = "mipVolumn";
	mipVolumePass.BindPoint = PipelineBindPoint::BP_COMPUTE;
	mipVolumePass.AttachToWindowNode = false;
	mipVolumePass.ComputeNodeDescVec.push_back(mipVolumeCnd);
	mipVolumePass.DependingNodeIndex.push_back(5);

	ComputeNodeDesc injectRadianceCnd = {};
	injectRadianceCnd.Invocation = Vec3(256, 256, 256);
	injectRadianceCnd.MaterialAddr = (char*)(injectRadianceMaterial.get());
	injectRadianceCnd.World = scene->GetWorldData().get();
	RenderingPipelineNodeDesc injectRadiancePass = {};
	injectRadiancePass.NodeName = "injectRadiance";
	injectRadiancePass.BindPoint = PipelineBindPoint::BP_COMPUTE;
	injectRadiancePass.AttachToWindowNode = false;
	injectRadiancePass.ComputeNodeDescVec.push_back(injectRadianceCnd);
	injectRadiancePass.DependingNodeIndex.push_back(3);
	/*
	RenderingPipelineNodeDesc scenePass = {};
	scenePass.NodeName = "lightPass";
	scenePass.BindPoint = PipelineBindPoint::BP_GRAPHICS;
	scenePass.AttachToWindowNode = true;
	scenePass.FrameBufferDesc.Width = gScreenWidth;
	scenePass.FrameBufferDesc.Height = gScreenHeight;
	scenePass.FrameBufferDesc.AttachmentName.push_back("SwapChainImage");
	scenePass.FrameBufferDesc.AttachmentName.push_back("DepthStencilAttachment");
	AttachmentDesc attachDesc;
	attachDesc.Usage = TextureUsageBits::TU_COLOR_ATTACHMENT;
	attachDesc.Format = TextureFormat::TF_B8G8R8A8SRGB;
	attachDesc.LoadOp = AttachmentOperator::AO_CLEAR;
	attachDesc.StoreOp = AttachmentOperator::AO_STORE;
	scenePass.FrameBufferLayoutDesc.AttachmentDesc.push_back(attachDesc);
	scenePass.RenderingNodeDescVec = scene->ExportAllRenderingNodeByMaterial("D:\\OfflineRenderer\\Asset\\lightVert.data", "D:\\OfflineRenderer\\Asset\\lightFrag.data", "", "SponzaSceneLight", MaterialMode::Normal);
	attachDesc.Usage = TextureUsageBits::TU_DEPTH_STENCIL;
	attachDesc.Format = TextureFormat::TF_D24US8;
	scenePass.FrameBufferLayoutDesc.AttachmentDesc.push_back(attachDesc);
	scenePass.DependingNodeIndex.push_back(6);
	*/
	RenderingPipelineNodeDesc shadowPass = {};
	shadowPass.NodeName = "shadowPass";
	shadowPass.BindPoint = PipelineBindPoint::BP_GRAPHICS;
	shadowPass.AttachToWindowNode = false;
	shadowPass.FrameBufferDesc.Width = 1024;
	shadowPass.FrameBufferDesc.Height = 1024;
	shadowPass.FrameBufferDesc.AttachmentName.push_back("ShadowMap");
	shadowPass.FrameBufferDesc.AttachmentName.push_back("ShadowDepthStencilAttachment");
	AttachmentDesc shadowAttachDesc;
	shadowAttachDesc.Usage = TextureUsageBits::TU_COLOR_ATTACHMENT;
	shadowAttachDesc.Format = TextureFormat::TF_R32G32B32A32SFloat;
	shadowAttachDesc.LoadOp = AttachmentOperator::AO_CLEAR;
	shadowAttachDesc.StoreOp = AttachmentOperator::AO_STORE;
	shadowPass.FrameBufferLayoutDesc.AttachmentDesc.push_back(shadowAttachDesc);
	shadowPass.RenderingNodeDescVec = scene->ExportAllRenderingNodeByMaterial("D:\\OfflineRenderer\\Asset\\evsmDepthVert.data", "D:\\OfflineRenderer\\Asset\\evsmDepthFrag.data", "", "SponzaSceneDepth", MaterialMode::Normal);	
	shadowAttachDesc.Usage = TextureUsageBits::TU_DEPTH_STENCIL;
	shadowAttachDesc.Format = TextureFormat::TF_D24US8;
	shadowPass.FrameBufferLayoutDesc.AttachmentDesc.push_back(shadowAttachDesc);

	RenderingPipelineNodeDesc blurPass = {};
	blurPass.NodeName = "blurPass";
	blurPass.BindPoint = PipelineBindPoint::BP_GRAPHICS;
	blurPass.AttachToWindowNode = false;
	blurPass.FrameBufferDesc.Width = 1024;
	blurPass.FrameBufferDesc.Height = 1024;
	blurPass.FrameBufferDesc.AttachmentName.push_back("ShadowMapBlur");
	AttachmentDesc blurAttachDesc;
	blurAttachDesc.Usage = TextureUsageBits::TU_COLOR_ATTACHMENT;
	blurAttachDesc.Format = TextureFormat::TF_R32G32B32A32SFloat;
	blurAttachDesc.LoadOp = AttachmentOperator::AO_CLEAR;
	blurAttachDesc.StoreOp = AttachmentOperator::AO_STORE;
	blurPass.FrameBufferLayoutDesc.AttachmentDesc.push_back(blurAttachDesc);
	RenderingNodeDesc blurNode = {};
	blurNode.EmptyVertexCount = 0;
	blurNode.MaterialAddr = (char*)(gaussianBlurMaterial.get());
	blurNode.World = scene->GetWorldData().get();
	blurNode.CustomNode = blurVertNode;
	blurPass.RenderingNodeDescVec.push_back(blurNode);
	blurPass.DependingNodeIndex.push_back(0);

	RenderingPipelineNodeDesc blur2Pass = {};
	blur2Pass.NodeName = "blur2Pass";
	blur2Pass.BindPoint = PipelineBindPoint::BP_GRAPHICS;
	blur2Pass.AttachToWindowNode = false;
	blur2Pass.FrameBufferDesc.Width = 1024;
	blur2Pass.FrameBufferDesc.Height = 1024;
	blur2Pass.FrameBufferDesc.AttachmentName.push_back("ShadowMap");
	AttachmentDesc blur2AttachDesc;
	blur2AttachDesc.Usage = TextureUsageBits::TU_COLOR_ATTACHMENT;
	blur2AttachDesc.Format = TextureFormat::TF_R32G32B32A32SFloat;
	blur2AttachDesc.LoadOp = AttachmentOperator::AO_CLEAR;
	blur2AttachDesc.StoreOp = AttachmentOperator::AO_STORE;
	blur2Pass.FrameBufferLayoutDesc.AttachmentDesc.push_back(blur2AttachDesc);
	RenderingNodeDesc blur2Node = {};
	blur2Node.EmptyVertexCount = 0;
	blur2Node.MaterialAddr = (char*)(gaussianBlurMaterial.get());
	blur2Node.World = scene->GetWorldData().get();
	blur2Node.CustomNode = blurHoriNode;
	blur2Pass.RenderingNodeDescVec.push_back(blur2Node);
	blur2Pass.DependingNodeIndex.push_back(1);

	std::vector<RenderingPipelineNodeDesc> scenePipelineNodesVec;
	scenePipelineNodesVec.push_back(shadowPass);
	scenePipelineNodesVec.push_back(blurPass);
	scenePipelineNodesVec.push_back(blur2Pass);
	scenePipelineNodesVec.push_back(voxelizationPass);
	scenePipelineNodesVec.push_back(injectRadiancePass);
	scenePipelineNodesVec.push_back(mipBasePass);
	scenePipelineNodesVec.push_back(mipVolumePass);
	//scenePipelineNodesVec.push_back(scenePass);
	scenePipelineNodesVec.push_back(renderVoxelPass);
	scenePipelineNodesVec.push_back(clearVoxelPass);


	auto scenePipeline = new VulkanRenderingPipeline();
	scenePipeline->GenerateRenderingGraph(scenePipelineNodesVec);

	while (true)
	{
		int imageIndex = presentEngine->AcquireImage();
		scene->UpdateSceneData(imageIndex);
		auto semaphore = scenePipeline->SubmitRenderingCommands(imageIndex, graphicsQueue, presentEngine->GetCurrentFrameRenderFinishFence());
		presentEngine->PresentFrame(imageIndex, semaphore);
		if (window->MainLoop())
		{
			vkQueueWaitIdle(graphicsQueue);
			delete scenePipeline;
			break;
		}
	}

}

void compileShader()
{
	
	std::vector<std::string> shaderFiles = 
	{
		"light.vert",
		"light.frag",
		"voxelization.vert",
		"voxelization.frag",
		"voxelization.geom",
		"renderVoxel.vert",
		"renderVoxel.frag",
		"renderVoxel.geom",
		"clearVoxelMap.comp",
		"averageVoxelMap.comp",
		"anisoMipMapBase.comp",
		"anisoMipMapVolume.comp",
		"injectRadiance.comp"
	};
	
	/*
	std::vector<std::string> shaderFiles =
	{
		"light.vert",
		"light.frag",
		"evsmDepth.vert",
		"evsmDepth.frag",
		"quad.vert",
		"gaussianBlur.frag"
	};
	*/

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