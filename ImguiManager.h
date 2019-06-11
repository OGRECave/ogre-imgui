#pragma once

#include <imgui.h>
#include <OgrePrerequisites.h>

#include <OgreRenderQueueListener.h>
#include <OgreSingleton.h>
#include <OgreTexture.h>
#include <OgreResourceGroupManager.h>
#include <OgreRenderable.h>
#include <OgreRenderOperation.h>

namespace OgreBites
{
    struct InputListener;
}

namespace Ogre
{
    class SceneManager;

    class ImguiManager : public RenderQueueListener, public Singleton<ImguiManager>
    {
    public:
        static void createSingleton();

        ImguiManager();
        ~ImguiManager();

        /// add font from ogre .fontdef file
        /// must be called before init()
        ImFont* addFont(const String& name, const String& group OGRE_RESOURCE_GROUP_INIT);

        virtual void init(Ogre::SceneManager* mgr);

        virtual void newFrame(float deltaTime,const Ogre::Rect & windowRect);

        //inherited from RenderQueueListener
        virtual void renderQueueEnded(uint8 queueGroupId, const String& invocation,bool& repeatThisInvocation);

        OgreBites::InputListener* getInputListener();

        static ImguiManager& getSingleton(void);
        static ImguiManager* getSingletonPtr(void);

    protected:

        class ImGUIRenderable : public Renderable
        {
        protected:
            void initImGUIRenderable(void);

        public:
            ImGUIRenderable();
            ~ImGUIRenderable();

            void updateVertexData(const ImVector<ImDrawVert>& vtxBuf, const ImVector<ImDrawIdx>& idxBuf);
            Real getSquaredViewDepth(const Camera* cam) const   { (void)cam; return 0; }

            virtual const MaterialPtr& getMaterial(void) const { return mMaterial; }
            virtual void getWorldTransforms( Matrix4* xform ) const { *xform = mXform; }
            virtual void getRenderOperation( RenderOperation& op ) { op = mRenderOp; }
            virtual const LightList& getLights(void) const;

            MaterialPtr              mMaterial;
            Matrix4                  mXform;
            RenderOperation          mRenderOp;

        };

        void createFontTexture();
        void createMaterial();

        SceneManager*				mSceneMgr;

        ImGUIRenderable             mRenderable;
        TexturePtr                  mFontTex;

        bool                        mFrameEnded;

        typedef std::vector<ImWchar> CodePointRange;
        std::vector<CodePointRange> mCodePointRanges;
    };
}
