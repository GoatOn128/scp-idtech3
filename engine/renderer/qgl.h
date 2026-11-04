/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
** QGL.H
*/
#ifndef __QGL_H__
#define __QGL_H__

//
// OpenGL entry points come from the vendored glad2 loader (external/glad).
// Every GL function lives in the GladGLContext struct; gladLoaderLoadGLContext
// fills it in once a context is current.  The historical qglXxx names are
// macros that redirect into that struct so the renderer sources keep their
// original call sites.
//

#include "glad/gl.h"

// global glad context, defined by the platform GLimp code
extern GladGLContext glimpGL;

// qglXxx -> glad context members
#define qglAccum glimpGL.Accum
#define qglActiveTextureARB glimpGL.ActiveTexture
#define qglAlphaFunc glimpGL.AlphaFunc
#define qglAreTexturesResident glimpGL.AreTexturesResident
#define qglArrayElement glimpGL.ArrayElement
#define qglBegin glimpGL.Begin
#define qglBindTexture glimpGL.BindTexture
#define qglBitmap glimpGL.Bitmap
#define qglBlendFunc glimpGL.BlendFunc
#define qglCallList glimpGL.CallList
#define qglCallLists glimpGL.CallLists
#define qglClear glimpGL.Clear
#define qglClearAccum glimpGL.ClearAccum
#define qglClearColor glimpGL.ClearColor
#define qglClearDepth glimpGL.ClearDepth
#define qglClearIndex glimpGL.ClearIndex
#define qglClearStencil glimpGL.ClearStencil
#define qglClientActiveTextureARB glimpGL.ClientActiveTexture
#define qglClipPlane glimpGL.ClipPlane
#define qglColor3b glimpGL.Color3b
#define qglColor3bv glimpGL.Color3bv
#define qglColor3d glimpGL.Color3d
#define qglColor3dv glimpGL.Color3dv
#define qglColor3f glimpGL.Color3f
#define qglColor3fv glimpGL.Color3fv
#define qglColor3i glimpGL.Color3i
#define qglColor3iv glimpGL.Color3iv
#define qglColor3s glimpGL.Color3s
#define qglColor3sv glimpGL.Color3sv
#define qglColor3ub glimpGL.Color3ub
#define qglColor3ubv glimpGL.Color3ubv
#define qglColor3ui glimpGL.Color3ui
#define qglColor3uiv glimpGL.Color3uiv
#define qglColor3us glimpGL.Color3us
#define qglColor3usv glimpGL.Color3usv
#define qglColor4b glimpGL.Color4b
#define qglColor4bv glimpGL.Color4bv
#define qglColor4d glimpGL.Color4d
#define qglColor4dv glimpGL.Color4dv
#define qglColor4f glimpGL.Color4f
#define qglColor4fv glimpGL.Color4fv
#define qglColor4i glimpGL.Color4i
#define qglColor4iv glimpGL.Color4iv
#define qglColor4s glimpGL.Color4s
#define qglColor4sv glimpGL.Color4sv
#define qglColor4ub glimpGL.Color4ub
#define qglColor4ubv glimpGL.Color4ubv
#define qglColor4ui glimpGL.Color4ui
#define qglColor4uiv glimpGL.Color4uiv
#define qglColor4us glimpGL.Color4us
#define qglColor4usv glimpGL.Color4usv
#define qglColorMask glimpGL.ColorMask
#define qglColorMaterial glimpGL.ColorMaterial
#define qglColorPointer glimpGL.ColorPointer
#define qglCopyPixels glimpGL.CopyPixels
#define qglCopyTexImage1D glimpGL.CopyTexImage1D
#define qglCopyTexImage2D glimpGL.CopyTexImage2D
#define qglCopyTexSubImage1D glimpGL.CopyTexSubImage1D
#define qglCopyTexSubImage2D glimpGL.CopyTexSubImage2D
#define qglCullFace glimpGL.CullFace
#define qglDeleteLists glimpGL.DeleteLists
#define qglDeleteTextures glimpGL.DeleteTextures
#define qglDepthFunc glimpGL.DepthFunc
#define qglDepthMask glimpGL.DepthMask
#define qglDepthRange glimpGL.DepthRange
#define qglDisable glimpGL.Disable
#define qglDisableClientState glimpGL.DisableClientState
#define qglDrawArrays glimpGL.DrawArrays
#define qglDrawBuffer glimpGL.DrawBuffer
#define qglDrawElements glimpGL.DrawElements
#define qglDrawPixels glimpGL.DrawPixels
#define qglEdgeFlag glimpGL.EdgeFlag
#define qglEdgeFlagPointer glimpGL.EdgeFlagPointer
#define qglEdgeFlagv glimpGL.EdgeFlagv
#define qglEnable glimpGL.Enable
#define qglEnableClientState glimpGL.EnableClientState
#define qglEnd glimpGL.End
#define qglEndList glimpGL.EndList
#define qglEvalCoord1d glimpGL.EvalCoord1d
#define qglEvalCoord1dv glimpGL.EvalCoord1dv
#define qglEvalCoord1f glimpGL.EvalCoord1f
#define qglEvalCoord1fv glimpGL.EvalCoord1fv
#define qglEvalCoord2d glimpGL.EvalCoord2d
#define qglEvalCoord2dv glimpGL.EvalCoord2dv
#define qglEvalCoord2f glimpGL.EvalCoord2f
#define qglEvalCoord2fv glimpGL.EvalCoord2fv
#define qglEvalMesh1 glimpGL.EvalMesh1
#define qglEvalMesh2 glimpGL.EvalMesh2
#define qglEvalPoint1 glimpGL.EvalPoint1
#define qglEvalPoint2 glimpGL.EvalPoint2
#define qglFeedbackBuffer glimpGL.FeedbackBuffer
#define qglFinish glimpGL.Finish
#define qglFlush glimpGL.Flush
#define qglFogf glimpGL.Fogf
#define qglFogfv glimpGL.Fogfv
#define qglFogi glimpGL.Fogi
#define qglFogiv glimpGL.Fogiv
#define qglFrontFace glimpGL.FrontFace
#define qglFrustum glimpGL.Frustum
#define qglGenLists glimpGL.GenLists
#define qglGenTextures glimpGL.GenTextures
#define qglGetBooleanv glimpGL.GetBooleanv
#define qglGetClipPlane glimpGL.GetClipPlane
#define qglGetDoublev glimpGL.GetDoublev
#define qglGetError glimpGL.GetError
#define qglGetFloatv glimpGL.GetFloatv
#define qglGetIntegerv glimpGL.GetIntegerv
#define qglGetLightfv glimpGL.GetLightfv
#define qglGetLightiv glimpGL.GetLightiv
#define qglGetMapdv glimpGL.GetMapdv
#define qglGetMapfv glimpGL.GetMapfv
#define qglGetMapiv glimpGL.GetMapiv
#define qglGetMaterialfv glimpGL.GetMaterialfv
#define qglGetMaterialiv glimpGL.GetMaterialiv
#define qglGetPixelMapfv glimpGL.GetPixelMapfv
#define qglGetPixelMapuiv glimpGL.GetPixelMapuiv
#define qglGetPixelMapusv glimpGL.GetPixelMapusv
#define qglGetPointerv glimpGL.GetPointerv
#define qglGetPolygonStipple glimpGL.GetPolygonStipple
#define qglGetString glimpGL.GetString
#define qglGetTexEnvfv glimpGL.GetTexEnvfv
#define qglGetTexEnviv glimpGL.GetTexEnviv
#define qglGetTexGendv glimpGL.GetTexGendv
#define qglGetTexGenfv glimpGL.GetTexGenfv
#define qglGetTexGeniv glimpGL.GetTexGeniv
#define qglGetTexImage glimpGL.GetTexImage
#define qglGetTexLevelParameterfv glimpGL.GetTexLevelParameterfv
#define qglGetTexLevelParameteriv glimpGL.GetTexLevelParameteriv
#define qglGetTexParameterfv glimpGL.GetTexParameterfv
#define qglGetTexParameteriv glimpGL.GetTexParameteriv
#define qglHint glimpGL.Hint
#define qglIndexd glimpGL.Indexd
#define qglIndexdv glimpGL.Indexdv
#define qglIndexf glimpGL.Indexf
#define qglIndexfv glimpGL.Indexfv
#define qglIndexi glimpGL.Indexi
#define qglIndexiv glimpGL.Indexiv
#define qglIndexMask glimpGL.IndexMask
#define qglIndexPointer glimpGL.IndexPointer
#define qglIndexs glimpGL.Indexs
#define qglIndexsv glimpGL.Indexsv
#define qglIndexub glimpGL.Indexub
#define qglIndexubv glimpGL.Indexubv
#define qglInitNames glimpGL.InitNames
#define qglInterleavedArrays glimpGL.InterleavedArrays
#define qglIsEnabled glimpGL.IsEnabled
#define qglIsList glimpGL.IsList
#define qglIsTexture glimpGL.IsTexture
#define qglLightf glimpGL.Lightf
#define qglLightfv glimpGL.Lightfv
#define qglLighti glimpGL.Lighti
#define qglLightiv glimpGL.Lightiv
#define qglLightModelf glimpGL.LightModelf
#define qglLightModelfv glimpGL.LightModelfv
#define qglLightModeli glimpGL.LightModeli
#define qglLightModeliv glimpGL.LightModeliv
#define qglLineStipple glimpGL.LineStipple
#define qglLineWidth glimpGL.LineWidth
#define qglListBase glimpGL.ListBase
#define qglLoadIdentity glimpGL.LoadIdentity
#define qglLoadMatrixd glimpGL.LoadMatrixd
#define qglLoadMatrixf glimpGL.LoadMatrixf
#define qglLoadName glimpGL.LoadName
#define qglLockArraysEXT glimpGL.LockArraysEXT
#define qglLogicOp glimpGL.LogicOp
#define qglMap1d glimpGL.Map1d
#define qglMap1f glimpGL.Map1f
#define qglMap2d glimpGL.Map2d
#define qglMap2f glimpGL.Map2f
#define qglMapGrid1d glimpGL.MapGrid1d
#define qglMapGrid1f glimpGL.MapGrid1f
#define qglMapGrid2d glimpGL.MapGrid2d
#define qglMapGrid2f glimpGL.MapGrid2f
#define qglMaterialf glimpGL.Materialf
#define qglMaterialfv glimpGL.Materialfv
#define qglMateriali glimpGL.Materiali
#define qglMaterialiv glimpGL.Materialiv
#define qglMatrixMode glimpGL.MatrixMode
#define qglMultiTexCoord2fARB glimpGL.MultiTexCoord2f
#define qglMultMatrixd glimpGL.MultMatrixd
#define qglMultMatrixf glimpGL.MultMatrixf
#define qglNewList glimpGL.NewList
#define qglNormal3b glimpGL.Normal3b
#define qglNormal3bv glimpGL.Normal3bv
#define qglNormal3d glimpGL.Normal3d
#define qglNormal3dv glimpGL.Normal3dv
#define qglNormal3f glimpGL.Normal3f
#define qglNormal3fv glimpGL.Normal3fv
#define qglNormal3i glimpGL.Normal3i
#define qglNormal3iv glimpGL.Normal3iv
#define qglNormal3s glimpGL.Normal3s
#define qglNormal3sv glimpGL.Normal3sv
#define qglNormalPointer glimpGL.NormalPointer
#define qglOrtho glimpGL.Ortho
#define qglPassThrough glimpGL.PassThrough
#define qglPixelMapfv glimpGL.PixelMapfv
#define qglPixelMapuiv glimpGL.PixelMapuiv
#define qglPixelMapusv glimpGL.PixelMapusv
#define qglPixelStoref glimpGL.PixelStoref
#define qglPixelStorei glimpGL.PixelStorei
#define qglPixelTransferf glimpGL.PixelTransferf
#define qglPixelTransferi glimpGL.PixelTransferi
#define qglPixelZoom glimpGL.PixelZoom
#define qglPNTrianglesfATI glimpGL.PNTrianglesfATI
#define qglPNTrianglesiATI glimpGL.PNTrianglesiATI
#define qglPointSize glimpGL.PointSize
#define qglPolygonMode glimpGL.PolygonMode
#define qglPolygonOffset glimpGL.PolygonOffset
#define qglPolygonStipple glimpGL.PolygonStipple
#define qglPopAttrib glimpGL.PopAttrib
#define qglPopClientAttrib glimpGL.PopClientAttrib
#define qglPopMatrix glimpGL.PopMatrix
#define qglPopName glimpGL.PopName
#define qglPrioritizeTextures glimpGL.PrioritizeTextures
#define qglPushAttrib glimpGL.PushAttrib
#define qglPushClientAttrib glimpGL.PushClientAttrib
#define qglPushMatrix glimpGL.PushMatrix
#define qglPushName glimpGL.PushName
#define qglRasterPos2d glimpGL.RasterPos2d
#define qglRasterPos2dv glimpGL.RasterPos2dv
#define qglRasterPos2f glimpGL.RasterPos2f
#define qglRasterPos2fv glimpGL.RasterPos2fv
#define qglRasterPos2i glimpGL.RasterPos2i
#define qglRasterPos2iv glimpGL.RasterPos2iv
#define qglRasterPos2s glimpGL.RasterPos2s
#define qglRasterPos2sv glimpGL.RasterPos2sv
#define qglRasterPos3d glimpGL.RasterPos3d
#define qglRasterPos3dv glimpGL.RasterPos3dv
#define qglRasterPos3f glimpGL.RasterPos3f
#define qglRasterPos3fv glimpGL.RasterPos3fv
#define qglRasterPos3i glimpGL.RasterPos3i
#define qglRasterPos3iv glimpGL.RasterPos3iv
#define qglRasterPos3s glimpGL.RasterPos3s
#define qglRasterPos3sv glimpGL.RasterPos3sv
#define qglRasterPos4d glimpGL.RasterPos4d
#define qglRasterPos4dv glimpGL.RasterPos4dv
#define qglRasterPos4f glimpGL.RasterPos4f
#define qglRasterPos4fv glimpGL.RasterPos4fv
#define qglRasterPos4i glimpGL.RasterPos4i
#define qglRasterPos4iv glimpGL.RasterPos4iv
#define qglRasterPos4s glimpGL.RasterPos4s
#define qglRasterPos4sv glimpGL.RasterPos4sv
#define qglReadBuffer glimpGL.ReadBuffer
#define qglReadPixels glimpGL.ReadPixels
#define qglRectd glimpGL.Rectd
#define qglRectdv glimpGL.Rectdv
#define qglRectf glimpGL.Rectf
#define qglRectfv glimpGL.Rectfv
#define qglRecti glimpGL.Recti
#define qglRectiv glimpGL.Rectiv
#define qglRects glimpGL.Rects
#define qglRectsv glimpGL.Rectsv
#define qglRenderMode glimpGL.RenderMode
#define qglRotated glimpGL.Rotated
#define qglRotatef glimpGL.Rotatef
#define qglScaled glimpGL.Scaled
#define qglScalef glimpGL.Scalef
#define qglScissor glimpGL.Scissor
#define qglSelectBuffer glimpGL.SelectBuffer
#define qglShadeModel glimpGL.ShadeModel
#define qglStencilFunc glimpGL.StencilFunc
#define qglStencilMask glimpGL.StencilMask
#define qglStencilOp glimpGL.StencilOp
#define qglTexCoord1d glimpGL.TexCoord1d
#define qglTexCoord1dv glimpGL.TexCoord1dv
#define qglTexCoord1f glimpGL.TexCoord1f
#define qglTexCoord1fv glimpGL.TexCoord1fv
#define qglTexCoord1i glimpGL.TexCoord1i
#define qglTexCoord1iv glimpGL.TexCoord1iv
#define qglTexCoord1s glimpGL.TexCoord1s
#define qglTexCoord1sv glimpGL.TexCoord1sv
#define qglTexCoord2d glimpGL.TexCoord2d
#define qglTexCoord2dv glimpGL.TexCoord2dv
#define qglTexCoord2f glimpGL.TexCoord2f
#define qglTexCoord2fv glimpGL.TexCoord2fv
#define qglTexCoord2i glimpGL.TexCoord2i
#define qglTexCoord2iv glimpGL.TexCoord2iv
#define qglTexCoord2s glimpGL.TexCoord2s
#define qglTexCoord2sv glimpGL.TexCoord2sv
#define qglTexCoord3d glimpGL.TexCoord3d
#define qglTexCoord3dv glimpGL.TexCoord3dv
#define qglTexCoord3f glimpGL.TexCoord3f
#define qglTexCoord3fv glimpGL.TexCoord3fv
#define qglTexCoord3i glimpGL.TexCoord3i
#define qglTexCoord3iv glimpGL.TexCoord3iv
#define qglTexCoord3s glimpGL.TexCoord3s
#define qglTexCoord3sv glimpGL.TexCoord3sv
#define qglTexCoord4d glimpGL.TexCoord4d
#define qglTexCoord4dv glimpGL.TexCoord4dv
#define qglTexCoord4f glimpGL.TexCoord4f
#define qglTexCoord4fv glimpGL.TexCoord4fv
#define qglTexCoord4i glimpGL.TexCoord4i
#define qglTexCoord4iv glimpGL.TexCoord4iv
#define qglTexCoord4s glimpGL.TexCoord4s
#define qglTexCoord4sv glimpGL.TexCoord4sv
#define qglTexCoordPointer glimpGL.TexCoordPointer
#define qglTexEnvf glimpGL.TexEnvf
#define qglTexEnvfv glimpGL.TexEnvfv
#define qglTexEnvi glimpGL.TexEnvi
#define qglTexEnviv glimpGL.TexEnviv
#define qglTexGend glimpGL.TexGend
#define qglTexGendv glimpGL.TexGendv
#define qglTexGenf glimpGL.TexGenf
#define qglTexGenfv glimpGL.TexGenfv
#define qglTexGeni glimpGL.TexGeni
#define qglTexGeniv glimpGL.TexGeniv
#define qglTexImage1D glimpGL.TexImage1D
#define qglTexImage2D glimpGL.TexImage2D
#define qglTexParameterf glimpGL.TexParameterf
#define qglTexParameterfv glimpGL.TexParameterfv
#define qglTexParameteri glimpGL.TexParameteri
#define qglTexParameteriv glimpGL.TexParameteriv
#define qglTexSubImage1D glimpGL.TexSubImage1D
#define qglTexSubImage2D glimpGL.TexSubImage2D
#define qglTranslated glimpGL.Translated
#define qglTranslatef glimpGL.Translatef
#define qglUnlockArraysEXT glimpGL.UnlockArraysEXT
#define qglVertex2d glimpGL.Vertex2d
#define qglVertex2dv glimpGL.Vertex2dv
#define qglVertex2f glimpGL.Vertex2f
#define qglVertex2fv glimpGL.Vertex2fv
#define qglVertex2i glimpGL.Vertex2i
#define qglVertex2iv glimpGL.Vertex2iv
#define qglVertex2s glimpGL.Vertex2s
#define qglVertex2sv glimpGL.Vertex2sv
#define qglVertex3d glimpGL.Vertex3d
#define qglVertex3dv glimpGL.Vertex3dv
#define qglVertex3f glimpGL.Vertex3f
#define qglVertex3fv glimpGL.Vertex3fv
#define qglVertex3i glimpGL.Vertex3i
#define qglVertex3iv glimpGL.Vertex3iv
#define qglVertex3s glimpGL.Vertex3s
#define qglVertex3sv glimpGL.Vertex3sv
#define qglVertex4d glimpGL.Vertex4d
#define qglVertex4dv glimpGL.Vertex4dv
#define qglVertex4f glimpGL.Vertex4f
#define qglVertex4fv glimpGL.Vertex4fv
#define qglVertex4i glimpGL.Vertex4i
#define qglVertex4iv glimpGL.Vertex4iv
#define qglVertex4s glimpGL.Vertex4s
#define qglVertex4sv glimpGL.Vertex4sv
#define qglVertexPointer glimpGL.VertexPointer
#define qglViewport glimpGL.Viewport

//
// legacy extension constants that predate the glad headers
//
#ifndef GL_ACTIVE_TEXTURE_ARB
#define GL_ACTIVE_TEXTURE_ARB               0x84E0
#endif
#ifndef GL_CLIENT_ACTIVE_TEXTURE_ARB
#define GL_CLIENT_ACTIVE_TEXTURE_ARB        0x84E1
#endif
#ifndef GL_MAX_ACTIVE_TEXTURES_ARB
#define GL_MAX_ACTIVE_TEXTURES_ARB          0x84E2
#endif
#ifndef GL_TEXTURE0_ARB
#define GL_TEXTURE0_ARB                     0x84C0
#endif
#ifndef GL_TEXTURE1_ARB
#define GL_TEXTURE1_ARB                     0x84C1
#endif
#ifndef GL_TEXTURE2_ARB
#define GL_TEXTURE2_ARB                     0x84C2
#endif
#ifndef GL_TEXTURE3_ARB
#define GL_TEXTURE3_ARB                     0x84C3
#endif

#ifndef GL_ATI_pn_triangles
#define GL_ATI_pn_triangles 1
#define GL_PN_TRIANGLES_ATI                         0x87F0
#define GL_MAX_PN_TRIANGLES_TESSELATION_LEVEL_ATI   0x87F1
#define GL_PN_TRIANGLES_POINT_MODE_ATI              0x87F2
#define GL_PN_TRIANGLES_NORMAL_MODE_ATI             0x87F3
#define GL_PN_TRIANGLES_TESSELATION_LEVEL_ATI       0x87F4
#define GL_PN_TRIANGLES_POINT_MODE_LINEAR_ATI       0x87F5
#define GL_PN_TRIANGLES_POINT_MODE_CUBIC_ATI        0x87F6
#define GL_PN_TRIANGLES_NORMAL_MODE_LINEAR_ATI      0x87F7
#define GL_PN_TRIANGLES_NORMAL_MODE_QUADRATIC_ATI   0x87F8
#endif

#ifndef GL_NV_fog_distance
#define GL_FOG_DISTANCE_MODE_NV           0x855A
#define GL_EYE_RADIAL_NV                  0x855B
#define GL_EYE_PLANE_ABSOLUTE_NV          0x855C
#endif

// S3TC compression constants
#ifndef GL_RGB_S3TC
#define GL_RGB_S3TC                         0x83A0
#endif
#ifndef GL_RGB4_S3TC
#define GL_RGB4_S3TC                        0x83A1
#endif

// GL_EXT_texture_compression_s3tc constants
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3
#endif

// GL_EXT_texture_filter_anisotropic constants
#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#endif
