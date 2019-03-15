#pragma once

#include "Hazel/Core.h"

#include "Hazel/Rendering/Command.h"
#include "Hazel/Rendering/Texture.h"
#include "Hazel/Rendering/Mesh.h"

namespace Hazel {

	// Forward declarations:
	class Renderer;

	class HAZEL_API RenderPass
	{
	public:
		//! TYPEDEF/ENUMS:
		enum Type
		{
			eDrawOnce,
			eDrawOnceBackground,
			eDrawSync,
			eDrawSyncBackground
		};

		enum BlendFactor
		{
			eZero,
			eOne,
			eSrcColor,
			eOneMinusSrcColor,
			eDstColor,
			eOneMinusDstColor,
			eSrcAlpha,
			eOneMinusSrcAlpha,
			eDstAlpha,
			eOneMinusDstAlpha,
			eConstantColor,
			eOneMinusConstantColor,
			eConstantAlpha,
			eOneMinusConstantAlpha,
			eSrcAlphaSaturate,
			eSrc1Color,
			eOneMinusSrc1Color,
			eSrc1Alpha,
			eOneMinusSrc1Alpha
		};

		enum BlendOp
		{
			eAdd,
			eSubtract,
			eReverseSubtract,
			eMin,
			eMax
		};

		struct Blending
		{
			//! SERVICES:
			static Blending GetDefault();
			static Blending GetAdditive();
			static Blending GetAdditiveAlpha();

			//! MEMBERS:
			bool BlendEnable;
			BlendFactor SrcColorBlendFactor;
			BlendFactor DstColorBlendFactor;
			BlendOp ColorBlendOp;
			BlendFactor SrcAlphaBlendFactor;
			BlendFactor DstAlphaBlendFactor;
			BlendOp AlphaBlendOp;
		};

		struct Attachment
		{
			Texture* Input;
			Blending Blending_;
		};

		struct CreateInfo
		{
			//! SERVICES:
			static CreateInfo GetDefault();

			//! MEMBERS:
			Type Type_;
			// Camera
			// Shader
			Mesh::Topology Topology_;
			Mesh::VertexInput VertexInput_;
			uint32_t VertexStride;
			ivec2 Resolution;
			Color ClearColor;
			bool EnableDepth;
			uint32_t NumThreads;
			std::vector<Attachment> Attachments;
			std::vector<Texture*> Samplers;
			std::vector<Buffer*> Ubos;
		};

		//! CTOR/DTOR:
		RenderPass(Renderer* renderer, RenderPass* parent);
		virtual ~RenderPass();

		//! VIRTUALS:
		virtual void Build(const CreateInfo& ci) = 0;
		virtual void Resize(int width, int height) = 0;
		virtual void Record() = 0;
		virtual void Cleanup() = 0;

		//! ACCESSORS:
		Renderer* GetRenderer() const;
		Type GetType() const;

		// TODO: Remove Callback!
		typedef std::function<void(Command* cmd)> CallbackFn;
		CallbackFn OnRecord;

	protected:
		struct CameraData
		{
			mat4 Proj;
			mat4 View;
			mat4 ViewProj;
			mat4 InvProj;
			mat4 InvView;
			vec2 Viewport;
		};

		//! MEMBERS:
		Renderer* m_Renderer;
		RenderPass* m_Parent;
		CreateInfo m_CreateInfo;
		std::shared_ptr<Buffer> m_CameraUBO;
		CameraData m_CameraData;
	};

	////////////////////////////////////////////////////////////////////////////////
	// RenderPass::Blending inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline RenderPass::Blending RenderPass::Blending::GetDefault()
	{
		return Blending() =
		{
			false,
			BlendFactor::eZero,
			BlendFactor::eOne,
			BlendOp::eAdd,
			BlendFactor::eZero,
			BlendFactor::eZero,
			BlendOp::eAdd
		};
	}

	inline RenderPass::Blending RenderPass::Blending::GetAdditive()
	{
		return Blending() =
		{
			false,
			BlendFactor::eSrcAlpha,
			BlendFactor::eOneMinusSrcAlpha,
			BlendOp::eAdd,
			BlendFactor::eOne,
			BlendFactor::eOne,
			BlendOp::eAdd
		};
	}

	inline RenderPass::Blending RenderPass::Blending::GetAdditiveAlpha()
	{
		return Blending() =
		{
			false,
			BlendFactor::eSrcAlpha,
			BlendFactor::eOneMinusSrcAlpha,
			BlendOp::eAdd,
			BlendFactor::eOneMinusSrcAlpha,
			BlendFactor::eZero,
			BlendOp::eAdd
		};
	}

	////////////////////////////////////////////////////////////////////////////////
	// RenderPass::CreateInfo inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline RenderPass::CreateInfo RenderPass::CreateInfo::GetDefault()
	{
		return CreateInfo() =
		{
			eDrawSync,
			// Camera
			// Shader = nullptr
			Mesh::eTriangleList,
			Mesh::GetVertexInput2D(),
			sizeof(Mesh::Vertex2D),
			ivec2(1, 1),
			Color::Black,
			false,
			1
		};
	}

	////////////////////////////////////////////////////////////////////////////////
	// RenderPass inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline RenderPass::RenderPass(Renderer* renderer, RenderPass* parent) :
		m_Renderer(renderer),
		m_Parent(parent)
	{
	}

	inline RenderPass::~RenderPass()
	{
	}

	inline Renderer* RenderPass::GetRenderer() const
	{
		return m_Renderer;
	}

	inline RenderPass::Type RenderPass::GetType() const
	{
		return m_CreateInfo.Type_;
	}

}