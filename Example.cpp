#include <Ogre.h>
#include <OgreApplicationContext.h>

#include "ImguiManager.h"

class ImguiExample : public OgreBites::ApplicationContext, public OgreBites::InputListener
{
public:
    ImguiExample() : OgreBites::ApplicationContext("OgreImguiExample")
    {
    }

    bool frameStarted(const Ogre::FrameEvent& evt)
    {
        OgreBites::ApplicationContext::frameStarted(evt);

        Ogre::ImguiManager::getSingleton().newFrame(
            evt.timeSinceLastFrame,
            Ogre::Rect(0, 0, getRenderWindow()->getWidth(), getRenderWindow()->getHeight()));

        ImGui::ShowDemoWindow();

        return true;
    }

    void setup()
    {
        OgreBites::ApplicationContext::setup();
        addInputListener(this);

        Ogre::ImguiManager::createSingleton();
        addInputListener(Ogre::ImguiManager::getSingletonPtr());

        // get a pointer to the already created root
        Ogre::Root* root = getRoot();
        Ogre::SceneManager* scnMgr = root->createSceneManager();
        Ogre::ImguiManager::getSingleton().init(scnMgr);

        // register our scene with the RTSS
        Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
        shadergen->addSceneManager(scnMgr);


        Ogre::Light* light = scnMgr->createLight("MainLight");
        Ogre::SceneNode* lightNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->setPosition(0, 10, 15);
        lightNode->attachObject(light);


        Ogre::SceneNode* camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
        camNode->setPosition(0, 0, 15);
        camNode->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);

        Ogre::Camera* cam = scnMgr->createCamera("myCam");
        cam->setNearClipDistance(5); // specific to this sample
        cam->setAutoAspectRatio(true);
        camNode->attachObject(cam);
        getRenderWindow()->addViewport(cam);

        Ogre::Entity* ent = scnMgr->createEntity("Sinbad.mesh");
        Ogre::SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode();
        node->attachObject(ent);
    }

    bool keyPressed(const OgreBites::KeyboardEvent& evt)
    {
        if (evt.keysym.sym == 27)
        {
            getRoot()->queueEndRendering();
        }
        return true;
    }
};


int main(int argc, char *argv[])
{
    ImguiExample app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();

    return 0;
}
