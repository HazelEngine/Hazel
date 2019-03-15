#pragma once

#include "Hazel/Core.h"

namespace Hazel {

	// Forward declarations:
	class Renderer;
	class Command;

	class HAZEL_API Buffer
	{
	public:
		//! TYPEDEF/ENUMS:
		enum UsageFlags
		{
			eTransferSrc     = 1 << 1,
			eTransferDst     = 1 << 2,
			eUniformTexel    = 1 << 3,
			eStorageTexel    = 1 << 4,
			eUniform         = 1 << 5,
			eStorage         = 1 << 6,
			eIndex           = 1 << 7,
			eVertex          = 1 << 8,
			eIndirect        = 1 << 9
		};

		enum MemoryPropertyFlags
		{
			eDeviceLocal     = 1 << 1,
			eHostVisible     = 1 << 2,
			eHostCoherent    = 1 << 3,
			eHostCached      = 1 << 4,
			eLazilyAllocated = 1 << 5
		};

		struct CreateInfo
		{
			//! SERVICES:
			static CreateInfo GetDefault();
			static CreateInfo GetStaging();

			//! MEMBERS:
			uint32_t Usage;
			uint32_t MemProps;
			uint32_t VertexStride;
			bool Dynamic;
		};

		//! CTOR/DTOR:
		Buffer(Renderer* renderer);
		virtual ~Buffer();

		//! VIRTUALS:
		virtual void Build(uint32_t size, const void* data, const CreateInfo& ci) = 0;
		virtual void* Map(uint32_t size, uint32_t offset) = 0;
		virtual void Unmap() = 0;
		virtual void Flush() = 0;
		virtual void Bind(Command* cmd) = 0;
		virtual void Cleanup() = 0;

		//! ACCESSORS:
		void* GetMappedMemory() const;
		uint32_t GetSize() const;
		uint32_t GetVertexStride() const;
		bool IsDynamic() const;
		bool IsMapped() const;

	protected:
		//! MEMBERS:
		Renderer* m_Renderer;
		uint32_t m_Size;
		uint32_t m_Usage;
		uint32_t m_VertexStride;
		bool m_IsDynamic;
		bool m_IsMapped;
		void* m_Mapped;
	};

	////////////////////////////////////////////////////////////////////////////////
	// Buffer::CreateInfo inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline Buffer::CreateInfo Buffer::CreateInfo::GetDefault()
	{
		return CreateInfo() =
		{
			eTransferDst,
			eDeviceLocal,
			0,
			false
		};
	}

	inline Buffer::CreateInfo Buffer::CreateInfo::GetStaging()
	{
		return CreateInfo() =
		{
			eTransferSrc,
			eHostVisible,
			0,
			false
		};
	}

	////////////////////////////////////////////////////////////////////////////////
	// Buffer inline implementation:
	////////////////////////////////////////////////////////////////////////////////

	inline Buffer::Buffer(Renderer* renderer) :
		m_Renderer(renderer),
		m_Size(0),
		m_Usage(eTransferDst),
		m_VertexStride(0),
		m_IsDynamic(false),
		m_IsMapped(false),
		m_Mapped(nullptr)
	{
	}

	inline Buffer::~Buffer()
	{
	}

	inline void* Buffer::GetMappedMemory() const
	{
		return m_Mapped;
	}

	inline uint32_t Buffer::GetSize() const
	{
		return m_Size;
	}

	inline uint32_t Buffer::GetVertexStride() const
	{
		return m_VertexStride;
	}

	inline bool Buffer::IsDynamic() const
	{
		return m_IsDynamic;
	}

	inline bool Buffer::IsMapped() const
	{
		return m_IsMapped;
	}

}