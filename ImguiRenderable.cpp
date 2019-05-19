/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include <imgui.h>

#include "ImguiRenderable.h"

#include "OgreHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include <OgreSceneManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRoot.h>


namespace Ogre
{
    ImGUIRenderable::ImGUIRenderable()
    {
        initImGUIRenderable();

        //By default we want ImGUIRenderables to still work in wireframe mode
        setPolygonModeOverrideable( false );
        
    }
    //-----------------------------------------------------------------------------------
    void ImGUIRenderable::initImGUIRenderable(void)
    {
        // use identity projection and view matrices
        mUseIdentityProjection  = true;
        mUseIdentityView        = true;

        mRenderOp.vertexData = OGRE_NEW VertexData();
        mRenderOp.indexData  = OGRE_NEW IndexData();

        mRenderOp.vertexData->vertexCount   = 0;
        mRenderOp.vertexData->vertexStart   = 0;

        mRenderOp.indexData->indexCount = 0;
        mRenderOp.indexData->indexStart = 0;
        mRenderOp.operationType             = RenderOperation::OT_TRIANGLE_LIST;
        mRenderOp.useIndexes                                    = true; 
        mRenderOp.useGlobalInstancingVertexBufferIsAvailable    = false;

        VertexDeclaration* decl     = mRenderOp.vertexData->vertexDeclaration;
        
        // vertex declaration
        size_t offset = 0;
        decl->addElement(0,offset,Ogre::VET_FLOAT2,Ogre::VES_POSITION);
        offset += VertexElement::getTypeSize( VET_FLOAT2 );
        decl->addElement(0,offset,Ogre::VET_FLOAT2,Ogre::VES_TEXTURE_COORDINATES,0);
        offset += VertexElement::getTypeSize( VET_FLOAT2 );
        decl->addElement(0,offset,Ogre::VET_COLOUR,Ogre::VES_DIFFUSE);
    }
    //-----------------------------------------------------------------------------------
    ImGUIRenderable::~ImGUIRenderable()
    {
        OGRE_DELETE mRenderOp.vertexData;
        OGRE_DELETE mRenderOp.indexData;
    }
    //-----------------------------------------------------------------------------------
    void ImGUIRenderable::updateVertexData(const ImVector<ImDrawVert>& vtxBuf, const ImVector<ImDrawIdx>& idxBuf)
    {
        Ogre::VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

        if (bind->getBindings().empty() || bind->getBuffer(0)->getNumVertices() != vtxBuf.size())
        {
	        bind->setBinding(0, HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(ImDrawVert), vtxBuf.size(), HardwareBuffer::HBU_WRITE_ONLY));
        }
        if (!mRenderOp.indexData->indexBuffer || mRenderOp.indexData->indexBuffer->getNumIndexes() != idxBuf.size())
        {
	        mRenderOp.indexData->indexBuffer =
		        HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT, idxBuf.size(), HardwareBuffer::HBU_WRITE_ONLY);
        }
      
        // Copy all vertices
        bind->getBuffer(0)->writeData(0, vtxBuf.size_in_bytes(), vtxBuf.Data, true);
        mRenderOp.indexData->indexBuffer->writeData(0, idxBuf.size_in_bytes(), idxBuf.Data, true);

        mRenderOp.vertexData->vertexStart = 0;
        mRenderOp.vertexData->vertexCount = vtxBuf.size();
    }
    //-----------------------------------------------------------------------------------
    const LightList& ImGUIRenderable::getLights(void) const
    {
        static const LightList l;
        return l;
    }
}
