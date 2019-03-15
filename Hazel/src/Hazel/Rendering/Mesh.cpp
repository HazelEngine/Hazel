#include "hzpch.h"
#include "Mesh.h"

#include "Renderer.h"

namespace Hazel {

	void Mesh::Build(
		uint32_t vertexSize,
		const void* vertexData,
		uint32_t indexSize,
		const void* indexData,
		const Buffer::CreateInfo& ci)
	{
		if (vertexSize > 0)
		{
			auto info = Buffer::CreateInfo::GetDefault();
			info.Usage = ci.Usage | Buffer::eVertex;
			info.MemProps = ci.MemProps;
			info.VertexStride = ci.VertexStride;

			m_VBO = std::shared_ptr<Buffer>(
				m_Renderer->CreateBuffer(vertexSize, vertexData, info)
			);
		}

		if (indexSize > 0)
		{
			auto info = Buffer::CreateInfo::GetDefault();
			info.Usage = ci.Usage | Buffer::eIndex;
			info.MemProps = ci.MemProps;

			m_IBO = std::shared_ptr<Buffer>(
				m_Renderer->CreateBuffer(indexSize, indexData, info)
			);
		}
	}

	void Mesh::Draw(
		Command* cmd,
		uint32_t instanceCount,
		uint32_t firstIndex,
		int32_t vertexOffset,
		uint32_t firstInstance)
	{
		if (m_VBO)
		{
			m_VBO->Bind(cmd);
		}

		if (m_IBO)
		{
			m_IBO->Bind(cmd);

			cmd->DrawIndexed(
				static_cast<uint32_t>(m_IndexCount),
				instanceCount,
				firstIndex,
				vertexOffset,
				firstInstance
			);
		}
		else
		{
			cmd->Draw(
				static_cast<uint32_t>(m_IndexCount),
				instanceCount,
				vertexOffset,
				firstInstance
			);
		}
	}

	void Mesh::Cleanup()
	{
	}

	std::shared_ptr<Hazel::Mesh> Mesh::BuildCube(Renderer* renderer, const vec3& min, const vec3& max)
	{
		HZ_CORE_INFO("Mesh::BuildCube")

		std::vector<Vertex3D> vertices =
		{
			{ min.x, min.y, min.z, 0.0f, 0.0f },
			{ min.x, min.y, max.z, 1.0f, 0.0f },
			{ min.x, max.y, max.z, 1.0f, 1.0f },
			{ min.x, max.y, min.z, 0.0f, 1.0f },
			{ max.x, min.y, max.z, 0.0f, 0.0f },
			{ max.x, min.y, min.z, 1.0f, 0.0f },
			{ max.x, max.y, min.z, 1.0f, 1.0f },
			{ max.x, max.y, max.z, 0.0f, 1.0f },
			{ min.x, min.y, min.z, 0.0f, 0.0f },
			{ max.x, min.y, min.z, 1.0f, 0.0f },
			{ max.x, min.y, max.z, 1.0f, 1.0f },
			{ min.x, min.y, max.z, 0.0f, 1.0f },
			{ max.x, max.y, min.z, 0.0f, 0.0f },
			{ min.x, max.y, min.z, 1.0f, 0.0f },
			{ min.x, max.y, max.z, 1.0f, 1.0f },
			{ max.x, max.y, max.z, 0.0f, 1.0f },
			{ min.x, min.y, min.z, 0.0f, 0.0f },
			{ min.x, max.y, min.z, 1.0f, 0.0f },
			{ max.x, max.y, min.z, 1.0f, 1.0f },
			{ max.x, min.y, min.z, 0.0f, 1.0f },
			{ min.x, max.y, max.y, 0.0f, 0.0f },
			{ min.x, min.y, max.y, 1.0f, 0.0f },
			{ max.x, min.y, max.y, 1.0f, 1.0f },
			{ max.x, max.y, max.y, 0.0f, 1.0f }
		};

		std::vector<glm::ivec4> quads =
		{
			{  0,  1,  2,  3 },
			{  4,  5,  6,  7 },
			{  8,  9, 10, 11 },
			{ 12, 13, 14, 15 },
			{ 16, 17, 18, 19 },
			{ 20, 21, 22, 23 }
		};

		std::vector<unsigned short> indices;
		for (auto& q : quads)
		{
			indices.push_back(q.x);
			indices.push_back(q.y);
			indices.push_back(q.z);

			indices.push_back(q.x);
			indices.push_back(q.z);
			indices.push_back(q.w);
		}

		Buffer::CreateInfo info = Buffer::CreateInfo::GetDefault();
		info.VertexStride = sizeof(Vertex3D);

		std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(renderer));
		mesh->SetIndexCount(indices.size());
		mesh->Build(
			static_cast<uint32_t>(vertices.size() * sizeof(Vertex3D)),
			vertices.data(),
			static_cast<uint32_t>(indices.size() * sizeof(unsigned short)),
			indices.data(),
			info
		);

		return mesh;
	}

	std::shared_ptr<Hazel::Mesh> Mesh::BuildGrid(Renderer* renderer, unsigned int width, unsigned int height)
	{
		HZ_CORE_INFO("Mesh::BuildGrid")

		std::vector<Vertex3D> vertices;
		std::vector<unsigned short> indices;
		vertices.resize(width * height);

		for (unsigned int i = 0; i < width; ++i)
		{
			for (unsigned int j = 0; j < height; ++j)
			{
				float x = (float)i / (float)(width - 1);
				float y = (float)j / (float)(height - 1);

				vertices[i + j * width] = { x, y, 0.0f, x, y };

				if (i < width - 1 && j < height - 1)
				{
					indices.push_back(i + j * width);
					indices.push_back(i + (j + 1) * width);
					indices.push_back((i + 1) + j * width);

					indices.push_back(i + (j + 1) * width);
					indices.push_back((i + 1) + (j + 1) * width);
					indices.push_back((i + 1) + j * width);
				}
			}
		}

		Buffer::CreateInfo info = Buffer::CreateInfo::GetDefault();
		info.VertexStride = sizeof(Vertex3D);

		std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(renderer));
		mesh->SetIndexCount(indices.size());
		mesh->Build(
			static_cast<uint32_t>(vertices.size() * sizeof(Vertex3D)),
			vertices.data(),
			static_cast<uint32_t>(indices.size() * sizeof(unsigned short)),
			indices.data(),
			info
		);

		return mesh;
	}

	std::shared_ptr<Hazel::Mesh> Mesh::BuildPatch(Renderer* renderer, unsigned int width, unsigned int height)
	{
		HZ_CORE_INFO("Mesh::BuildPatch")

		std::vector<Vertex3D> vertices;
		std::vector<unsigned short> indices;
		vertices.resize(width * height);

		for (unsigned int i = 0; i < width; ++i)
		{
			for (unsigned int j = 0; j < height; ++j)
			{
				float x = (float)i / (float)(width - 1);
				float y = (float)j / (float)(height - 1);

				vertices[i + j * width] = { x, y, 0.0f, x, y };

				if (i < width - 1 && j < height - 1)
				{
					unsigned short p1 = i + j * width;
					unsigned short p2 = p1 + width;
					unsigned short p3 = p2 + 1;
					unsigned short p4 = p1 + 1;

					indices.push_back(p1);
					indices.push_back(p2);
					indices.push_back(p3);
					indices.push_back(p4);
				}
			}
		}

		Buffer::CreateInfo info = Buffer::CreateInfo::GetDefault();
		info.VertexStride = sizeof(Vertex3D);

		std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(renderer));
		mesh->SetIndexCount(indices.size());
		mesh->Build(
			static_cast<uint32_t>(vertices.size() * sizeof(Vertex3D)),
			vertices.data(),
			static_cast<uint32_t>(indices.size() * sizeof(unsigned short)),
			indices.data(),
			info
		);

		return mesh;
	}

	std::shared_ptr<Hazel::Mesh> Mesh::BuildQuad(Renderer* renderer, float width, float height, const Color& color)
	{
		HZ_CORE_INFO("Mesh::BuildQuad")

		Vertex2D data[] = 
		{
			{ -width, -height, 0.0f, 0.0f, color.R, color.G, color.B, color.A },
			{  width, -height, 1.0f, 0.0f, color.R, color.G, color.B, color.A },
			{ -width,  height, 0.0f, 1.0f, color.R, color.G, color.B, color.A },
			{  width,  height, 1.0f, 1.0f, color.R, color.G, color.B, color.A }
		};

		Buffer::CreateInfo info = Buffer::CreateInfo::GetDefault();
		info.VertexStride = sizeof(Vertex2D);

		std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh(renderer));
		mesh->SetIndexCount(4);
		mesh->Build(4 * sizeof(Vertex2D), &data[0], 0U, nullptr, info);
		
		return mesh;
	}
	
}