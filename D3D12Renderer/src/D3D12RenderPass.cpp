#include "hzpch.h"
#include "D3D12RenderPass.h"

#include "D3D12Buffer.h"
#include "D3D12Texture.h"
#include "D3D12Command.h"
#include "D3D12Renderer.h"

namespace Hazel {

	void D3D12RenderPass::Build(const CreateInfo& ci)
	{
		m_CreateInfo = ci;

		D3D12RenderPass::CreateCommandList();
		D3D12RenderPass::CreateRootSignature();
		D3D12RenderPass::CreatePipelineStateObject();

		// TODO: Modify the camera UBO (CBV)
		m_CameraUBO = m_Renderer->CreateBuffer(
			sizeof(CameraData),
			&m_CameraData,
			Buffer::CreateInfo::GetStaging()
		);

		D3D12RenderPass::Resize(
			static_cast<int>(ci.Resolution.x),
			static_cast<int>(ci.Resolution.y)
		);
	}

	void D3D12RenderPass::Resize(int width, int height)
	{
		m_Resolution = ivec2(width, height);
	}

	void D3D12RenderPass::Record()
	{
		auto device = static_cast<D3D12Renderer*>(m_Renderer)->m_Device;
		auto bufferIndex = m_Renderer->GetBufferIndex();
		auto dxSwapchainImageView = static_cast<D3D12Renderer*>(m_Renderer)->m_SwapchainImages[bufferIndex].View;
		auto dxSwapchainHeap = static_cast<D3D12Renderer*>(m_Renderer)->m_SwapchainHeap;
		auto dxCmdQueue = static_cast<D3D12Renderer*>(m_Renderer)->m_CommandQueue;
		auto dxCmdAlloc = static_cast<D3D12Command*>(m_Commands[bufferIndex].get())->m_Allocator;

		auto rtvStride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
			dxSwapchainHeap->GetCPUDescriptorHandleForHeapStart(),
			bufferIndex,
			rtvStride
		);

		float clearColor[4] =
		{
			m_CreateInfo.ClearColor.R * (1.0f / 255.0f),
			m_CreateInfo.ClearColor.G * (1.0f / 255.0f),
			m_CreateInfo.ClearColor.B * (1.0f / 255.0f),
			m_CreateInfo.ClearColor.A * (1.0f / 255.0f)
		};

		dxCmdAlloc->Reset();
		m_CommandList->Reset(dxCmdAlloc, m_PipelineStateObject);
		m_CommandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				dxSwapchainImageView,
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET
			)
		);

		m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		m_CommandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
		m_CommandList->SetGraphicsRootSignature(m_RootSignature);

		m_Commands[bufferIndex]->Viewport(0.0f, 0.0f, (float)m_Resolution.x, (float)m_Resolution.y);
		m_Commands[bufferIndex]->Scissor(0, 0, m_Resolution.x, m_Resolution.y);

		m_CommandList->IASetPrimitiveTopology(
			D3D12RenderPass::ConvertToDxTopology(m_CreateInfo.Topology_)
		);
		// TODO: Remove this!
		OnRecord(m_Commands[bufferIndex].get());

		m_CommandList->ResourceBarrier(
			1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				dxSwapchainImageView,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT
			)
		);

		m_CommandList->Close();
		ID3D12CommandList* ppCommandLists[] = { m_CommandList };
		dxCmdQueue->ExecuteCommandLists(
			_countof(ppCommandLists), ppCommandLists
		);
	}

	void D3D12RenderPass::Cleanup()
	{
	}

	void D3D12RenderPass::CreateCommandList()
	{
		auto device = static_cast<D3D12Renderer*>(m_Renderer)->m_Device;

		for (uint32_t i = 0; i < m_Renderer->GetBufferCount(); ++i)
		{
			auto cmd = std::shared_ptr<Command>(new D3D12Command(this));
			cmd->Build();

			m_Commands.push_back(cmd);
		}

		if (FAILED(device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			static_cast<D3D12Command*>(m_Commands[0].get())->m_Allocator,
			nullptr,
			__uuidof(ID3D12GraphicsCommandList),
			(void**)&m_CommandList
		)))
		{
			//HZ_CORE_ERROR("Failed to create command list!")
			return;
		}

		if (FAILED(m_CommandList->Close()))
		{
			//HZ_CORE_ERROR("Failed to close command list!")
			return;
		}

		for (auto& cmd : m_Commands)
		{
			auto dxCmd = static_cast<D3D12Command*>(cmd.get());
			dxCmd->SetCommandListFromParent();
		}
	}

	void D3D12RenderPass::CreateRootSignature()
	{
		auto device = static_cast<D3D12Renderer*>(m_Renderer)->m_Device;
		std::vector<CD3DX12_DESCRIPTOR_RANGE> descRanges;
		std::vector<CD3DX12_ROOT_PARAMETER> rootParams;
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		ID3DBlob* rootSignatureBlob;

		CD3DX12_DESCRIPTOR_RANGE cameraCbv = {};
		cameraCbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		descRanges.push_back(cameraCbv);

		CD3DX12_ROOT_PARAMETER cameraRootParam = {};
		cameraRootParam.InitAsDescriptorTable(1, &descRanges[0], D3D12_SHADER_VISIBILITY_ALL);
		rootParams.push_back(cameraRootParam);

		CD3DX12_STATIC_SAMPLER_DESC staticSamplers[1];
		staticSamplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

		rootSignatureDesc.Init(
			static_cast<UINT>(rootParams.size()),
			rootParams.data(),
			1,
			staticSamplers,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS     |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS   |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		);

		if (FAILED(D3D12SerializeRootSignature(
			&rootSignatureDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&rootSignatureBlob,
			nullptr
		)))
		{
			//HZ_CORE_ERROR("Failed to serialize root signature!")
			return;
		}

		if (FAILED(device->CreateRootSignature(
			0,
			rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&m_RootSignature)
		)))
		{
			//HZ_CORE_ERROR("Failed to create root signature!")
			return;
		}
	}

	void D3D12RenderPass::CreatePipelineStateObject()
	{
		auto ConvertToDxBlending = [](const Blending& input) -> D3D12_RENDER_TARGET_BLEND_DESC
		{
			auto BlendFactorToD3D12 = std::map<BlendFactor, D3D12_BLEND>() =
			{
				{ eZero                  , D3D12_BLEND_ZERO             },
				{ eOne                   , D3D12_BLEND_ONE              },
				{ eSrcColor              , D3D12_BLEND_SRC_COLOR        },
				{ eOneMinusSrcColor      , D3D12_BLEND_INV_SRC_COLOR    },
				{ eDstColor              , D3D12_BLEND_DEST_COLOR       },
				{ eOneMinusDstColor      , D3D12_BLEND_INV_DEST_COLOR   },
				{ eSrcAlpha              , D3D12_BLEND_SRC_ALPHA        },
				{ eOneMinusSrcAlpha      , D3D12_BLEND_INV_SRC_ALPHA    },
				{ eDstAlpha              , D3D12_BLEND_DEST_ALPHA       },
				{ eOneMinusDstAlpha      , D3D12_BLEND_INV_DEST_ALPHA   },
				{ eConstantColor         , D3D12_BLEND_BLEND_FACTOR     },
				{ eOneMinusConstantColor , D3D12_BLEND_INV_BLEND_FACTOR },
				{ eConstantAlpha         , D3D12_BLEND_BLEND_FACTOR     },
				{ eOneMinusConstantAlpha , D3D12_BLEND_INV_BLEND_FACTOR },
				{ eSrcAlphaSaturate      , D3D12_BLEND_SRC_ALPHA_SAT    },
				{ eSrc1Color             , D3D12_BLEND_SRC1_COLOR       },
				{ eOneMinusSrc1Color     , D3D12_BLEND_INV_SRC1_COLOR   },
				{ eSrc1Alpha             , D3D12_BLEND_SRC1_ALPHA       },
				{ eOneMinusSrc1Alpha     , D3D12_BLEND_INV_SRC1_ALPHA   }
			};

			auto BlendOpToD3D12 = std::map<BlendOp, D3D12_BLEND_OP>() =
			{
				{ eAdd                   , D3D12_BLEND_OP_ADD           },
				{ eSubtract              , D3D12_BLEND_OP_SUBTRACT      },
				{ eReverseSubtract       , D3D12_BLEND_OP_REV_SUBTRACT  },
				{ eMin                   , D3D12_BLEND_OP_MIN           },
				{ eMax                   , D3D12_BLEND_OP_MAX           }
			};

			D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
			rtBlendDesc.BlendEnable = input.BlendEnable;
			rtBlendDesc.SrcBlend = BlendFactorToD3D12[input.SrcColorBlendFactor];
			rtBlendDesc.DestBlend = BlendFactorToD3D12[input.DstColorBlendFactor];
			rtBlendDesc.BlendOp = BlendOpToD3D12[input.ColorBlendOp];
			rtBlendDesc.SrcBlendAlpha = BlendFactorToD3D12[input.SrcAlphaBlendFactor];
			rtBlendDesc.DestBlendAlpha = BlendFactorToD3D12[input.DstAlphaBlendFactor];
			rtBlendDesc.BlendOpAlpha = BlendOpToD3D12[input.AlphaBlendOp];
			rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			return rtBlendDesc;
		};

		auto TopologyToDx = std::map<Mesh::Topology, D3D12_PRIMITIVE_TOPOLOGY_TYPE>() =
		{
			{ Mesh::Topology::ePointList                  , D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT    },
			{ Mesh::Topology::eLineList                   , D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE     },
			{ Mesh::Topology::eLineStrip                  , D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE     },
			{ Mesh::Topology::eTriangleList               , D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
			{ Mesh::Topology::eTriangleStrip              , D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
			{ Mesh::Topology::eTriangleFan                , D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
			{ Mesh::Topology::eLineListWithAdjacency      , D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE     },
			{ Mesh::Topology::eLineStripWithAdjacency     , D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE     },
			{ Mesh::Topology::eTriangleListWithAdjacency  , D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
			{ Mesh::Topology::eTriangleStripWithAdjacency , D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
			{ Mesh::Topology::ePatchList                  , D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH    }
		};

		/*
		auto StageToDx = std::map<Shader::Stage, LPCSTR>() =
		{
			{ Shader::eVertex        , "vs_5_0" },
			{ Shader::eTessControl   , "hs_5_0" },
			{ Shader::eTessEvaluation, "ds_5_0" },
			{ Shader::eGeometry      , "gs_5_0" },
			{ Shader::eFragment      , "ps_5_0" },
			{ Shader::eCompute       , "cs_5_0" }
		}
		*/

		auto device = static_cast<D3D12Renderer*>(m_Renderer)->m_Device;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		ID3DBlob* shaderBlob;
		ID3DBlob* shaderBlobError;

		auto inputElementDescs = std::vector<D3D12_INPUT_ELEMENT_DESC>();
		for (auto& vi : m_CreateInfo.VertexInput_)
		{
			inputElementDescs.push_back(
				D3D12_INPUT_ELEMENT_DESC() =
				{
					vi.second.Name.c_str(),
					0,
					D3D12Renderer::ConvertToDxFormat(vi.second.Format_),
					0,
					vi.second.Offset,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					0
				}
			);
		}

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
		inputLayoutDesc.NumElements = static_cast<UINT>(inputElementDescs.size());
		inputLayoutDesc.pInputElementDescs = inputElementDescs.data();
		psoDesc.InputLayout = inputLayoutDesc;

		//
		// SHADERS
		//

		std::string vertexData =
			"struct VSInput"
			"{"
			"	float3 Position : POSITION0;"
			"	float2 TexCoord : TEXCOORD0;"
			"};"

			"struct VSOutput"
			"{"
			"	float4 Position : SV_Position;"
			"	float2 TexCoord : TEXCOORD0;"
			"};"

			"VSOutput main(VSInput input)"
			"{"
			"	VSOutput output;"

			"	output.Position = float4(input.Position, 1.0f);"
			"	output.TexCoord = input.TexCoord;"

			"	return output;"
			"}";

		std::string pixelData =
			"struct PSInput"
			"{"
			"	float4 Position : SV_Position;"
			"	float2 TexCoord : TEXCOORD0;"
			"};"

			"float4 main(PSInput input) : SV_Target"
			"{"
			"	return float4(input.TexCoord.x, input.TexCoord.y, 1.0f, 1.0f);"
			"}";

		if (FAILED(D3DCompile(
			static_cast<LPCVOID>(vertexData.data()),
			vertexData.size(),
			nullptr,
			nullptr,
			nullptr,
			"main",
			"vs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&shaderBlob,
			&shaderBlobError
		)))
		{
			auto e = (char*)shaderBlobError->GetBufferPointer();
			//HZ_CORE_ERROR("{0}", e)
			std::cout << e << std::endl;
		}

		const auto vs = D3D12_SHADER_BYTECODE () =
		{
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize()
		};

		psoDesc.VS = vs;

		if (FAILED(D3DCompile(
			static_cast<LPCVOID>(pixelData.data()),
			pixelData.size(),
			nullptr,
			nullptr,
			nullptr,
			"main",
			"ps_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&shaderBlob,
			&shaderBlobError
		)))
		{
			auto e = (char*)shaderBlobError->GetBufferPointer();
			//HZ_CORE_ERROR("{0}", e)
			std::cout << e << std::endl;
		}

		const auto ps = D3D12_SHADER_BYTECODE() =
		{
			shaderBlob->GetBufferPointer(),
			shaderBlob->GetBufferSize()
		};

		psoDesc.PS = ps;

		////////////////////////////////////////////////////////

		DXGI_SAMPLE_DESC sampleDesc = {};
		sampleDesc.Count = 1;

		D3D12_RASTERIZER_DESC& rastDesc = psoDesc.RasterizerState;
		rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rastDesc.CullMode = D3D12_CULL_MODE_NONE;
		rastDesc.FrontCounterClockwise = FALSE;
		rastDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rastDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rastDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rastDesc.DepthClipEnable = TRUE;
		rastDesc.MultisampleEnable = FALSE;
		rastDesc.AntialiasedLineEnable = FALSE;
		rastDesc.ForcedSampleCount = 0;
		rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		psoDesc.pRootSignature = m_RootSignature;
		psoDesc.PrimitiveTopologyType = TopologyToDx[m_CreateInfo.Topology_];
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.SampleDesc = sampleDesc;

		psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
		psoDesc.BlendState.RenderTarget[0] = ConvertToDxBlending(Blending::GetDefault());
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.NumRenderTargets = 1;

		if (FAILED(device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(&m_PipelineStateObject)
		)))
		{
			//HZ_CORE_ERROR("Failed to create PSO!")
			return;
		}
	}

	D3D_PRIMITIVE_TOPOLOGY D3D12RenderPass::ConvertToDxTopology(Mesh::Topology topology)
	{
		auto TopologyToDx = std::map<Mesh::Topology, D3D_PRIMITIVE_TOPOLOGY>() =
		{
			{ Mesh::Topology::ePointList                  , D3D_PRIMITIVE_TOPOLOGY_POINTLIST                 },
			{ Mesh::Topology::eLineList                   , D3D_PRIMITIVE_TOPOLOGY_LINELIST                  },
			{ Mesh::Topology::eLineStrip                  , D3D_PRIMITIVE_TOPOLOGY_LINESTRIP                 },
			{ Mesh::Topology::eTriangleList               , D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST              },
			{ Mesh::Topology::eTriangleStrip              , D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP             },
			{ Mesh::Topology::eTriangleFan                , D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST              },
			{ Mesh::Topology::eLineListWithAdjacency      , D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ              },
			{ Mesh::Topology::eLineStripWithAdjacency     , D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ             },
			{ Mesh::Topology::eTriangleListWithAdjacency  , D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ          },
			{ Mesh::Topology::eTriangleStripWithAdjacency , D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ         },
			{ Mesh::Topology::ePatchList                  , D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST }
		};

		return TopologyToDx[topology];
	}

}