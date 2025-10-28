#pragma once

#include "Runic2D/Core/Core.h"

namespace Runic2D
{

	struct FrameBufferSpecification
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		bool SwapChainTarget = false;
		uint32_t Samples = 1;
		// TODO: Add more specification options
		// e.g. Texture formats, depth buffer, etc.
	};

	class FrameBuffer
	{
	public:
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID() const = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
	};
}

