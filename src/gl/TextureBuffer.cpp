#include "gl/TextureBuffer.h"
#include "gl/DepthBuffer.h"

TextureBuffer::TextureBuffer(
	ovrSession session, bool rendertarget, bool displayableOnHmd, Sizei size, int mipLevels, unsigned char * data) :
	Session(session),
	texChain(nullptr),
	texId(0),
	fboId(0),
	texSize(0, 0)
{

	UNREFERENCED_PARAMETER(data);
	UNREFERENCED_PARAMETER(displayableOnHmd);

	texSize = size;

	// This texture isn't necessarily going to be a rendertarget, but it usually is.
	assert(session); // No HMD? A little odd.

	ovrTextureSwapChainDesc desc = {};
	desc.Type = ovrTexture_2D;
	desc.ArraySize = 1;
	desc.Width = size.w;
	desc.Height = size.h;
	desc.MipLevels = 1;
	desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleCount = 1;
	desc.StaticImage = ovrFalse;

	ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &texChain);

	int length = 0;
	ovr_GetTextureSwapChainLength(session, texChain, &length);

	if (OVR_SUCCESS(result))
	{
		for (int i = 0; i < length; ++i)
		{
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(Session, texChain, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);

			if (rendertarget)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
		}
	}

	if (mipLevels > 1)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glGenFramebuffers(1, &fboId);
}

TextureBuffer::TextureBuffer(Sizei size, int mipLevels, unsigned char * data, bool renderTarget) :
	Session(nullptr),
	texChain(nullptr),
	texId(0),
	fboId(0),
	texSize(size)
{

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	if (renderTarget)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texSize.w, texSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);


	if (mipLevels > 1)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glGenFramebuffers(1, &fboId);
}



TextureBuffer::~TextureBuffer()
{
	if (texChain)
	{
		ovr_DestroyTextureSwapChain(Session, texChain);
		texChain = nullptr;
	}
	if (texId)
	{
		glDeleteTextures(1, &texId);
		texId = 0;
	}
	if (fboId)
	{
		glDeleteFramebuffers(1, &fboId);
		fboId = 0;
	}
}

Sizei TextureBuffer::GetSize() const
{
	return texSize;
}

void TextureBuffer::SetAndClearRenderSurface(DepthBuffer* dbuffer)
{
	GLuint curTexId;
	if (texChain)
	{
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(Session, texChain, &curIndex);
		ovr_GetTextureSwapChainBufferGL(Session, texChain, curIndex, &curTexId);
	}
	else
	{
		curTexId = texId;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbuffer->textureId(), 0);

	glViewport(0, 0, texSize.w, texSize.h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void TextureBuffer::UnsetRenderSurface()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
}

void TextureBuffer::Commit()
{
	if (texChain)
	{
		ovr_CommitTextureSwapChain(Session, texChain);
	}
}