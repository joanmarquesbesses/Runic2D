#pragma once

#include "Runic2D/Renderer/FrameBuffer.h"

namespace Runic2D
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }
		
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }

		void Invalidate();
	
	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment;
		uint32_t m_DepthAttachment;
		FrameBufferSpecification m_Specification;
	};
}


