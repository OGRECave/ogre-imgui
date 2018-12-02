import Ogre
import OgreRTShader
import OgreBites
import OgreImgui


class ImguiExample(OgreBites.ApplicationContext, OgreBites.InputListener):

    def __init__(self):
        OgreBites.ApplicationContext.__init__(self, "OgreImguiExample", False)
        OgreBites.InputListener.__init__(self)

    def keyPressed(self, evt):
        if evt.keysym.sym == OgreBites.SDLK_ESCAPE:
            self.getRoot().queueEndRendering()

        return True

    def frameStarted(self, evt):
        OgreBites.ApplicationContext.frameStarted(self, evt)

        OgreImgui.ImguiManager.getSingleton().newFrame(
            evt.timeSinceLastFrame,
            Ogre.Rect(0, 0, self.getRenderWindow().getWidth(), self.getRenderWindow().getHeight()))

        OgreImgui.ShowDemoWindow()

        return True

    def setup(self):
        OgreBites.ApplicationContext.setup(self)
        self.addInputListener(self)

        OgreImgui.ImguiManager.createSingleton()

        root = self.getRoot()
        scn_mgr = root.createSceneManager()
        OgreImgui.ImguiManager.getSingleton().init(scn_mgr)
        self.addInputListener(OgreImgui.ImguiManager.getSingleton())

        shadergen = OgreRTShader.ShaderGenerator.getSingleton()
        shadergen.addSceneManager(scn_mgr)  # must be done before we do anything with the scene

        scn_mgr.setAmbientLight(Ogre.ColourValue(.1, .1, .1))

        light = scn_mgr.createLight("MainLight")
        lightnode = scn_mgr.getRootSceneNode().createChildSceneNode()
        lightnode.setPosition(0, 10, 15)
        lightnode.attachObject(light)

        cam = scn_mgr.createCamera("myCam")
        cam.setNearClipDistance(5)
        cam.setAutoAspectRatio(True)
        camnode = scn_mgr.getRootSceneNode().createChildSceneNode()
        camnode.setPosition(0, 0, 15)

        camnode.lookAt(Ogre.Vector3(0, 0, -1), Ogre.Node.TS_PARENT)
        camnode.attachObject(cam)

        vp = self.getRenderWindow().addViewport(cam)
        vp.setBackgroundColour(Ogre.ColourValue(.3, .3, .3))

        ent = scn_mgr.createEntity("Sinbad.mesh")
        node = scn_mgr.getRootSceneNode().createChildSceneNode()
        node.attachObject(ent)


if __name__ == "__main__":
    app = ImguiExample()
    app.initApp()
    app.getRoot().startRendering()
    app.closeApp()
