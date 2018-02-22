#include <imgui.h>
#include "ImguiRenderable.h"
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

using namespace Ogre;


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
,mLastRenderedFrame(-1)
{
    ImGui::CreateContext();
}
ImguiManager::~ImguiManager()
{
    ImGui::DestroyContext();
    mSceneMgr->removeRenderQueueListener(this);
}

// SDL2 keycode to scancode
static int kc2sc(int kc)
{
    return kc & ~(1 << 30);
}

void ImguiManager::init(Ogre::SceneManager * mgr)
{
    using namespace OgreBites;

    mSceneMgr  = mgr;

    mSceneMgr->addRenderQueueListener(this);
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

    createFontTexture();
    createMaterial();
}

bool ImguiManager::mouseWheelRolled(const OgreBites::MouseWheelEvent& arg)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = Ogre::Math::Sign(arg.y);
    return true;
}

bool ImguiManager::mouseMoved( const OgreBites::MouseMotionEvent &arg )
{

    ImGuiIO& io = ImGui::GetIO();

    io.MousePos.x = arg.x;
    io.MousePos.y = arg.y;

    return true;
}

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

bool ImguiManager::mousePressed( const OgreBites::MouseButtonEvent &arg)
{
    ImGuiIO& io = ImGui::GetIO();
    int b = sdl2imgui(arg.button);
    if(b<5)
    {
        io.MouseDown[b] = true;
    }
    return true;
}
bool ImguiManager::mouseReleased( const OgreBites::MouseButtonEvent &arg)
{
    ImGuiIO& io = ImGui::GetIO();
    int b = sdl2imgui(arg.button);
    if(b<5)
    {
        io.MouseDown[b] = false;
    }
    return true;
}
bool ImguiManager::keyPressed( const OgreBites::KeyboardEvent &arg )
{
    using namespace OgreBites;

    ImGuiIO& io = ImGui::GetIO();
    
    io.KeyCtrl = arg.keysym.mod & KMOD_CTRL;
    io.KeyShift = arg.keysym.sym == SDLK_LSHIFT;

    int key = kc2sc(arg.keysym.sym);

    if(key > 0 && key < 512)
    {
        io.KeysDown[key] = true;
        io.AddInputCharacter((unsigned short)arg.keysym.sym);
    }

    return true;
}
bool ImguiManager::keyReleased( const OgreBites::KeyboardEvent &arg )
{
    int key = kc2sc(arg.keysym.sym);
    if(key < 0 || key >= 512)
        return true;

    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[key] = false;
    return true;
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
    
    if ((vp == nullptr) || (!vp->getTarget()->isPrimary()) || mFrameEnded)
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
    
    Ogre::Matrix4 projMatrix(       2.0f/(R-L),    0.0f,         0.0f,       (L+R)/(L-R),
                                    0.0f,         -2.0f/(B-T),   0.0f,       (T+B)/(B-T),
                                    0.0f,          0.0f,        -1.0f,       0.0f,
                                    0.0f,          0.0f,         0.0f,       1.0f);
    
    mPass->getVertexProgramParameters()->setNamedConstant("ProjectionMatrix", projMatrix);

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
        unsigned int startIdx = 0;

        for (int j = 0; j < draw_list->CmdBuffer.Size; ++j)
        {
            // Create a renderable and fill it's buffers
            ImGUIRenderable renderable;
            const ImDrawCmd *drawCmd = &draw_list->CmdBuffer[j];
            renderable.updateVertexData(draw_list->VtxBuffer.Data, &draw_list->IdxBuffer.Data[startIdx], draw_list->VtxBuffer.Size, drawCmd->ElemCount);

            // Set scissoring
            int scLeft   = static_cast<int>(drawCmd->ClipRect.x); // Obtain bounds
            int scTop    = static_cast<int>(drawCmd->ClipRect.y);
            int scRight  = static_cast<int>(drawCmd->ClipRect.z);
            int scBottom = static_cast<int>(drawCmd->ClipRect.w);

            scLeft   = scLeft   < 0 ? 0 : (scLeft  > vpWidth ? vpWidth : scLeft); // Clamp bounds to viewport dimensions
            scRight  = scRight  < 0 ? 0 : (scRight > vpWidth ? vpWidth : scRight);
            scTop    = scTop    < 0 ? 0 : (scTop    > vpHeight ? vpHeight : scTop);
            scBottom = scBottom < 0 ? 0 : (scBottom > vpHeight ? vpHeight : scBottom);


            if(drawCmd->TextureId != 0 )
            {
                Ogre::ResourceHandle handle = (Ogre::ResourceHandle)drawCmd->TextureId;
                Ogre::TexturePtr tex = Ogre::static_pointer_cast<Ogre::Texture>(
                    Ogre::TextureManager::getSingleton().getByHandle(handle));
                if (tex)
                {
                    mTexUnit->setTexture(tex);
                    mTexUnit->setTextureFiltering(Ogre::TFO_TRILINEAR);
                }
            }
            else
            {
                mTexUnit->setTexture(mFontTex);
                mTexUnit->setTextureFiltering(Ogre::TFO_NONE);
            }
            renderSys->setScissorTest(true, scLeft, scTop, scRight, scBottom);

            // Render!
            mSceneMgr->_injectRenderWithPass(mPass, &renderable, 0, false, NULL);

            // Update counts
            startIdx += drawCmd->ElemCount;
        }
    }
    renderSys->setScissorTest(false);
}
//-----------------------------------------------------------------------------------
void ImguiManager::createMaterial()
{
    
    static const char* vertexShaderSrcD3D11 =
    {
    "cbuffer vertexBuffer : register(b0) \n"
    "{\n"
    "float4x4 ProjectionMatrix; \n"
    "};\n"
    "struct VS_INPUT\n"
    "{\n"
    "float2 pos : POSITION;\n"
    "float4 col : COLOR0;\n"
    "float2 uv  : TEXCOORD0;\n"
    "};\n"
    "struct PS_INPUT\n"
    "{\n"
    "float4 pos : SV_POSITION;\n"
    "float4 col : COLOR0;\n"
    "float2 uv  : TEXCOORD0;\n"
    "};\n"
    "PS_INPUT main(VS_INPUT input)\n"
    "{\n"
    "PS_INPUT output;\n"
    "output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\n"
    "output.col = input.col;\n"
    "output.uv  = input.uv;\n"
    "return output;\n"
    "}"
    };
    

 
    static const char* pixelShaderSrcD3D11 =
    {
    "struct PS_INPUT\n"
    "{\n"
    "float4 pos : SV_POSITION;\n"
    "float4 col : COLOR0;\n"
    "float2 uv  : TEXCOORD0;\n"
    "};\n"
    "sampler sampler0;\n"
    "Texture2D texture0;\n"
    "\n"
    "float4 main(PS_INPUT input) : SV_Target\n"
    "{\n"
    "float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \n"
    "return out_col; \n"
    "}"
    };
	static const char* vertexShaderSrcD3D9 =
    {
    "uniform float4x4 ProjectionMatrix; \n"
    "struct VS_INPUT\n"
    "{\n"
    "float2 pos : POSITION;\n"
    "float4 col : COLOR0;\n"
    "float2 uv  : TEXCOORD0;\n"
    "};\n"
    "struct PS_INPUT\n"
    "{\n"
    "float4 pos : POSITION;\n"
    "float4 col : COLOR0;\n"
    "float2 uv  : TEXCOORD0;\n"
    "};\n"
    "PS_INPUT main(VS_INPUT input)\n"
    "{\n"
    "PS_INPUT output;\n"
    "output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\n"
    "output.col = input.col;\n"
    "output.uv  = input.uv;\n"
    "return output;\n"
    "}"
    };
   
    static const char* pixelShaderSrcSrcD3D9 =
     {
    "struct PS_INPUT\n"
    "{\n"
    "float4 pos : SV_POSITION;\n"
    "float4 col : COLOR0;\n"
    "float2 uv  : TEXCOORD0;\n"
    "};\n"
    "sampler2D sampler0;\n"
    "\n"
    "float4 main(PS_INPUT input) : SV_Target\n"
    "{\n"
    "float4 out_col = input.col.bgra * tex2D(sampler0, input.uv); \n"
    "return out_col; \n"
    "}"
    };

    static const char* vertexShaderSrcGLSL =
    {
    "#version 150\n"
    "uniform mat4 ProjectionMatrix; \n"
    "in vec2 vertex;\n"
    "in vec2 uv0;\n"
    "in vec4 colour;\n"
    "out vec2 Texcoord;\n"
    "out vec4 col;\n"
    "void main()\n"
    "{\n"
    "gl_Position = ProjectionMatrix* vec4(vertex.xy, 0.f, 1.f);\n"
    "Texcoord  = uv0;\n"
    "col = colour;\n"
    "}"
    };
    
    static const char* pixelShaderSrcGLSL =
    {
    "#version 150\n"
    "in vec2 Texcoord;\n"
    "in vec4 col;\n"
    "uniform sampler2D sampler0;\n"
    "out vec4 out_col;\n"
    "void main()\n"
    "{\n"
    "out_col = col * texture(sampler0, Texcoord); \n"
    "}"
    };
 
        //create the default shadows material
    Ogre::HighLevelGpuProgramManager& mgr = Ogre::HighLevelGpuProgramManager::getSingleton();

    Ogre::HighLevelGpuProgramPtr vertexShaderUnified = mgr.getByName("imgui/VP");
    Ogre::HighLevelGpuProgramPtr pixelShaderUnified = mgr.getByName("imgui/FP");
    
    Ogre::HighLevelGpuProgramPtr vertexShaderD3D11 = mgr.getByName("imgui/VP/D3D11");
    Ogre::HighLevelGpuProgramPtr pixelShaderD3D11 = mgr.getByName("imgui/FP/D3D11");

	Ogre::HighLevelGpuProgramPtr vertexShaderD3D9 = mgr.getByName("imgui/VP/D3D9");
    Ogre::HighLevelGpuProgramPtr pixelShaderD3D9 = mgr.getByName("imgui/FP/D3D9");

    Ogre::HighLevelGpuProgramPtr vertexShaderGL = mgr.getByName("imgui/VP/GL150");
    Ogre::HighLevelGpuProgramPtr pixelShaderGL = mgr.getByName("imgui/FP/GL150");
    
    if(!vertexShaderUnified)
    {
        vertexShaderUnified = mgr.createProgram("imgui/VP",Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,"unified",GPT_VERTEX_PROGRAM);
    }
    if(!pixelShaderUnified)
        {
        pixelShaderUnified = mgr.createProgram("imgui/FP",Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,"unified",GPT_FRAGMENT_PROGRAM);
    }

    UnifiedHighLevelGpuProgram* vertexShaderPtr = static_cast<UnifiedHighLevelGpuProgram*>(vertexShaderUnified.get());
    UnifiedHighLevelGpuProgram* pixelShaderPtr = static_cast<UnifiedHighLevelGpuProgram*>(pixelShaderUnified.get());

    if (!vertexShaderD3D11)
    {
        vertexShaderD3D11 = mgr.createProgram("imgui/VP/D3D11", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                "hlsl", Ogre::GPT_VERTEX_PROGRAM);
        vertexShaderD3D11->setParameter("target", "vs_4_0");
        vertexShaderD3D11->setParameter("entry_point", "main");
        vertexShaderD3D11->setSource(vertexShaderSrcD3D11);
        vertexShaderD3D11->load();

        vertexShaderPtr->addDelegateProgram(vertexShaderD3D11->getName());
        }
    if (!pixelShaderD3D11)
        {
        pixelShaderD3D11 = mgr.createProgram("imgui/FP/D3D11", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                "hlsl", Ogre::GPT_FRAGMENT_PROGRAM);
        pixelShaderD3D11->setParameter("target", "ps_4_0");
        pixelShaderD3D11->setParameter("entry_point", "main");
        pixelShaderD3D11->setSource(pixelShaderSrcD3D11);
        pixelShaderD3D11->load();

        pixelShaderPtr->addDelegateProgram(pixelShaderD3D11->getName());
        }
    if (!vertexShaderD3D9)
    {
        vertexShaderD3D9 = mgr.createProgram("imgui/VP/D3D9", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            "hlsl", Ogre::GPT_VERTEX_PROGRAM);
        vertexShaderD3D9->setParameter("target", "vs_2_0");
        vertexShaderD3D9->setParameter("entry_point", "main");
        vertexShaderD3D9->setSource(vertexShaderSrcD3D9);
        vertexShaderD3D9->load();
    
        vertexShaderPtr->addDelegateProgram(vertexShaderD3D9->getName());
    }
    if (!pixelShaderD3D9)
    {
        pixelShaderD3D9 = mgr.createProgram("imgui/FP/D3D9", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            "hlsl", Ogre::GPT_FRAGMENT_PROGRAM);
        pixelShaderD3D9->setParameter("target", "ps_2_0");
        pixelShaderD3D9->setParameter("entry_point", "main");
        pixelShaderD3D9->setSource(pixelShaderSrcSrcD3D9);
        pixelShaderD3D9->load();
    
        pixelShaderPtr->addDelegateProgram(pixelShaderD3D9->getName());
    }

    if (!vertexShaderGL)
    {
        vertexShaderGL = mgr.createProgram("imgui/VP/GL150", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                "glsl", Ogre::GPT_VERTEX_PROGRAM);
        vertexShaderGL->setSource(vertexShaderSrcGLSL);
        vertexShaderGL->load();
        vertexShaderPtr->addDelegateProgram(vertexShaderGL->getName());
        }
    if (!pixelShaderGL)
        {
        pixelShaderGL = mgr.createProgram("imgui/FP/GL150", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
        pixelShaderGL->setSource(pixelShaderSrcGLSL);
        pixelShaderGL->load();
        pixelShaderGL->setParameter("sampler0","int 0");

        pixelShaderPtr->addDelegateProgram(pixelShaderGL->getName());
        }
   
    Ogre::MaterialPtr imguiMaterial = Ogre::MaterialManager::getSingleton().create("imgui/material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mPass = imguiMaterial->getTechnique(0)->getPass(0);
    mPass->setFragmentProgram("imgui/FP");
    mPass->setVertexProgram("imgui/VP");
    mPass->setCullingMode(CULL_NONE);
    mPass->setDepthFunction(Ogre::CMPF_ALWAYS_PASS);
    mPass->setLightingEnabled(false);
    mPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
    mPass->setSeparateSceneBlendingOperation(Ogre::SBO_ADD,Ogre::SBO_ADD);
    mPass->setSeparateSceneBlending(Ogre::SBF_SOURCE_ALPHA,Ogre::SBF_ONE_MINUS_SOURCE_ALPHA,Ogre::SBF_ONE_MINUS_SOURCE_ALPHA,Ogre::SBF_ZERO);
        
    mTexUnit =  mPass->createTextureUnitState();
    mTexUnit->setTexture(mFontTex);
    mTexUnit->setTextureFiltering(Ogre::TFO_NONE);
}

void ImguiManager::createFontTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    mFontTex = TextureManager::getSingleton().createManual("ImguiFontTex",Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,TEX_TYPE_2D,width,height,1,1,PF_BYTE_RGBA);

    const PixelBox & lockBox = mFontTex->getBuffer()->lock(Box(0, 0, width, height), HardwareBuffer::HBL_DISCARD);
	size_t texDepth = PixelUtil::getNumElemBytes(lockBox.format);

    memcpy(lockBox.data,pixels, width*height*texDepth);
	mFontTex->getBuffer()->unlock();
    
    
}
void ImguiManager::newFrame(float deltaTime,const Ogre::Rect & windowRect)
{
    mFrameEnded=false;
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;

     // Read keyboard modifiers inputs
    io.KeyAlt = false;// mKeyInput->isKeyDown(OIS::KC_LMENU);
    io.KeySuper = false;

    // Setup display size (every frame to accommodate for window resizing)
     io.DisplaySize = ImVec2((float)(windowRect.right - windowRect.left), (float)(windowRect.bottom - windowRect.top));


    // Start the frame
    ImGui::NewFrame();
}
