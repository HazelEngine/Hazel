#pragma once

#include <Hazel/Core/Timestep.h>
#include <Hazel/Renderer/Buffer.h>
#include <Hazel/Renderer/Shader.h>
#include <Hazel/Renderer/Material.h>

#include <glm/glm.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;

namespace Assimp {
	class Importer;
}

namespace Hazel {

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 TexCoord;
	};

	struct AnimatedVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec3 Binormal;
		glm::vec2 TexCoord;

		uint32_t Ids[4] = { 0, 0, 0, 0 };
		float Weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		void AddBoneData(uint32_t boneId, float weight)
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (Weights[i] == 0.0f)
				{
					Ids[i] = boneId;
					Weights[i] = weight;
					return;
				}
			}

			// TODO: Keep top weights
			HZ_CORE_WARN(
				"Vertex has more than four bones/weights affecting it, extra data will be discarted "
				"(BoneId = {0}, Weight = {1})",
				boneId, 
				weight
			)
		}
	};

	struct Index
	{
		uint32_t V1, V2, V3;
	};

	static_assert(sizeof(Index) == 3 * sizeof(uint32_t));

	struct BoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};

	struct VertexBoneData
	{
		uint32_t Ids[4];
		float Weights[4];

		VertexBoneData()
		{
			memset(Ids, 0, sizeof(Ids));
			memset(Weights, 0, sizeof(Weights));
		}

		void AddBoneData(uint32_t boneId, float weight)
		{
			for (size_t i = 0; i < 4; i++)
			{
				if (Weights[i] == 0.0f)
				{
					Ids[i] = boneId;
					Weights[i] = weight;
					return;
				}
			}

			// Should never get here - more bones than we have space for
			HZ_CORE_ASSERT(false, "Too many bones!")
		}
	};

	class HAZEL_API Submesh
	{
	public:
		uint32_t BaseVertex;
		uint32_t BaseIndex;
		uint32_t MaterialIndex;
		uint32_t IndexCount;

		glm::mat4 Transform;
		glm::vec3 Min, Max; // TODO: AABB
	};

	class HAZEL_API Mesh
	{
		friend class OpenGLRenderCommandBuffer;
		friend class VulkanRenderCommandBuffer;

	public:
		Mesh(const std::string& path);
		~Mesh();

		void OnUpdate(Timestep ts);

		void DumpVertexBuffer();

		Ref<Shader> GetMeshShader() { return m_MeshShader; }
		Ref<Material> GetMaterial() { return m_BaseMaterial; }
		std::vector<Ref<MaterialInstance>> GetMaterials() { return m_Materials; }
		const std::vector<Ref<Texture2D>>& GetTextures() const { return m_Textures; }
		
		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }

		Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
		Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

		const std::string& GetFilePath() const { return m_FilePath; }
		bool IsAnimated() const { return m_IsAnimated; }

	private:
		void BoneTransform(float time);
		void ReadNodeHierarchy(float animTime, const aiNode* node, const glm::mat4& parentTransform);
		void TraverseNodes(aiNode* node, const glm::mat4& parentTransform = glm::mat4(1.0f), uint32_t level = 0);

		const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName);
		uint32_t FindPosition(float animTime, const aiNodeAnim* nodeAnim);
		uint32_t FindRotation(float animTime, const aiNodeAnim* nodeAnim);
		uint32_t FindScaling(float animTime, const aiNodeAnim* nodeAnim);
		glm::vec3 InterpolateTranslation(float animTime, const aiNodeAnim* nodeAnim);
		glm::quat InterpolateRotation(float animTime, const aiNodeAnim* nodeAnim);
		glm::vec3 InterpolateScale(float animTime, const aiNodeAnim* nodeAnim);

	private:
		Scope<Assimp::Importer> m_Importer;

		std::vector<Submesh> m_Submeshes;

		glm::mat4 m_InverseTransform;

		uint32_t m_BoneCount = 0;
		std::vector<BoneInfo> m_BoneInfo;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;

		std::vector<Vertex> m_StaticVertices;
		std::vector<AnimatedVertex> m_AnimatedVertices;
		std::vector<Index> m_Indices;

		std::unordered_map<std::string, uint32_t> m_BoneMapping;
		std::vector<glm::mat4> m_BoneTransforms;

		const aiScene* m_Scene;

		// Materials
		Ref<Shader> m_MeshShader;
		Ref<Material> m_BaseMaterial;
		std::vector<Ref<Texture2D>> m_Textures;
		std::vector<Ref<Texture2D>> m_NormalMaps;
		std::vector<Ref<MaterialInstance>> m_Materials;

		// Animation
		bool  m_IsAnimated = false;
		float m_AnimationTime = 0.0f;
		float m_WorldTime = 0.0f;
		float m_TimeMultiplier = 1.0f;
		bool  m_AnimationPlaying = true;

		std::string m_FilePath;
	};

}