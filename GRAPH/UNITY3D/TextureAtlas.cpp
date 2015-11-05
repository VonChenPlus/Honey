#include "GRAPH/UNITY3D/TextureAtlas.h"
#include "GRAPH/UNITY3D/TextureCache.h"
#include "GRAPH/UNITY3D/GLStateCache.h"

namespace GRAPH
{
    TextureAtlas::TextureAtlas()
        : dirty_(false)
        , texture_(nullptr)
        , u3dVertexBuffer_(nullptr)
        , u3dIndexBuffer_(nullptr)
        , u3dVertexFormat_(nullptr)
        , u3dContext_(Unity3DCreator::CreateContext()) {
    }

    TextureAtlas::~TextureAtlas() {
        SAFE_FREE(vbo_.u2.bufferData);
        SAFE_FREE(vbo_.u2.indexData);
        SAFE_DELETE(u3dVertexBuffer_);
        SAFE_DELETE(u3dIndexBuffer_);
        SAFE_DELETE(u3dVertexFormat_);
        SAFE_DELETE(u3dContext_);
        SAFE_RELEASE(texture_);
    }

    uint64 TextureAtlas::getTotalQuads() const {
        return vbo_.u2.bufferCount;
    }

    uint64 TextureAtlas::getCapacity() const {
        return vbo_.u2.bufferCapacity;
    }

    Unity3DTexture* TextureAtlas::getTexture() const {
        return texture_;
    }

    void TextureAtlas::setTexture(Unity3DTexture * var) {
        SAFE_RETAIN(var);
        SAFE_RELEASE(texture_);
        texture_ = var;
    }

    V3F_C4B_T2F_Quad* TextureAtlas::getQuads() {
        dirty_ = true;
        return vbo_.u2.bufferData;
    }

    void TextureAtlas::setQuads(V3F_C4B_T2F_Quad* quads) {
        vbo_.u2.bufferData = quads;
    }

    // TextureAtlas - alloc & init
    TextureAtlas * TextureAtlas::create(const std::string& file, uint64 capacity) {
        TextureAtlas * textureAtlas = new (std::nothrow) TextureAtlas();
        if(textureAtlas && textureAtlas->initWithFile(file, capacity)) {
            textureAtlas->autorelease();
            return textureAtlas;
        }
        SAFE_DELETE(textureAtlas);
        return nullptr;
    }

    TextureAtlas * TextureAtlas::createWithTexture(Unity3DTexture *texture, uint64 capacity) {
        TextureAtlas * textureAtlas = new (std::nothrow) TextureAtlas();
        if (textureAtlas && textureAtlas->initWithTexture(texture, capacity)) {
            textureAtlas->autorelease();
            return textureAtlas;
        }
        SAFE_DELETE(textureAtlas);
        return nullptr;
    }

    bool TextureAtlas::initWithFile(const std::string& file, uint64 capacity) {
        // retained in property
        Unity3DTexture *texture = TextureCache::getInstance().addImage(file);

        if (texture) {
            return initWithTexture(texture, capacity);
        }

        return false;
    }

    bool TextureAtlas::initWithTexture(Unity3DTexture *texture, uint64 capacity) {
        vbo_.u2.indexCapacity = vbo_.u2.bufferCapacity = capacity;
        vbo_.u2.indexCount = vbo_.u2.bufferCount = 0;

        // retained in property
        this->texture_ = texture;
        SAFE_RETAIN(texture_);

        vbo_.u2.bufferData = (V3F_C4B_T2F_Quad*)malloc( capacity * sizeof(V3F_C4B_T2F_Quad) );
        vbo_.u2.indexData = (GLushort *)malloc( capacity * 6 * sizeof(GLushort) );

        if( ! ( vbo_.u2.bufferData && vbo_.u2.indexData) && capacity > 0) {
            SAFE_FREE(vbo_.u2.bufferData);
            SAFE_FREE(vbo_.u2.indexData);

            // release texture, should set it to null, because the destruction will
            SAFE_RELEASE_NULL(texture_);
            return false;
        }

        memset( vbo_.u2.bufferData, 0, capacity * sizeof(V3F_C4B_T2F_Quad) );
        memset( vbo_.u2.indexData, 0, capacity * 6 * sizeof(GLushort) );

        setupIndices();
        setupBuffer();

        dirty_ = true;

        return true;
    }

    void TextureAtlas::setupIndices() {
        if (vbo_.u2.indexCapacity == 0)
            return;

        for( int i=0; i < vbo_.u2.indexCapacity; i++) {
            vbo_.u2.indexData[i*6+0] = i*4+0;
            vbo_.u2.indexData[i*6+1] = i*4+1;
            vbo_.u2.indexData[i*6+2] = i*4+2;

            // inverted index. issue #179
            vbo_.u2.indexData[i*6+3] = i*4+3;
            vbo_.u2.indexData[i*6+4] = i*4+2;
            vbo_.u2.indexData[i*6+5] = i*4+1;
        }
    }

    void TextureAtlas::setupBuffer() {
        u3dVertexBuffer_ = Unity3DCreator::CreateBuffer(VERTEXDATA | DYNAMIC);
        u3dIndexBuffer_ = Unity3DCreator::CreateBuffer(INDEXDATA);

        u3dVertexBuffer_->setData((const uint8 *) vbo_.u2.bufferData, sizeof(V3F_C4B_T2F) * vbo_.u2.bufferCapacity);
        u3dIndexBuffer_->setData((const uint8 *) vbo_.u2.indexData, sizeof(GLushort) * vbo_.u2.indexCapacity * 6);

        std::vector<Unity3DVertexComponent> vertexFormat = { 
            Unity3DVertexComponent(SEM_POSITION, FLOATx3, offsetof(V3F_C4B_T2F, vertices)),
            Unity3DVertexComponent(SEM_COLOR0, UNORM8x4, offsetof(V3F_C4B_T2F, colors)),
            Unity3DVertexComponent(SEM_TEXCOORD0, FLOATx2, offsetof(V3F_C4B_T2F, texCoords)) };
        u3dVertexFormat_ = Unity3DCreator::CreateVertexFormat(vertexFormat, sizeof(V3F_C4B_T2F));
    }

    // TextureAtlas - Update, Insert, Move & Remove
    void TextureAtlas::updateQuad(V3F_C4B_T2F_Quad *quad, uint64 index) {
        vbo_.u2.bufferCount = MATH::MATH_MAX( index+1, vbo_.u2.bufferCount);
        vbo_.u2.bufferData[index] = *quad;
        dirty_ = true;
    }

    void TextureAtlas::insertQuad(V3F_C4B_T2F_Quad *quad, uint64 index) {
        vbo_.u2.bufferCount++;

        // issue #575. index can be > totalQuads
        auto remaining = (vbo_.u2.bufferCount-1) - index;

        // last object doesn't need to be moved
        if( remaining > 0) {
            // texture coordinates
            memmove( &vbo_.u2.bufferData[index+1],&vbo_.u2.bufferData[index], sizeof(vbo_.u2.bufferData[0]) * remaining );
        }

        vbo_.u2.bufferData[index] = *quad;
        dirty_ = true;
    }

    void TextureAtlas::insertQuads(V3F_C4B_T2F_Quad* quads, uint64 index, uint64 amount) {
        vbo_.u2.bufferCount += amount;
        auto remaining = (vbo_.u2.bufferCount-1) - index - amount;

        // last object doesn't need to be moved
        if( remaining > 0) {
            // tex coordinates
            memmove( &vbo_.u2.bufferData[index+amount],&vbo_.u2.bufferData[index], sizeof(vbo_.u2.bufferData[0]) * remaining );
        }


        auto max = index + amount;
        int j = 0;
        for (uint64 i = index; i < max ; i++) {
            vbo_.u2.bufferData[index] = quads[j];
            index++;
            j++;
        }

        dirty_ = true;
    }

    void TextureAtlas::insertQuadFromIndex(uint64 oldIndex, uint64 newIndex) {
        if( oldIndex == newIndex ) {
            return;
        }
        // because it is ambiguous in iphone, so we implement abs ourselves
        // unsigned int howMany = abs( oldIndex - newIndex);
        auto howMany = (oldIndex - newIndex) > 0 ? (oldIndex - newIndex) :  (newIndex - oldIndex);
        auto dst = oldIndex;
        auto src = oldIndex + 1;
        if( oldIndex > newIndex) {
            dst = newIndex+1;
            src = newIndex;
        }

        // texture coordinates
        V3F_C4B_T2F_Quad quadsBackup = vbo_.u2.bufferData[oldIndex];
        memmove( &vbo_.u2.bufferData[dst],&vbo_.u2.bufferData[src], sizeof(vbo_.u2.bufferData[0]) * howMany );
        vbo_.u2.bufferData[newIndex] = quadsBackup;
        dirty_ = true;
    }

    void TextureAtlas::removeQuadAtIndex(uint64 index) {
        auto remaining = (vbo_.u2.bufferCount-1) - index;

        // last object doesn't need to be moved
        if( remaining ) {
            // texture coordinates
            memmove( &vbo_.u2.bufferData[index],&vbo_.u2.bufferData[index+1], sizeof(vbo_.u2.bufferData[0]) * remaining );
        }
        vbo_.u2.bufferCount--;
        dirty_ = true;
    }

    void TextureAtlas::removeQuadsAtIndex(uint64 index, uint64 amount) {
        auto remaining = (vbo_.u2.bufferCount) - (index + amount);

        vbo_.u2.bufferCount -= amount;

        if ( remaining ) {
            memmove( &vbo_.u2.bufferData[index], &vbo_.u2.bufferData[index+amount], sizeof(vbo_.u2.bufferData[0]) * remaining );
        }

        dirty_ = true;
    }

    void TextureAtlas::removeAllQuads() {
        vbo_.u2.bufferCount = 0;
    }

    // TextureAtlas - Resize
    bool TextureAtlas::resizeCapacity(uint64 newCapacity) {
        if( newCapacity == vbo_.u2.indexCapacity ) {
            return true;
        }
        auto oldCapactiy = vbo_.u2.indexCapacity;
        // update capacity and totolQuads
        vbo_.u2.bufferCount = MATH::MATH_MIN(vbo_.u2.bufferCount, newCapacity);
        vbo_.u2.indexCapacity = newCapacity;

        V3F_C4B_T2F_Quad* tmpQuads = nullptr;
        GLushort* tmpIndices = nullptr;

        // when calling initWithTexture(fileName, 0) on bada device, calloc(0, 1) will fail and return nullptr,
        // so here must judge whether vbo_.u2.bufferData and vbo_.u2.indexData is nullptr.
        if (vbo_.u2.bufferData == nullptr) {
            tmpQuads = (V3F_C4B_T2F_Quad*)malloc( vbo_.u2.indexCapacity * sizeof(vbo_.u2.bufferData[0]) );
            if (tmpQuads != nullptr) {
                memset(tmpQuads, 0, vbo_.u2.indexCapacity * sizeof(vbo_.u2.bufferData[0]) );
            }
        }
        else {
            tmpQuads = (V3F_C4B_T2F_Quad*)realloc( vbo_.u2.bufferData, sizeof(vbo_.u2.bufferData[0]) * vbo_.u2.indexCapacity );
            if (tmpQuads != nullptr && vbo_.u2.indexCapacity > oldCapactiy) {
                memset(tmpQuads+oldCapactiy, 0, (vbo_.u2.indexCapacity - oldCapactiy)*sizeof(vbo_.u2.bufferData[0]) );
            }
            vbo_.u2.bufferData = nullptr;
        }

        if (vbo_.u2.indexData == nullptr) {
            tmpIndices = (GLushort*)malloc( vbo_.u2.indexCapacity * 6 * sizeof(vbo_.u2.indexData[0]) );
            if (tmpIndices != nullptr) {
                memset( tmpIndices, 0, vbo_.u2.indexCapacity * 6 * sizeof(vbo_.u2.indexData[0]) );
            }
        }
        else {
            tmpIndices = (GLushort*)realloc( vbo_.u2.indexData, sizeof(vbo_.u2.indexData[0]) * vbo_.u2.indexCapacity * 6 );
            if (tmpIndices != nullptr && vbo_.u2.indexCapacity > oldCapactiy) {
                memset( tmpIndices+oldCapactiy, 0, (vbo_.u2.indexCapacity-oldCapactiy) * 6 * sizeof(vbo_.u2.indexData[0]) );
            }
            vbo_.u2.indexData = nullptr;
        }

        if( ! ( tmpQuads && tmpIndices) ) {
            SAFE_FREE(tmpQuads);
            SAFE_FREE(tmpIndices);
            SAFE_FREE(vbo_.u2.bufferData);
            SAFE_FREE(vbo_.u2.indexData);
            vbo_.u2.indexCapacity = vbo_.u2.bufferCount = 0;
            return false;
        }

        vbo_.u2.bufferData = tmpQuads;
        vbo_.u2.indexData = tmpIndices;

        setupIndices();
        u3dVertexBuffer_->setData((const uint8 *) vbo_.u2.bufferData, sizeof(V3F_C4B_T2F) * vbo_.u2.bufferCapacity);
        u3dIndexBuffer_->setData((const uint8 *) vbo_.u2.indexData, sizeof(GLushort) * vbo_.u2.indexCapacity * 6);

        dirty_ = true;

        return true;
    }

    void TextureAtlas::increaseTotalQuadsWith(uint64 amount) {
        vbo_.u2.bufferCount += amount;
    }

    void TextureAtlas::moveQuadsFromIndex(uint64 oldIndex, uint64 amount, uint64 newIndex) {
        if( oldIndex == newIndex ) {
            return;
        }
        //create buffer
        uint64 quadSize = sizeof(V3F_C4B_T2F_Quad);
        V3F_C4B_T2F_Quad* tempQuads = (V3F_C4B_T2F_Quad*)malloc( quadSize * amount);
        memcpy( tempQuads, &vbo_.u2.bufferData[oldIndex], quadSize * amount );

        if (newIndex < oldIndex) {
            // move quads from newIndex to newIndex + amount to make room for buffer
            memmove( &vbo_.u2.bufferData[newIndex], &vbo_.u2.bufferData[newIndex+amount], (oldIndex-newIndex)*quadSize);
        }
        else {
            // move quads above back
            memmove( &vbo_.u2.bufferData[oldIndex], &vbo_.u2.bufferData[oldIndex+amount], (newIndex-oldIndex)*quadSize);
        }
        memcpy( &vbo_.u2.bufferData[newIndex], tempQuads, amount*quadSize);
        free(tempQuads);
        dirty_ = true;
    }

    void TextureAtlas::moveQuadsFromIndex(uint64 index, uint64 newIndex) {
        memmove(vbo_.u2.bufferData + newIndex,vbo_.u2.bufferData + index, (vbo_.u2.bufferCount - index) * sizeof(vbo_.u2.bufferData[0]));
    }

    void TextureAtlas::fillWithEmptyQuadsFromIndex(uint64 index, uint64 amount) {
        V3F_C4B_T2F_Quad quad;
        memset(&quad, 0, sizeof(quad));

        auto to = index + amount;
        for (uint64 i = index ; i < to ; i++) {
            vbo_.u2.bufferData[i] = quad;
        }
    }

    // TextureAtlas - Drawing
    void TextureAtlas::drawQuads() {
        this->drawNumberOfQuads(vbo_.u2.bufferCount, 0);
    }

    void TextureAtlas::drawNumberOfQuads(uint64 numberOfQuads) {
        this->drawNumberOfQuads(numberOfQuads, 0);
    }

    void TextureAtlas::drawNumberOfQuads(uint64 numberOfQuads, uint64 start) {
        if(!numberOfQuads)
            return;

        GLStateCache::BindTexture2D(texture_->texture());

        if (dirty_) {
            u3dVertexBuffer_->bind();
            u3dVertexBuffer_->subData((const uint8 *) vbo_.u2.bufferData, 0, sizeof(vbo_.u2.bufferData[0]) * vbo_.u2.bufferCount);
            dirty_ = false;
        }

        u3dContext_->drawIndexed(PRIM_TRIANGLES, u3dVertexFormat_, u3dVertexBuffer_, u3dIndexBuffer_, (void *) (start * 6 * sizeof(GLushort)), (GLsizei) numberOfQuads * 6);
    }
}
