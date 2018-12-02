%module(directors="1") OgreImgui
%{
#include "imgui.h"

#include "Ogre.h"
#include "OgreTrays.h"
#include "OgreAdvancedRenderControls.h"
#include "OgreCameraMan.h"
#include "ImguiRenderable.h"
#include "ImguiManager.h"
%}

%include std_string.i
%include exception.i
%include stdint.i
%include typemaps.i
%import "Ogre.i"
%import "Bites/OgreBites.i"

/// Imgui
// ignore va list methods
%ignore ImGui::TextV;
%ignore ImGui::TextColoredV;
%ignore ImGui::TextDisabledV;
%ignore ImGui::TextWrappedV;
%ignore ImGui::LabelTextV;
%ignore ImGui::BulletTextV;
%ignore ImGui::TreeNodeV;
%ignore ImGui::TreeNodeExV;
%ignore ImGui::SetTooltipV;
%ignore ImGuiTextBuffer::appendfv;

%apply bool* INOUT { bool* p_open };
%include "imgui.h"

#ifdef SWIGPYTHON
%pythoncode
%{
    __version__ = IMGUI_VERSION
%}
#endif

/// Ogre
%include "ImguiRenderable.h"
%include "ImguiManager.h"