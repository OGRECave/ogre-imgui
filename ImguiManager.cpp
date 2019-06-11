#include <imgui.h>
#ifdef USE_FREETYPE
#include <imgui_freetype.h>
#endif

#include "ImguiManager.h"

#include <OgreMaterialManager.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>
#include <OgreSubMesh.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <OgreViewport.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreUnifiedHighLevelGpuProgram.h>
#include <OgreRoot.h>
#include <OgreTechnique.h>
#include <OgreTextureUnitState.h>
#include <OgreViewport.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTarget.h>
#include "OgreHardwareBufferManager.h"

#if OGRE_VERSION >= 0x10C00
#include "OgreComponents.h"
#endif

#ifdef OGRE_BUILD_COMPONENT_OVERLAY
#include <OgreFontManager.h>
#endif

#ifdef HAVE_OGRE_BITES
#include <OgreInput.h>
#endif

using namespace Ogre;

#ifdef HAVE_OGRE_BITES
// map sdl2 mouse buttons to imgui
static int sdl2imgui(int b)
{
    switch(b) {
    case 2:
        return 2;
    case 3:
        return 1;
    default:
        return b - 1;
    }
}

// SDL2 keycode to scancode
static int kc2sc(int kc)
{
    return kc & ~(1 << 30);
}

struct ImguiInputListener : public OgreBites::InputListener
{
    ImguiInputListener()
    {
        using namespace OgreBites;
        ImGuiIO& io = ImGui::GetIO();
        // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
        io.KeyMap[ImGuiKey_Tab] = '\t';
        io.KeyMap[ImGuiKey_LeftArrow] = kc2sc(SDLK_LEFT);
        io.KeyMap[ImGuiKey_RightArrow] = kc2sc(SDLK_RIGHT);
        io.KeyMap[ImGuiKey_UpArrow] = kc2sc(SDLK_UP);
        io.KeyMap[ImGuiKey_DownArrow] = kc2sc(SDLK_DOWN);
        io.KeyMap[ImGuiKey_PageUp] = kc2sc(SDLK_PAGEUP);
        io.KeyMap[ImGuiKey_PageDown] = kc2sc(SDLK_PAGEDOWN);
        io.KeyMap[ImGuiKey_Home] = -1;
        io.KeyMap[ImGuiKey_End] = -1;
        io.KeyMap[ImGuiKey_Delete] = -1;
        io.KeyMap[ImGuiKey_Backspace] = '\b';
        io.KeyMap[ImGuiKey_Enter] = '\r';
        io.KeyMap[ImGuiKey_Escape] = '\033';
        io.KeyMap[ImGuiKey_Space] = ' ';
        io.KeyMap[ImGuiKey_A] = 'a';
        io.KeyMap[ImGuiKey_C] = 'c';
        io.KeyMap[ImGuiKey_V] = 'v';
        io.KeyMap[ImGuiKey_X] = 'x';
        io.KeyMap[ImGuiKey_Y] = 'y';
        io.KeyMap[ImGuiKey_Z] = 'z';
    }

    bool mouseWheelRolled(const OgreBites::MouseWheelEvent& arg)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheel = Ogre::Math::Sign(arg.y);
        return io.WantCaptureMouse;
    }

    bool mouseMoved( const OgreBites::MouseMotionEvent &arg )
    {

        ImGuiIO& io = ImGui::GetIO();

        io.MousePos.x = arg.x;
        io.MousePos.y = arg.y;

        return io.WantCaptureMouse;
    }

    bool mousePressed( const OgreBites::MouseButtonEvent &arg)
    {
        ImGuiIO& io = ImGui::GetIO();
        int b = sdl2imgui(arg.button);
        if(b<5)
        {
            io.MouseDown[b] = true;
        }
        return io.WantCaptureMouse;
    }
    bool mouseReleased( const OgreBites::MouseButtonEvent &arg)
    {
        ImGuiIO& io = ImGui::GetIO();
        int b = sdl2imgui(arg.button);
        if(b<5)
        {
            io.MouseDown[b] = false;
        }
        return io.WantCaptureMouse;
    }
    bool keyPressed( const OgreBites::KeyboardEvent &arg )
    {
        using namespace OgreBites;

        ImGuiIO& io = ImGui::GetIO();
        
        // ignore
        if(arg.keysym.sym == SDLK_LSHIFT) return io.WantCaptureKeyboard;

        io.KeyCtrl = arg.keysym.mod & KMOD_CTRL;
        io.KeyShift = arg.keysym.mod & SDLK_LSHIFT;

        int key = kc2sc(arg.keysym.sym);

        if(key > 0 && key < 512)
        {
            io.KeysDown[key] = true;
            
            uint16_t sym = arg.keysym.sym;
            if (io.KeyShift) sym -= 32;
            io.AddInputCharacter(sym);
        }

        return io.WantCaptureKeyboard;
    }
    bool keyReleased( const OgreBites::KeyboardEvent &arg )
    {
        using namespace OgreBites;

        int key = kc2sc(arg.keysym.sym);
        if(key < 0 || key >= 512)
            return true;

        ImGuiIO& io = ImGui::GetIO();

        io.KeyCtrl = arg.keysym.mod & KMOD_CTRL;
        io.KeyShift = arg.keysym.mod & SDLK_LSHIFT;

        io.KeysDown[key] = false;
        return io.WantCaptureKeyboard;
    }
};
#endif

template<> ImguiManager* Singleton<ImguiManager>::msSingleton = 0;

void ImguiManager::createSingleton()
{
    if(!msSingleton)
    {
        msSingleton = new ImguiManager();
    }
}
ImguiManager* ImguiManager::getSingletonPtr(void)
{
    createSingleton();
    return msSingleton;
}
ImguiManager& ImguiManager::getSingleton(void)
{  
    createSingleton();
    return ( *msSingleton );  
}

ImguiManager::ImguiManager()
:mSceneMgr(0)
{
    ImGui::CreateContext();
}
ImguiManager::~ImguiManager()
{
    ImGui::DestroyContext();
    mSceneMgr->removeRenderQueueListener(this);
}

void ImguiManager::init(Ogre::SceneManager * mgr)
{
    mSceneMgr  = mgr;

    mSceneMgr->addRenderQueueListener(this);

    createFontTexture();
    createMaterial();
}

OgreBites::InputListener* ImguiManager::getInputListener()
{
#ifdef HAVE_OGRE_BITES
    static ImguiInputListener listener;
    return &listener;
#else
    OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "re-build with OgreBites component");
    return NULL;
#endif
}

//-----------------------------------------------------------------------------------
void ImguiManager::renderQueueEnded(uint8 queueGroupId, const String& invocation,bool& repeatThisInvocation)
{
    if((queueGroupId != Ogre::RENDER_QUEUE_OVERLAY) || (invocation == "SHADOWS"))
    {
        return;
    }
    Ogre::RenderSystem* renderSys = Ogre::Root::getSingletonPtr()->getRenderSystem();
    Ogre::Viewport* vp = renderSys->_getViewport();
    
    if (!vp || (!vp->getTarget()->isPrimary()) || mFrameEnded)
    {
        return;
    }
    
    mFrameEnded = true;
    ImGuiIO& io = ImGui::GetIO();
    
    // Construct projection matrix, taking texel offset corrections in account (important for DirectX9)
    // See also:
    //     - OGRE-API specific hint: http://www.ogre3d.org/forums/viewtopic.php?f=5&p=536881#p536881
    //     - IMGUI Dx9 demo solution: https://github.com/ocornut/imgui/blob/master/examples/directx9_example/imgui_impl_dx9.cpp#L127-L138
    const float texelOffsetX = renderSys->getHorizontalTexelOffset();
    const float texelOffsetY = renderSys->getVerticalTexelOffset();
    const float L = texelOffsetX;
    const float R = io.DisplaySize.x + texelOffsetX;
    const float T = texelOffsetY;
    const float B = io.DisplaySize.y + texelOffsetY;
    
    mRenderable.mXform = Matrix4(   2.0f/(R-L),    0.0f,         0.0f,       (L+R)/(L-R),
                                    0.0f,         -2.0f/(B-T),   0.0f,       (T+B)/(B-T),
                                    0.0f,          0.0f,        -1.0f,       0.0f,
                                    0.0f,          0.0f,         0.0f,       1.0f);
    
    // Instruct ImGui to Render() and process the resulting CmdList-s
    /// Adopted from https://bitbucket.org/ChaosCreator/imgui-ogre2.1-binding
    /// ... Commentary on OGRE forums: http://www.ogre3d.org/forums/viewtopic.php?f=5&t=89081#p531059
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    int vpWidth  = vp->getActualWidth();
    int vpHeight = vp->getActualHeight();
    for (int i = 0; i < draw_data->CmdListsCount; ++i)
    {
        const ImDrawList* draw_list = draw_data->CmdLists[i];
        mRenderable.updateVertexData(draw_list->VtxBuffer, draw_list->IdxBuffer);

        unsigned int startIdx = 0;

        for (int j = 0; j < draw_list->CmdBuffer.Size; ++j)
        {
            // Create a renderable and fill it's buffers
            const ImDrawCmd *drawCmd = &draw_list->CmdBuffer[j];

            // Set scissoring
            int scLeft   = static_cast<int>(drawCmd->ClipRect.x); // Obtain bounds
            int scTop    = static_cast<int>(drawCmd->ClipRect.y);
            int scRight  = static_cast<int>(drawCmd->ClipRect.z);
            int scBottom = static_cast<int>(drawCmd->ClipRect.w);

            scLeft   = scLeft   < 0 ? 0 : (scLeft  > vpWidth ? vpWidth : scLeft); // Clamp bounds to viewport dimensions
            scRight  = scRight  < 0 ? 0 : (scRight > vpWidth ? vpWidth : scRight);
            scTop    = scTop    < 0 ? 0 : (scTop    > vpHeight ? vpHeight : scTop);
            scBottom = scBottom < 0 ? 0 : (scBottom > vpHeight ? vpHeight : scBottom);

            if (mRenderable.mMaterial->getSupportedTechniques().empty())
            {
                mRenderable.mMaterial->load(); // Support for adding lights run time
            }
            Pass * pass = mRenderable.mMaterial->getBestTechnique()->getPass(0);
            TextureUnitState * st = pass->getTextureUnitState(0);
            if (drawCmd->TextureId != 0)
            {
                Ogre::ResourceHandle handle = (Ogre::ResourceHandle)drawCmd->TextureId;
                Ogre::TexturePtr tex = Ogre::static_pointer_cast<Ogre::Texture>(
                    Ogre::TextureManager::getSingleton().getByHandle(handle));
                if (tex)
                {
                    st->setTexture(tex);
                    st->setTextureFiltering(Ogre::TFO_TRILINEAR);
                }
            }
            else
            {
                st->setTexture(mFontTex);
                st->setTextureFiltering(Ogre::TFO_NONE);
            }
            renderSys->setScissorTest(true, scLeft, scTop, scRight, scBottom);

            // Render!
            mRenderable.mRenderOp.indexData->indexStart = startIdx;
            mRenderable.mRenderOp.indexData->indexCount = drawCmd->ElemCount;
            mSceneMgr->_injectRenderWithPass(pass,
                                             &mRenderable, false);

            // Update counts
            startIdx += drawCmd->ElemCount;
        }
    }
    renderSys->setScissorTest(false);
}
//-----------------------------------------------------------------------------------
void ImguiManager::createMaterial()
{
    mRenderable.mMaterial = dynamic_pointer_cast<Material>(
        MaterialManager::getSingleton()
            .createOrRetrieve("imgui/material", ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME)
            .first);
    Pass* mPass = mRenderable.mMaterial->getTechnique(0)->getPass(0);
    mPass->setCullingMode(CULL_NONE);
    mPass->setDepthFunction(Ogre::CMPF_ALWAYS_PASS);
    mPass->setLightingEnabled(false);
    mPass->setVertexColourTracking(TVC_DIFFUSE);
    mPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    mPass->setSeparateSceneBlendingOperation(Ogre::SBO_ADD,Ogre::SBO_ADD);
    mPass->setSeparateSceneBlending(Ogre::SBF_SOURCE_ALPHA,Ogre::SBF_ONE_MINUS_SOURCE_ALPHA,Ogre::SBF_ONE_MINUS_SOURCE_ALPHA,Ogre::SBF_ZERO);
        
    TextureUnitState*  mTexUnit =  mPass->createTextureUnitState();
    mTexUnit->setTexture(mFontTex);
    mTexUnit->setTextureFiltering(Ogre::TFO_NONE);

    mRenderable.mMaterial->load();
}

ImFont* ImguiManager::addFont(const String& name, const String& group)
{
#ifdef OGRE_BUILD_COMPONENT_OVERLAY
    FontPtr font = FontManager::getSingleton().getByName(name, group);
    OgreAssert(font, "font does not exist");
    OgreAssert(font->getType() == FT_TRUETYPE, "font must be of FT_TRUETYPE");
    DataStreamPtr dataStreamPtr =
        ResourceGroupManager::getSingleton().openResource(font->getSource(), font->getGroup());
    MemoryDataStream ttfchunk(dataStreamPtr, false); // transfer ownership to imgui

    // convert codepoint ranges for imgui
    CodePointRange cprange;
    for(const auto& r : font->getCodePointRangeList())
    {
        cprange.push_back(r.first);
        cprange.push_back(r.second);
    }

    ImGuiIO& io = ImGui::GetIO();
    const ImWchar* cprangePtr = io.Fonts->GetGlyphRangesDefault();
    if(!cprange.empty())
    {
        cprange.push_back(0); // terminate
        mCodePointRanges.push_back(cprange);
        // ptr must persist until createFontTexture
        cprangePtr = mCodePointRanges.back().data();
    }

    ImFontConfig cfg;
    strncpy(cfg.Name, name.c_str(), 40);
    return io.Fonts->AddFontFromMemoryTTF(ttfchunk.getPtr(), ttfchunk.size(),
                                          font->getTrueTypeSize(), &cfg, cprangePtr);
#else
    OGRE_EXCEPT(Exception::ERR_INVALID_CALL, "Ogre Overlay Component required");
    return NULL;
#endif
}

void ImguiManager::createFontTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    if(io.Fonts->Fonts.empty())
        io.Fonts->AddFontDefault();
#ifdef USE_FREETYPE
    ImGuiFreeType::BuildFontAtlas(io.Fonts, 0);
#endif

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    mFontTex = TextureManager::getSingleton().createManual(
        "ImguiFontTex", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
        width, height, 1, 1, PF_BYTE_RGBA);

    mFontTex->getBuffer()->blitFromMemory(PixelBox(Box(0, 0, width, height), PF_BYTE_RGBA, pixels));

    mCodePointRanges.clear(); 
}
void ImguiManager::newFrame(float deltaTime,const Ogre::Rect & windowRect)
{
    mFrameEnded=false;
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = std::max(deltaTime, 1e-4f); // see https://github.com/ocornut/imgui/commit/3c07ec6a6126fb6b98523a9685d1f0f78ca3c40c

     // Read keyboard modifiers inputs
    io.KeyAlt = false;// mKeyInput->isKeyDown(OIS::KC_LMENU);
    io.KeySuper = false;

    // Setup display size (every frame to accommodate for window resizing)
     io.DisplaySize = ImVec2((float)(windowRect.right - windowRect.left), (float)(windowRect.bottom - windowRect.top));


    // Start the frame
    ImGui::NewFrame();
}

ImguiManager::ImGUIRenderable::ImGUIRenderable()
{
    initImGUIRenderable();

    //By default we want ImGUIRenderables to still work in wireframe mode
    setPolygonModeOverrideable( false );

}
//-----------------------------------------------------------------------------------
void ImguiManager::ImGUIRenderable::initImGUIRenderable(void)
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
ImguiManager::ImGUIRenderable::~ImGUIRenderable()
{
    OGRE_DELETE mRenderOp.vertexData;
    OGRE_DELETE mRenderOp.indexData;
}
//-----------------------------------------------------------------------------------
void ImguiManager::ImGUIRenderable::updateVertexData(const ImVector<ImDrawVert>& vtxBuf, const ImVector<ImDrawIdx>& idxBuf)
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
const LightList& ImguiManager::ImGUIRenderable::getLights(void) const
{
    static const LightList l;
    return l;
}
