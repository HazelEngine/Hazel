#pragma once

#include "Hazel/Core.h"
#include "Hazel/MathDef.h"

#include "Hazel/Rendering/Color.h"
#include "Hazel/Rendering/Format.h"
#include "Hazel/Rendering/Buffer.h"

namespace Hazel {

	// Forward declarations:
	class Renderer;
	class Command;

	class Mesh
	{
	public:
		//! TYPEDEF/ENUMS:
		enum Topology
		{
			ePointList,
			eLineList,
			eLineStrip,
			eTriangleList,
			eTriangleStrip,
			eTriangleFan,
			eLineListWithAdjacency,
			eLineStripWithAdjacency,
			eTriangleListWithAdjacency,
			eTriangleStripWithAdjacency,
			ePatchList
		};

		struct Vertex2D
		{
			float X, Y;
			float S, T;
			unsigned char R, G, B, A;
		};

		struct Vertex3D
		{
			float X, Y, Z;
			float S, T;
		};

		struct VertexAttribute
		{
			std::string Name;
			uint32_t Offset;
			Format Format_;
		};

		typedef std::map<uint32_t, VertexAttribute> VertexInput;

		//! CTOR/DTOR:
		Mesh(Renderer* renderer);
		virtual ~Mesh();

		//! VIRTUALS:
		virtual void Build(
			uint32_t vertexSize = 0,
			const void* vertexData = nullptr,
			uint32_t indexSize = 0,
			const void* indexData = nullptr,
			const Buffer::CreateInfo& ci = Buffer::CreateInfo::GetDefault()
		);

		virtual void Draw(
			Command* cmd,
			uint32_t instanceCount = 1,
			uint32_t firstIndex = 0,
			int32_t vertexOffset = 0,
			uint32_t firstInstance = 0
		);

		virtual void Cleanup();

		//! SERVICES:
		static VertexInput GetVertexInput2D();
		static VertexInput GetVertexInput3D();

		static std::shared_ptr<Mesh> BuildCube(Renderer* renderer, const vec3& min, const vec3& max);
		static std::shared_ptr<Mesh> BuildCube(Renderer* renderer, float size);
		static std::shared_ptr<Mesh> BuildGrid(Renderer* renderer, unsigned int width, unsigned int height);
		static std::shared_ptr<Mesh> BuildPatch(Renderer* renderer, unsigned int width, unsigned int height);
		static std::shared_ptr<Mesh> BuildQuad(Renderer* renderer, float width, float height, const Color& color);
		static std::shared_ptr<Mesh> BuildQuad(Renderer* renderer);

		//! ACCESSORS:
		Buffer* GetVBO() const;
		Buffer* GetIBO() const;
		size_t GetIndexCount() const;
		void SetIndexCount(size_t count);

	protected:
		//! MEMBERS:
		Renderer* m_Renderer;
		std::shared_ptr<Buffer> m_VBO;
		std::shared_ptr<Buffer> m_IBO;
		size_t m_IndexCount;
	};

	////////////////////////////////////////////////////////////////////////////////
	// Mesh inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline Mesh::Mesh(Renderer* renderer) :
		m_Renderer(renderer),
		m_IndexCount(0)
	{
	}

	inline Mesh::~Mesh()
	{
	}

	inline Mesh::VertexInput Mesh::GetVertexInput2D()
	{
		return
		{
			{ 0, { "POSITION", offsetof(Mesh::Vertex2D, X), Format::eRG32F } },
			{ 1, { "TEXCOORD", offsetof(Mesh::Vertex2D, S), Format::eRG32F } },
			{ 2, { "COLOR"   , offsetof(Mesh::Vertex2D, R), Format::eRGBA8 } }
		};
	}

	inline Mesh::VertexInput Mesh::GetVertexInput3D()
	{
		return
		{
			{ 0, { "POSITION", offsetof(Mesh::Vertex3D, X), Format::eRGB32F } },
			{ 1, { "TEXCOORD", offsetof(Mesh::Vertex3D, S), Format::eRG32F  } }
		};
	}

	inline std::shared_ptr<Mesh> Mesh::BuildCube(Renderer* renderer, float size)
	{
		return Mesh::BuildCube(renderer, vec3(size * 0.5f), vec3(-size * 0.5f));
	}

	inline std::shared_ptr<Mesh> Mesh::BuildQuad(Renderer* renderer)
	{
		return Mesh::BuildQuad(renderer, 1.0f, 1.0f, Color::White);
	}

	inline Buffer* Mesh::GetVBO() const
	{
		return m_VBO.get();
	}

	inline Buffer* Mesh::GetIBO() const
	{
		return m_IBO.get();
	}

	inline size_t Mesh::GetIndexCount() const
	{
		return m_IndexCount;
	}

	inline void Mesh::SetIndexCount(size_t count)
	{
		m_IndexCount = count;
	}

}