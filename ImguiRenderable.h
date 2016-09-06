
#ifndef _ImGUIRenderable_H__
#define _ImGUIRenderable_H__

#include "OgrePrerequisites.h"
#include "OgreRenderable.h"
#include <OgreRenderOperation.h>

struct ImDrawData;
namespace Ogre 
{
    class SceneManager;
    class ImGUIRenderable : public Renderable
    {
    protected:

        MaterialPtr mMaterial;
        RenderOperation mRenderOp;

        void initImGUIRenderable(void);
       
    public:
        ImGUIRenderable();
        ~ImGUIRenderable();

        void updateVertexData(ImDrawData* data,unsigned int cmdIndex);
        Real getSquaredViewDepth(const Camera* cam) const   { (void)cam; return 0; }

        void setMaterial( const String& matName );
        virtual const MaterialPtr& getMaterial(void) const;
        virtual void getWorldTransforms( Matrix4* xform ) const;
        virtual void getRenderOperation( RenderOperation& op );
        virtual const LightList& getLights(void) const;

        int                      mVertexBufferSize;
        int                      mIndexBufferSize;

        

    };
    /** @} */
    /** @} */

}// namespace

#endif
