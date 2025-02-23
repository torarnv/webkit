/*
 Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#ifndef TextureMapperGL_h
#define TextureMapperGL_h

#if USE(ACCELERATED_COMPOSITING)

#include "FloatQuad.h"
#include "IntSize.h"
#include "OpenGLShims.h"
#include "TextureMapper.h"
#include "TransformationMatrix.h"

namespace WebCore {

class TextureMapperGLData;
class GraphicsContext;
class TextureMapperShaderProgram;

// An OpenGL-ES2 implementation of TextureMapper.
class TextureMapperGL : public TextureMapper {
public:
    TextureMapperGL();
    virtual ~TextureMapperGL();

    enum Flag {
        SupportsBlending = 0x01,
        ShouldFlipTexture = 0x02
    };

    typedef int Flags;

    // reimps from TextureMapper
    virtual void drawTexture(const BitmapTexture&, const FloatRect&, const TransformationMatrix&, float opacity, const BitmapTexture* maskTexture);
    virtual void drawTexture(uint32_t texture, Flags, const IntSize& textureSize, const FloatRect& targetRect, const TransformationMatrix& modelViewMatrix, float opacity, const BitmapTexture* maskTexture);
    virtual void bindSurface(BitmapTexture* surface);
    virtual void beginClip(const TransformationMatrix&, const FloatRect&);
    virtual void beginPainting(PaintFlags = 0);
    virtual void endPainting();
    virtual void endClip();
    virtual IntSize maxTextureSize() { return IntSize(2000, 2000); }
    virtual PassRefPtr<BitmapTexture> createTexture();
    virtual const char* type() const;
    static PassOwnPtr<TextureMapperGL> create() { return adoptPtr(new TextureMapperGL); }
    void setGraphicsContext(GraphicsContext* context) { m_context = context; }
    GraphicsContext* graphicsContext() { return m_context; }
    virtual bool isOpenGLBacked() const { return true; }
    void platformUpdateContents(NativeImagePtr, const IntRect&, const IntRect&);
    virtual AccelerationMode accelerationMode() const { return OpenGLMode; }

private:
    bool beginScissorClip(const TransformationMatrix&, const FloatRect&);
    inline TextureMapperGLData& data() { return *m_data; }
    TextureMapperGLData* m_data;
    GraphicsContext* m_context;
    friend class BitmapTextureGL;
};

class BitmapTextureGL : public BitmapTexture {
public:
    virtual IntSize size() const;
    virtual bool isValid() const;
    virtual bool canReuseWith(const IntSize& contentsSize, Flags = 0);
    virtual void didReset();
    void bind();
    void initializeStencil();
    ~BitmapTextureGL();
    virtual uint32_t id() const { return m_id; }
    uint32_t textureTarget() const { return GL_TEXTURE_2D; }
    IntSize textureSize() const { return m_textureSize; }
    void setTextureMapper(TextureMapperGL* texmap) { m_textureMapper = texmap; }
    void updateContents(Image*, const IntRect&, const IntPoint&);
    virtual void updateContents(const void*, const IntRect& target, const IntPoint& sourceOffset, int bytesPerLine);
    virtual bool isBackedByOpenGL() const { return true; }

private:
    GLuint m_id;
    IntSize m_textureSize;
    IntRect m_dirtyRect;
    GLuint m_fbo;
    GLuint m_rbo;
    bool m_surfaceNeedsReset;
    TextureMapperGL* m_textureMapper;
    BitmapTextureGL()
        : m_id(0)
        , m_fbo(0)
        , m_rbo(0)
        , m_surfaceNeedsReset(true)
        , m_textureMapper(0)
    {
    }

    friend class TextureMapperGL;
};

typedef uint64_t ImageUID;
ImageUID uidForImage(Image*);
BitmapTextureGL* toBitmapTextureGL(BitmapTexture*);

}
#endif

#endif
