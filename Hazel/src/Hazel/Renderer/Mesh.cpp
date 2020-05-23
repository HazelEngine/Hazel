#include "hzpch.h"
#include "Mesh.h"

#include "Hazel/Renderer/Renderer.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/pbrmaterial.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <filesystem>

namespace Hazel {

#define MESH_DEBUG_LOG 1
#if MESH_DEBUG_LOG
	#define HZ_MESH_LOG(...) HZ_CORE_TRACE(__VA_ARGS__)
#else
	#define HZ_MESH_LOG(...)
#endif

	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
		aiProcess_Triangulate |             // Make sure we're triangles
		aiProcess_SortByPType |             // Split meshes by primitive type
		aiProcess_GenNormals |              // Make sure we have legit normals
		aiProcess_GenUVCoords |             // Convert UVs if required 
		aiProcess_OptimizeMeshes |          // Batch draws where possible
		aiProcess_ValidateDataStructure;    // Validation

	struct LogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
			}
		}

		virtual void write(const char* message) override
		{
			HZ_CORE_ERROR("Assimp error: {0}", message)
		}
	};

	glm::mat4 Mat4FromAssimpMat4(const aiMatrix4x4& matrix)
	{
		glm::mat4 result;
		
		// The a, b, c, d in Assimp is the row; the 1, 2, 3, 4 is the column
		result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
		result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
		result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
		result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
		
		return result;
	}

	Mesh::Mesh(const std::string& path)
		: m_FilePath(path)
	{
		LogStream::Initialize();

		HZ_CORE_INFO("Loading mesh: {0}", path)

		m_Importer = CreateScope<Assimp::Importer>();

		const aiScene* scene = m_Importer->ReadFile(path, s_MeshImportFlags);
		if (!scene || !scene->HasMeshes())
		{
			HZ_CORE_ERROR("Failed to load mesh file: {0}", path)
			HZ_CORE_ASSERT(true, "Failed to load mesh file!")
		}

		m_Scene = scene;

		m_IsAnimated = scene->mAnimations != nullptr;
		if (m_IsAnimated)
		{
			m_MeshShader = Shader::Create(
				"PBR_Anim",
				"assets/Shaders/Compiled/PBR_Anim.vert.spv",
				"assets/Shaders/Compiled/PBR.frag.spv"
			);
		}
		else
		{
			m_MeshShader = Shader::Create(
				"PBR_Static",
				"assets/Shaders/Compiled/PBR_Static.vert.spv",
				"assets/Shaders/Compiled/PBR.frag.spv"
			);
		}
		
		m_BaseMaterial = Material::Create(m_MeshShader);
		m_InverseTransform = glm::inverse(Mat4FromAssimpMat4(scene->mRootNode->mTransformation));

		uint32_t vertexCount = 0, indexCount = 0;

		// Meshes
		m_Submeshes.reserve(scene->mNumMeshes);
		for (size_t m = 0; m < scene->mNumMeshes; m++)
		{
			aiMesh* mesh = scene->mMeshes[m];

			Submesh& submesh = m_Submeshes.emplace_back();
			submesh.BaseVertex = vertexCount;
			submesh.BaseIndex = indexCount;
			submesh.MaterialIndex = mesh->mMaterialIndex;
			submesh.IndexCount = mesh->mNumFaces * 3;

			vertexCount += mesh->mNumVertices;
			indexCount += submesh.IndexCount;

			HZ_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.")
			HZ_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.")

			// Vertices
			if (m_IsAnimated)
			{
				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					AnimatedVertex vertex;

					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

					if (mesh->HasTangentsAndBitangents())
					{
						vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
						vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
						vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

					m_AnimatedVertices.push_back(vertex);
				}
			}
			else
			{
				submesh.Min = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
				submesh.Max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

				for (size_t i = 0; i < mesh->mNumVertices; i++)
				{
					Vertex vertex;

					vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
					vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

					submesh.Min.x = glm::min(vertex.Position.x, submesh.Min.x);
					submesh.Min.y = glm::min(vertex.Position.y, submesh.Min.y);
					submesh.Min.z = glm::min(vertex.Position.z, submesh.Min.z);
					submesh.Max.x = glm::max(vertex.Position.x, submesh.Max.x);
					submesh.Max.y = glm::max(vertex.Position.y, submesh.Max.y);
					submesh.Max.z = glm::max(vertex.Position.z, submesh.Max.z);

					if (mesh->HasTangentsAndBitangents())
					{
						vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
						vertex.Binormal = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
					}

					if (mesh->HasTextureCoords(0))
						vertex.TexCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

					m_StaticVertices.push_back(vertex);
				}
			}

			// Indices
			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				HZ_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices.")
				m_Indices.push_back({
					mesh->mFaces[i].mIndices[0],
					mesh->mFaces[i].mIndices[1],
					mesh->mFaces[i].mIndices[2]
				});
			}
		}

		TraverseNodes(scene->mRootNode);

		// Bones
		if (m_IsAnimated)
		{
			for (size_t m = 0; m < scene->mNumMeshes; m++)
			{
				aiMesh* mesh = scene->mMeshes[m];
				Submesh& submesh = m_Submeshes[m];

				// Process the children bones
				for (size_t i = 0; i < mesh->mNumBones; i++)
				{
					aiBone* bone = mesh->mBones[i];
					std::string boneName(bone->mName.data);
					int boneIndex = 0;

					if (m_BoneMapping.find(boneName) == m_BoneMapping.end())
					{
						// Allocate an index for a new bone
						boneIndex = m_BoneCount++;
						BoneInfo info;
						m_BoneInfo.push_back(info);
						m_BoneInfo[boneIndex].BoneOffset = Mat4FromAssimpMat4(bone->mOffsetMatrix);
						m_BoneMapping[boneName] = boneIndex;
					}
					else
					{
						HZ_MESH_LOG("Found existing bone in map")
						boneIndex = m_BoneMapping[boneName];
					}

					for (size_t j = 0; j < bone->mNumWeights; j++)
					{
						int vertexId = submesh.BaseVertex + bone->mWeights[j].mVertexId;
						float weight = bone->mWeights[j].mWeight;
						m_AnimatedVertices[vertexId].AddBoneData(boneIndex, weight);
					}
				}
			}
		}

		m_BoneTransforms.resize(m_BoneCount);

		// Materials
		if (scene->HasMaterials())
		{
			m_Materials.resize(scene->mNumMaterials);

			for (uint32_t i = 0; i < scene->mNumMaterials; i++)
			{
				auto aiMaterial = scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();

				auto instance = MaterialInstance::Create(m_BaseMaterial);
				m_Materials[i] = instance;

				HZ_CORE_INFO("Material name = {0}; Index = {1}", aiMaterialName.data, i)
				aiString aiTexPath;

				// Properties
				aiColor3D aiColor;
				float aiMetalness, aiRoughness;
				aiMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, aiColor);
				aiMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, aiMetalness);
				aiMaterial->Get(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, aiRoughness);
				HZ_MESH_LOG("  Albedo color = {0}, {1}, {2}", aiColor.r, aiColor.g, aiColor.b)
				HZ_MESH_LOG("  Metalness = {0}", aiMetalness)
				HZ_MESH_LOG("  Roughness = {0}", aiRoughness)

				instance->Set("AlbedoColor", glm::vec3(aiColor.r, aiColor.g, aiColor.b));
				instance->Set("Metalness", aiMetalness);
				instance->Set("Roughness", aiRoughness);

				// Albedo map
				if (aiMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &aiTexPath) == AI_SUCCESS)
				{
					// TODO: Temp - this should be handled by Hazel's filesystem
					std::filesystem::path filePath = path;
					auto parentPath = filePath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();

					auto texture = Texture2D::Create(texturePath);
					if (texture)
					{
						m_Textures.push_back(texture);
						HZ_MESH_LOG("  Albedo texture path = {0}", texturePath)
						instance->Set("u_AlbedoTexture", m_Textures[i]);
						instance->Set("EnableAlbedoTexture", true);
					}
					else
					{
						HZ_CORE_ERROR("Could not load texture '{0}'", texturePath)
					}
				}
				else
				{
					HZ_MESH_LOG("  Mesh has no albedo texture")
					instance->Set("EnableAlbedoTexture", false);
				}

				// Normal map
				if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS)
				{
					// TODO: Temp - this should be handled by Hazel's filesystem
					std::filesystem::path filePath = path;
					auto parentPath = filePath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();

					auto texture = Texture2D::Create(texturePath);
					if (texture)
					{
						HZ_MESH_LOG("  Normal texture path = {0}", texturePath)
						instance->Set("u_NormalTexture", texture);
						instance->Set("EnableNormalTexture", true);
					}
					else 
					{
						HZ_CORE_ERROR("Could not load texture '{0}'", texturePath)
					}
				}
				else
				{
					HZ_MESH_LOG("  Mesh has no normal texture")
					instance->Set("EnableNormalTexture", false);
				}

				// Roughness/Metalness map
				if (aiMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &aiTexPath) == AI_SUCCESS)
				{
					// TODO: Temp - this should be handled by Hazel's filesystem
					std::filesystem::path filePath = path;
					auto parentPath = filePath.parent_path();
					parentPath /= std::string(aiTexPath.data);
					std::string texturePath = parentPath.string();

					auto texture = Texture2D::Create(texturePath);
					if (texture)
					{
						HZ_MESH_LOG("  Roughness/Metalness texture path = {0}", texturePath)
						instance->Set("u_RoughnessMetalnessTexture", texture);
						instance->Set("EnableRoughnessMetalnessTexture", true);
					}
					else
					{
						HZ_CORE_ERROR("Could not load texture '{0}'", texturePath)
					}
				}
				else
				{
					HZ_MESH_LOG("  Mesh has no roughness/metalness texture")
					instance->Set("EnableRoughnessMetalnessTexture", false);
				}

				// Bind the instance to upload params to shader
				instance->Bind();
			}
		}

		// VertexBuffer
		if (m_IsAnimated)
		{
			m_VertexBuffer = VertexBuffer::Create(
				m_AnimatedVertices.data(),
				m_AnimatedVertices.size() * sizeof(AnimatedVertex)
			);

			m_VertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position"    },
				{ ShaderDataType::Float3, "a_Normal"      },
				{ ShaderDataType::Float3, "a_Tangent"     },
				{ ShaderDataType::Float3, "a_Binormal"    },
				{ ShaderDataType::Float2, "a_TexCoord"    },
				{ ShaderDataType::Int4,   "a_BoneIndices" },
				{ ShaderDataType::Float4, "a_BoneWeights" }
			});
		}
		else
		{
			m_VertexBuffer = VertexBuffer::Create(
				m_StaticVertices.data(),
				m_StaticVertices.size() * sizeof(Vertex)
			);

			m_VertexBuffer->SetLayout({
				{ ShaderDataType::Float3, "a_Position"    },
				{ ShaderDataType::Float3, "a_Normal"      },
				{ ShaderDataType::Float3, "a_Tangent"     },
				{ ShaderDataType::Float3, "a_Binormal"    },
				{ ShaderDataType::Float2, "a_TexCoord"    }
			});
		}

		// IndexBuffer
		m_IndexBuffer = IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));
	}

	Mesh::~Mesh() {}

	void Mesh::OnUpdate(Timestep ts)
	{
		if (m_IsAnimated)
		{
			if (m_AnimationPlaying)
			{
				m_WorldTime += ts;

				float ticksPerSecond = (float)(m_Scene->mAnimations[0]->mTicksPerSecond != 0 ? m_Scene->mAnimations[0]->mTicksPerSecond : 25.0f) * m_TimeMultiplier;
				m_AnimationTime += ts * ticksPerSecond;
				m_AnimationTime = fmod(m_AnimationTime, (float)m_Scene->mAnimations[0]->mDuration);
			}

			// TODO: We only need to recalc bones if rendering has been requested at the current animation frame
			BoneTransform(m_AnimationTime);
		}
	}

	void Mesh::DumpVertexBuffer()
	{
		// TODO: Implement!
	}

	void Mesh::BoneTransform(float time)
	{
		ReadNodeHierarchy(time, m_Scene->mRootNode, glm::mat4(1.0f));
		//m_BoneTransforms.resize(m_BoneCount);
		for (size_t i = 0; i < m_BoneCount; i++)
			m_BoneTransforms[i] = m_BoneInfo[i].FinalTransformation;
	}

	void Mesh::ReadNodeHierarchy(float animTime, const aiNode* node, const glm::mat4& parentTransform)
	{
		std::string name(node->mName.data);
		const aiAnimation* animation = m_Scene->mAnimations[0];
		glm::mat4 nodeTransform(Mat4FromAssimpMat4(node->mTransformation));
		const aiNodeAnim* nodeAnim = FindNodeAnim(animation, name);

		if (nodeAnim)
		{
			glm::vec3 translation = InterpolateTranslation(animTime, nodeAnim);
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);

			glm::quat rotation = InterpolateRotation(animTime, nodeAnim);
			glm::mat4 rotationMatrix = glm::toMat4(rotation);

			glm::vec3 scale = InterpolateScale(animTime, nodeAnim);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

			nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
		}

		glm::mat4 transform = parentTransform * nodeTransform;

		if (m_BoneMapping.find(name) != m_BoneMapping.end())
		{
			uint32_t boneIndex = m_BoneMapping[name];
			glm::mat4 boneOffset = m_BoneInfo[boneIndex].BoneOffset;
			m_BoneInfo[boneIndex].FinalTransformation = m_InverseTransform * transform * boneOffset;
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			ReadNodeHierarchy(animTime, node->mChildren[i], transform);
	}

	void Mesh::TraverseNodes(aiNode* node, const glm::mat4& parentTransform, uint32_t level)
	{
		glm::mat4 transform = parentTransform * Mat4FromAssimpMat4(node->mTransformation);
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			uint32_t mesh = node->mMeshes[i];
			auto& submesh = m_Submeshes[mesh];
			submesh.Transform = transform;
		}

		// HZ_MESH_LOG("{0} {1}", LevelToSpaces(level), node->mName.C_Str());

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			TraverseNodes(node->mChildren[i], transform, level + 1);
	}

	const aiNodeAnim* Mesh::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
	{
		for (uint32_t i = 0; i < animation->mNumChannels; i++)
		{
			const aiNodeAnim* nodeAnim = animation->mChannels[i];
			if (std::string(nodeAnim->mNodeName.data) == nodeName)
				return nodeAnim;
		}
		return nullptr;
	}

	uint32_t Mesh::FindPosition(float animTime, const aiNodeAnim* nodeAnim)
	{
		HZ_CORE_ASSERT(nodeAnim->mNumPositionKeys > 0, "")

		for (uint32_t i = 0; i < nodeAnim->mNumPositionKeys - 1; i++)
		{
			if (animTime < (float)nodeAnim->mPositionKeys[i + 1].mTime)
				return i;
		}

		return 0;
	}

	uint32_t Mesh::FindRotation(float animTime, const aiNodeAnim* nodeAnim)
	{
		HZ_CORE_ASSERT(nodeAnim->mNumRotationKeys > 0, "")

		for (uint32_t i = 0; i < nodeAnim->mNumRotationKeys - 1; i++)
		{
			if (animTime < (float)nodeAnim->mRotationKeys[i + 1].mTime)
				return i;
		}

		return 0;
	}

	uint32_t Mesh::FindScaling(float animTime, const aiNodeAnim* nodeAnim)
	{
		HZ_CORE_ASSERT(nodeAnim->mNumScalingKeys > 0, "")

		for (uint32_t i = 0; i < nodeAnim->mNumScalingKeys - 1; i++)
		{
			if (animTime < (float)nodeAnim->mScalingKeys[i + 1].mTime)
				return i;
		}

		return 0;
	}

	glm::vec3 Mesh::InterpolateTranslation(float animTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumPositionKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mPositionKeys[0].mValue;
			return glm::vec3(v.x, v.y, v.z);
		}

		uint32_t posIndex = FindPosition(animTime, nodeAnim);
		uint32_t nextPosIndex = (posIndex + 1);
		HZ_CORE_ASSERT(nextPosIndex < nodeAnim->mNumPositionKeys, "")

		float dt = (float)(nodeAnim->mPositionKeys[nextPosIndex].mTime - nodeAnim->mPositionKeys[posIndex].mTime);
		float factor = (animTime - (float)nodeAnim->mPositionKeys[posIndex].mTime) / dt;
		if (factor < 0.0f) factor = 0.0f;
		HZ_CORE_ASSERT(factor <= 1.0f, "Factor must be below 1.0f")

		const aiVector3D& start = nodeAnim->mPositionKeys[posIndex].mValue;
		const aiVector3D& end = nodeAnim->mPositionKeys[nextPosIndex].mValue;
		aiVector3D delta = end - start;
		auto aiVec = start + factor * delta;

		return { aiVec.x, aiVec.y, aiVec.z };
	}

	glm::quat Mesh::InterpolateRotation(float animTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumRotationKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mRotationKeys[0].mValue;
			return glm::quat(v.w, v.x, v.y, v.z);
		}

		uint32_t rotIndex = FindRotation(animTime, nodeAnim);
		uint32_t nextRotIndex = (rotIndex + 1);
		HZ_CORE_ASSERT(nextRotIndex < nodeAnim->mNumRotationKeys, "")

		float dt = (float)(nodeAnim->mRotationKeys[nextRotIndex].mTime - nodeAnim->mRotationKeys[rotIndex].mTime);
		float factor = (animTime - (float)nodeAnim->mRotationKeys[rotIndex].mTime) / dt;
		if (factor < 0.0f) factor = 0.0f;
		HZ_CORE_ASSERT(factor <= 1.0f, "Factor must be below 1.0f")

		const aiQuaternion& start = nodeAnim->mRotationKeys[rotIndex].mValue;
		const aiQuaternion& end = nodeAnim->mRotationKeys[nextRotIndex].mValue;
		auto q = aiQuaternion();
		aiQuaternion::Interpolate(q, start, end, factor);
		q = q.Normalize();

		return glm::quat(q.w, q.x, q.y, q.z);
	}

	glm::vec3 Mesh::InterpolateScale(float animTime, const aiNodeAnim* nodeAnim)
	{
		if (nodeAnim->mNumScalingKeys == 1)
		{
			// No interpolation necessary for single value
			auto v = nodeAnim->mScalingKeys[0].mValue;
			return glm::vec3(v.x, v.y, v.z);
		}

		uint32_t scaleIndex = FindScaling(animTime, nodeAnim);
		uint32_t nextScaleIndex = (scaleIndex + 1);
		HZ_CORE_ASSERT(nextScaleIndex < nodeAnim->mNumScalingKeys, "")

		float dt = (float)(nodeAnim->mScalingKeys[nextScaleIndex].mTime - nodeAnim->mScalingKeys[scaleIndex].mTime);
		float factor = (animTime - (float)nodeAnim->mScalingKeys[scaleIndex].mTime) / dt;
		if (factor < 0.0f) factor = 0.0f;
		HZ_CORE_ASSERT(factor <= 1.0f, "Factor must be below 1.0f")

		const aiVector3D& start = nodeAnim->mScalingKeys[scaleIndex].mValue;
		const aiVector3D& end = nodeAnim->mScalingKeys[nextScaleIndex].mValue;
		aiVector3D delta = end - start;
		auto aiVec = start + factor * delta;

		return { aiVec.x, aiVec.y, aiVec.z };
	}

}