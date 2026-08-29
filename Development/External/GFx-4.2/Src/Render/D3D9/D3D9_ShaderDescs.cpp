/**************************************************************************

    PublicHeader:   Render
    Filename    :   D3D9_ShaderDescs.cpp
    Content     :   D3D9 Shader descriptors
    Created     :   04.08.2026
    Authors     :   Automatically generated.

    Copyright   :   Copyright 2026 Autodesk, Inc. All Rights reserved.

    Use of this software is subject to the terms of the Autodesk license
    agreement provided at the time of installation or download, or which
    otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/D3D9/D3D9_Shader.h"
#include "Render/D3D9/D3D9_ShaderDescs.h"

namespace Scaleform { namespace Render { namespace D3D9 {

const char* ShaderUniformNames[Uniform::SU_Count] = 
{
    "cxadd",
    "cxmul",
    "cxmul1",
    "fsize",
    "mvp",
    "offset",
    "scolor",
    "scolor2",
    "srctex",
    "srctexscale",
    "tex",
    "texgen",
    "texscale",
    "vfmuniforms",
    "vfuniforms",
};

const unsigned char Uniform::UniformFlags[Uniform::SU_Count] = 
{
    0,  // cxadd
    0,  // cxmul
    0,  // cxmul1
    0,  // fsize
    0,  // mvp
    0,  // offset
    0,  // scolor
    0,  // scolor2
    0,  // srctex
    0,  // srctexscale
    0,  // tex
    0,  // texgen
    0,  // texscale
    0,  // vfmuniforms
    0,  // vfuniforms
};

bool ShaderDesc::IsShaderVersionSupported(ShaderVersion ver)
{
    switch(ver)
    {
        case ShaderVersion_D3D9SM20: return true;
        case ShaderVersion_D3D9SM30: return true;
        default: return false;
    }
};

ShaderDesc::ShaderType ShaderDesc::GetShaderTypeForComboIndex(unsigned comboIndex, ShaderVersion ver)
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_D3D9SM20:
        {
            SF_UNUSED(comboIndex);
            SF_DEBUG_ASSERT(1, "D3D9SM20 indicated that ShaderCombo information was not required.");
            return ST_None;
        }
        case ShaderDesc::ShaderVersion_D3D9SM30:
        {
            SF_UNUSED(comboIndex);
            SF_DEBUG_ASSERT(1, "D3D9SM30 indicated that ShaderCombo information was not required.");
            return ST_None;
        }
        default: SF_DEBUG_ASSERT1(1, "Invalid shader platform provided (%d)", ver); return ST_None;
    }
};

ShaderDesc::ShaderIndex ShaderDesc::GetShaderIndex(ShaderType type)
{
    switch(type)
    {
        case ST_TexTG:                                           return STI_TexTG;
        case ST_TexTGMul:                                        return STI_TexTGMul;
        case ST_BatchTexTG:                                      return STI_BatchTexTG;
        case ST_BatchTexTGMul:                                   return STI_BatchTexTGMul;
        case ST_InstancedTexTG:                                  return STI_InstancedTexTG;
        case ST_InstancedTexTGMul:                               return STI_InstancedTexTGMul;
        case ST_Position3dTexTG:                                 return STI_Position3dTexTG;
        case ST_Position3dTexTGMul:                              return STI_Position3dTexTGMul;
        case ST_BatchPosition3dTexTG:                            return STI_BatchPosition3dTexTG;
        case ST_BatchPosition3dTexTGMul:                         return STI_BatchPosition3dTexTGMul;
        case ST_InstancedPosition3dTexTG:                        return STI_InstancedPosition3dTexTG;
        case ST_InstancedPosition3dTexTGMul:                     return STI_InstancedPosition3dTexTGMul;
        case ST_TexTGCxform:                                     return STI_TexTGCxform;
        case ST_TexTGCxformMul:                                  return STI_TexTGCxformMul;
        case ST_BatchTexTGCxform:                                return STI_BatchTexTGCxform;
        case ST_BatchTexTGCxformMul:                             return STI_BatchTexTGCxformMul;
        case ST_InstancedTexTGCxform:                            return STI_InstancedTexTGCxform;
        case ST_InstancedTexTGCxformMul:                         return STI_InstancedTexTGCxformMul;
        case ST_Position3dTexTGCxform:                           return STI_Position3dTexTGCxform;
        case ST_Position3dTexTGCxformMul:                        return STI_Position3dTexTGCxformMul;
        case ST_BatchPosition3dTexTGCxform:                      return STI_BatchPosition3dTexTGCxform;
        case ST_BatchPosition3dTexTGCxformMul:                   return STI_BatchPosition3dTexTGCxformMul;
        case ST_InstancedPosition3dTexTGCxform:                  return STI_InstancedPosition3dTexTGCxform;
        case ST_InstancedPosition3dTexTGCxformMul:               return STI_InstancedPosition3dTexTGCxformMul;
        case ST_TexTGEAlpha:                                     return STI_TexTGEAlpha;
        case ST_TexTGEAlphaMul:                                  return STI_TexTGEAlphaMul;
        case ST_BatchTexTGEAlpha:                                return STI_BatchTexTGEAlpha;
        case ST_BatchTexTGEAlphaMul:                             return STI_BatchTexTGEAlphaMul;
        case ST_InstancedTexTGEAlpha:                            return STI_InstancedTexTGEAlpha;
        case ST_InstancedTexTGEAlphaMul:                         return STI_InstancedTexTGEAlphaMul;
        case ST_Position3dTexTGEAlpha:                           return STI_Position3dTexTGEAlpha;
        case ST_Position3dTexTGEAlphaMul:                        return STI_Position3dTexTGEAlphaMul;
        case ST_BatchPosition3dTexTGEAlpha:                      return STI_BatchPosition3dTexTGEAlpha;
        case ST_BatchPosition3dTexTGEAlphaMul:                   return STI_BatchPosition3dTexTGEAlphaMul;
        case ST_InstancedPosition3dTexTGEAlpha:                  return STI_InstancedPosition3dTexTGEAlpha;
        case ST_InstancedPosition3dTexTGEAlphaMul:               return STI_InstancedPosition3dTexTGEAlphaMul;
        case ST_TexTGCxformEAlpha:                               return STI_TexTGCxformEAlpha;
        case ST_TexTGCxformEAlphaMul:                            return STI_TexTGCxformEAlphaMul;
        case ST_BatchTexTGCxformEAlpha:                          return STI_BatchTexTGCxformEAlpha;
        case ST_BatchTexTGCxformEAlphaMul:                       return STI_BatchTexTGCxformEAlphaMul;
        case ST_InstancedTexTGCxformEAlpha:                      return STI_InstancedTexTGCxformEAlpha;
        case ST_InstancedTexTGCxformEAlphaMul:                   return STI_InstancedTexTGCxformEAlphaMul;
        case ST_Position3dTexTGCxformEAlpha:                     return STI_Position3dTexTGCxformEAlpha;
        case ST_Position3dTexTGCxformEAlphaMul:                  return STI_Position3dTexTGCxformEAlphaMul;
        case ST_BatchPosition3dTexTGCxformEAlpha:                return STI_BatchPosition3dTexTGCxformEAlpha;
        case ST_BatchPosition3dTexTGCxformEAlphaMul:             return STI_BatchPosition3dTexTGCxformEAlphaMul;
        case ST_InstancedPosition3dTexTGCxformEAlpha:            return STI_InstancedPosition3dTexTGCxformEAlpha;
        case ST_InstancedPosition3dTexTGCxformEAlphaMul:         return STI_InstancedPosition3dTexTGCxformEAlphaMul;
        case ST_TexTGInv:                                        return STI_TexTGInv;
        case ST_BatchTexTGInv:                                   return STI_BatchTexTGInv;
        case ST_InstancedTexTGInv:                               return STI_InstancedTexTGInv;
        case ST_Position3dTexTGInv:                              return STI_Position3dTexTGInv;
        case ST_BatchPosition3dTexTGInv:                         return STI_BatchPosition3dTexTGInv;
        case ST_InstancedPosition3dTexTGInv:                     return STI_InstancedPosition3dTexTGInv;
        case ST_TexTGCxformInv:                                  return STI_TexTGCxformInv;
        case ST_BatchTexTGCxformInv:                             return STI_BatchTexTGCxformInv;
        case ST_InstancedTexTGCxformInv:                         return STI_InstancedTexTGCxformInv;
        case ST_Position3dTexTGCxformInv:                        return STI_Position3dTexTGCxformInv;
        case ST_BatchPosition3dTexTGCxformInv:                   return STI_BatchPosition3dTexTGCxformInv;
        case ST_InstancedPosition3dTexTGCxformInv:               return STI_InstancedPosition3dTexTGCxformInv;
        case ST_TexTGEAlphaInv:                                  return STI_TexTGEAlphaInv;
        case ST_BatchTexTGEAlphaInv:                             return STI_BatchTexTGEAlphaInv;
        case ST_InstancedTexTGEAlphaInv:                         return STI_InstancedTexTGEAlphaInv;
        case ST_Position3dTexTGEAlphaInv:                        return STI_Position3dTexTGEAlphaInv;
        case ST_BatchPosition3dTexTGEAlphaInv:                   return STI_BatchPosition3dTexTGEAlphaInv;
        case ST_InstancedPosition3dTexTGEAlphaInv:               return STI_InstancedPosition3dTexTGEAlphaInv;
        case ST_TexTGCxformEAlphaInv:                            return STI_TexTGCxformEAlphaInv;
        case ST_BatchTexTGCxformEAlphaInv:                       return STI_BatchTexTGCxformEAlphaInv;
        case ST_InstancedTexTGCxformEAlphaInv:                   return STI_InstancedTexTGCxformEAlphaInv;
        case ST_Position3dTexTGCxformEAlphaInv:                  return STI_Position3dTexTGCxformEAlphaInv;
        case ST_BatchPosition3dTexTGCxformEAlphaInv:             return STI_BatchPosition3dTexTGCxformEAlphaInv;
        case ST_InstancedPosition3dTexTGCxformEAlphaInv:         return STI_InstancedPosition3dTexTGCxformEAlphaInv;
        case ST_TexTGCxformAc:                                   return STI_TexTGCxformAc;
        case ST_TexTGCxformAcMul:                                return STI_TexTGCxformAcMul;
        case ST_BatchTexTGCxformAc:                              return STI_BatchTexTGCxformAc;
        case ST_BatchTexTGCxformAcMul:                           return STI_BatchTexTGCxformAcMul;
        case ST_InstancedTexTGCxformAc:                          return STI_InstancedTexTGCxformAc;
        case ST_InstancedTexTGCxformAcMul:                       return STI_InstancedTexTGCxformAcMul;
        case ST_Position3dTexTGCxformAc:                         return STI_Position3dTexTGCxformAc;
        case ST_Position3dTexTGCxformAcMul:                      return STI_Position3dTexTGCxformAcMul;
        case ST_BatchPosition3dTexTGCxformAc:                    return STI_BatchPosition3dTexTGCxformAc;
        case ST_BatchPosition3dTexTGCxformAcMul:                 return STI_BatchPosition3dTexTGCxformAcMul;
        case ST_InstancedPosition3dTexTGCxformAc:                return STI_InstancedPosition3dTexTGCxformAc;
        case ST_InstancedPosition3dTexTGCxformAcMul:             return STI_InstancedPosition3dTexTGCxformAcMul;
        case ST_TexTGCxformAcEAlpha:                             return STI_TexTGCxformAcEAlpha;
        case ST_TexTGCxformAcEAlphaMul:                          return STI_TexTGCxformAcEAlphaMul;
        case ST_BatchTexTGCxformAcEAlpha:                        return STI_BatchTexTGCxformAcEAlpha;
        case ST_BatchTexTGCxformAcEAlphaMul:                     return STI_BatchTexTGCxformAcEAlphaMul;
        case ST_InstancedTexTGCxformAcEAlpha:                    return STI_InstancedTexTGCxformAcEAlpha;
        case ST_InstancedTexTGCxformAcEAlphaMul:                 return STI_InstancedTexTGCxformAcEAlphaMul;
        case ST_Position3dTexTGCxformAcEAlpha:                   return STI_Position3dTexTGCxformAcEAlpha;
        case ST_Position3dTexTGCxformAcEAlphaMul:                return STI_Position3dTexTGCxformAcEAlphaMul;
        case ST_BatchPosition3dTexTGCxformAcEAlpha:              return STI_BatchPosition3dTexTGCxformAcEAlpha;
        case ST_BatchPosition3dTexTGCxformAcEAlphaMul:           return STI_BatchPosition3dTexTGCxformAcEAlphaMul;
        case ST_InstancedPosition3dTexTGCxformAcEAlpha:          return STI_InstancedPosition3dTexTGCxformAcEAlpha;
        case ST_InstancedPosition3dTexTGCxformAcEAlphaMul:       return STI_InstancedPosition3dTexTGCxformAcEAlphaMul;
        case ST_TexTGCxformAcInv:                                return STI_TexTGCxformAcInv;
        case ST_BatchTexTGCxformAcInv:                           return STI_BatchTexTGCxformAcInv;
        case ST_InstancedTexTGCxformAcInv:                       return STI_InstancedTexTGCxformAcInv;
        case ST_Position3dTexTGCxformAcInv:                      return STI_Position3dTexTGCxformAcInv;
        case ST_BatchPosition3dTexTGCxformAcInv:                 return STI_BatchPosition3dTexTGCxformAcInv;
        case ST_InstancedPosition3dTexTGCxformAcInv:             return STI_InstancedPosition3dTexTGCxformAcInv;
        case ST_TexTGCxformAcEAlphaInv:                          return STI_TexTGCxformAcEAlphaInv;
        case ST_BatchTexTGCxformAcEAlphaInv:                     return STI_BatchTexTGCxformAcEAlphaInv;
        case ST_InstancedTexTGCxformAcEAlphaInv:                 return STI_InstancedTexTGCxformAcEAlphaInv;
        case ST_Position3dTexTGCxformAcEAlphaInv:                return STI_Position3dTexTGCxformAcEAlphaInv;
        case ST_BatchPosition3dTexTGCxformAcEAlphaInv:           return STI_BatchPosition3dTexTGCxformAcEAlphaInv;
        case ST_InstancedPosition3dTexTGCxformAcEAlphaInv:       return STI_InstancedPosition3dTexTGCxformAcEAlphaInv;
        case ST_Vertex:                                          return STI_Vertex;
        case ST_VertexMul:                                       return STI_VertexMul;
        case ST_BatchVertex:                                     return STI_BatchVertex;
        case ST_BatchVertexMul:                                  return STI_BatchVertexMul;
        case ST_InstancedVertex:                                 return STI_InstancedVertex;
        case ST_InstancedVertexMul:                              return STI_InstancedVertexMul;
        case ST_Position3dVertex:                                return STI_Position3dVertex;
        case ST_Position3dVertexMul:                             return STI_Position3dVertexMul;
        case ST_BatchPosition3dVertex:                           return STI_BatchPosition3dVertex;
        case ST_BatchPosition3dVertexMul:                        return STI_BatchPosition3dVertexMul;
        case ST_InstancedPosition3dVertex:                       return STI_InstancedPosition3dVertex;
        case ST_InstancedPosition3dVertexMul:                    return STI_InstancedPosition3dVertexMul;
        case ST_VertexCxform:                                    return STI_VertexCxform;
        case ST_VertexCxformMul:                                 return STI_VertexCxformMul;
        case ST_BatchVertexCxform:                               return STI_BatchVertexCxform;
        case ST_BatchVertexCxformMul:                            return STI_BatchVertexCxformMul;
        case ST_InstancedVertexCxform:                           return STI_InstancedVertexCxform;
        case ST_InstancedVertexCxformMul:                        return STI_InstancedVertexCxformMul;
        case ST_Position3dVertexCxform:                          return STI_Position3dVertexCxform;
        case ST_Position3dVertexCxformMul:                       return STI_Position3dVertexCxformMul;
        case ST_BatchPosition3dVertexCxform:                     return STI_BatchPosition3dVertexCxform;
        case ST_BatchPosition3dVertexCxformMul:                  return STI_BatchPosition3dVertexCxformMul;
        case ST_InstancedPosition3dVertexCxform:                 return STI_InstancedPosition3dVertexCxform;
        case ST_InstancedPosition3dVertexCxformMul:              return STI_InstancedPosition3dVertexCxformMul;
        case ST_VertexEAlpha:                                    return STI_VertexEAlpha;
        case ST_VertexEAlphaMul:                                 return STI_VertexEAlphaMul;
        case ST_BatchVertexEAlpha:                               return STI_BatchVertexEAlpha;
        case ST_BatchVertexEAlphaMul:                            return STI_BatchVertexEAlphaMul;
        case ST_InstancedVertexEAlpha:                           return STI_InstancedVertexEAlpha;
        case ST_InstancedVertexEAlphaMul:                        return STI_InstancedVertexEAlphaMul;
        case ST_Position3dVertexEAlpha:                          return STI_Position3dVertexEAlpha;
        case ST_Position3dVertexEAlphaMul:                       return STI_Position3dVertexEAlphaMul;
        case ST_BatchPosition3dVertexEAlpha:                     return STI_BatchPosition3dVertexEAlpha;
        case ST_BatchPosition3dVertexEAlphaMul:                  return STI_BatchPosition3dVertexEAlphaMul;
        case ST_InstancedPosition3dVertexEAlpha:                 return STI_InstancedPosition3dVertexEAlpha;
        case ST_InstancedPosition3dVertexEAlphaMul:              return STI_InstancedPosition3dVertexEAlphaMul;
        case ST_VertexCxformEAlpha:                              return STI_VertexCxformEAlpha;
        case ST_VertexCxformEAlphaMul:                           return STI_VertexCxformEAlphaMul;
        case ST_BatchVertexCxformEAlpha:                         return STI_BatchVertexCxformEAlpha;
        case ST_BatchVertexCxformEAlphaMul:                      return STI_BatchVertexCxformEAlphaMul;
        case ST_InstancedVertexCxformEAlpha:                     return STI_InstancedVertexCxformEAlpha;
        case ST_InstancedVertexCxformEAlphaMul:                  return STI_InstancedVertexCxformEAlphaMul;
        case ST_Position3dVertexCxformEAlpha:                    return STI_Position3dVertexCxformEAlpha;
        case ST_Position3dVertexCxformEAlphaMul:                 return STI_Position3dVertexCxformEAlphaMul;
        case ST_BatchPosition3dVertexCxformEAlpha:               return STI_BatchPosition3dVertexCxformEAlpha;
        case ST_BatchPosition3dVertexCxformEAlphaMul:            return STI_BatchPosition3dVertexCxformEAlphaMul;
        case ST_InstancedPosition3dVertexCxformEAlpha:           return STI_InstancedPosition3dVertexCxformEAlpha;
        case ST_InstancedPosition3dVertexCxformEAlphaMul:        return STI_InstancedPosition3dVertexCxformEAlphaMul;
        case ST_VertexInv:                                       return STI_VertexInv;
        case ST_BatchVertexInv:                                  return STI_BatchVertexInv;
        case ST_InstancedVertexInv:                              return STI_InstancedVertexInv;
        case ST_Position3dVertexInv:                             return STI_Position3dVertexInv;
        case ST_BatchPosition3dVertexInv:                        return STI_BatchPosition3dVertexInv;
        case ST_InstancedPosition3dVertexInv:                    return STI_InstancedPosition3dVertexInv;
        case ST_VertexCxformInv:                                 return STI_VertexCxformInv;
        case ST_BatchVertexCxformInv:                            return STI_BatchVertexCxformInv;
        case ST_InstancedVertexCxformInv:                        return STI_InstancedVertexCxformInv;
        case ST_Position3dVertexCxformInv:                       return STI_Position3dVertexCxformInv;
        case ST_BatchPosition3dVertexCxformInv:                  return STI_BatchPosition3dVertexCxformInv;
        case ST_InstancedPosition3dVertexCxformInv:              return STI_InstancedPosition3dVertexCxformInv;
        case ST_VertexEAlphaInv:                                 return STI_VertexEAlphaInv;
        case ST_BatchVertexEAlphaInv:                            return STI_BatchVertexEAlphaInv;
        case ST_InstancedVertexEAlphaInv:                        return STI_InstancedVertexEAlphaInv;
        case ST_Position3dVertexEAlphaInv:                       return STI_Position3dVertexEAlphaInv;
        case ST_BatchPosition3dVertexEAlphaInv:                  return STI_BatchPosition3dVertexEAlphaInv;
        case ST_InstancedPosition3dVertexEAlphaInv:              return STI_InstancedPosition3dVertexEAlphaInv;
        case ST_VertexCxformEAlphaInv:                           return STI_VertexCxformEAlphaInv;
        case ST_BatchVertexCxformEAlphaInv:                      return STI_BatchVertexCxformEAlphaInv;
        case ST_InstancedVertexCxformEAlphaInv:                  return STI_InstancedVertexCxformEAlphaInv;
        case ST_Position3dVertexCxformEAlphaInv:                 return STI_Position3dVertexCxformEAlphaInv;
        case ST_BatchPosition3dVertexCxformEAlphaInv:            return STI_BatchPosition3dVertexCxformEAlphaInv;
        case ST_InstancedPosition3dVertexCxformEAlphaInv:        return STI_InstancedPosition3dVertexCxformEAlphaInv;
        case ST_VertexCxformAc:                                  return STI_VertexCxformAc;
        case ST_VertexCxformAcMul:                               return STI_VertexCxformAcMul;
        case ST_BatchVertexCxformAc:                             return STI_BatchVertexCxformAc;
        case ST_BatchVertexCxformAcMul:                          return STI_BatchVertexCxformAcMul;
        case ST_InstancedVertexCxformAc:                         return STI_InstancedVertexCxformAc;
        case ST_InstancedVertexCxformAcMul:                      return STI_InstancedVertexCxformAcMul;
        case ST_Position3dVertexCxformAc:                        return STI_Position3dVertexCxformAc;
        case ST_Position3dVertexCxformAcMul:                     return STI_Position3dVertexCxformAcMul;
        case ST_BatchPosition3dVertexCxformAc:                   return STI_BatchPosition3dVertexCxformAc;
        case ST_BatchPosition3dVertexCxformAcMul:                return STI_BatchPosition3dVertexCxformAcMul;
        case ST_InstancedPosition3dVertexCxformAc:               return STI_InstancedPosition3dVertexCxformAc;
        case ST_InstancedPosition3dVertexCxformAcMul:            return STI_InstancedPosition3dVertexCxformAcMul;
        case ST_VertexCxformAcEAlpha:                            return STI_VertexCxformAcEAlpha;
        case ST_VertexCxformAcEAlphaMul:                         return STI_VertexCxformAcEAlphaMul;
        case ST_BatchVertexCxformAcEAlpha:                       return STI_BatchVertexCxformAcEAlpha;
        case ST_BatchVertexCxformAcEAlphaMul:                    return STI_BatchVertexCxformAcEAlphaMul;
        case ST_InstancedVertexCxformAcEAlpha:                   return STI_InstancedVertexCxformAcEAlpha;
        case ST_InstancedVertexCxformAcEAlphaMul:                return STI_InstancedVertexCxformAcEAlphaMul;
        case ST_Position3dVertexCxformAcEAlpha:                  return STI_Position3dVertexCxformAcEAlpha;
        case ST_Position3dVertexCxformAcEAlphaMul:               return STI_Position3dVertexCxformAcEAlphaMul;
        case ST_BatchPosition3dVertexCxformAcEAlpha:             return STI_BatchPosition3dVertexCxformAcEAlpha;
        case ST_BatchPosition3dVertexCxformAcEAlphaMul:          return STI_BatchPosition3dVertexCxformAcEAlphaMul;
        case ST_InstancedPosition3dVertexCxformAcEAlpha:         return STI_InstancedPosition3dVertexCxformAcEAlpha;
        case ST_InstancedPosition3dVertexCxformAcEAlphaMul:      return STI_InstancedPosition3dVertexCxformAcEAlphaMul;
        case ST_VertexCxformAcInv:                               return STI_VertexCxformAcInv;
        case ST_BatchVertexCxformAcInv:                          return STI_BatchVertexCxformAcInv;
        case ST_InstancedVertexCxformAcInv:                      return STI_InstancedVertexCxformAcInv;
        case ST_Position3dVertexCxformAcInv:                     return STI_Position3dVertexCxformAcInv;
        case ST_BatchPosition3dVertexCxformAcInv:                return STI_BatchPosition3dVertexCxformAcInv;
        case ST_InstancedPosition3dVertexCxformAcInv:            return STI_InstancedPosition3dVertexCxformAcInv;
        case ST_VertexCxformAcEAlphaInv:                         return STI_VertexCxformAcEAlphaInv;
        case ST_BatchVertexCxformAcEAlphaInv:                    return STI_BatchVertexCxformAcEAlphaInv;
        case ST_InstancedVertexCxformAcEAlphaInv:                return STI_InstancedVertexCxformAcEAlphaInv;
        case ST_Position3dVertexCxformAcEAlphaInv:               return STI_Position3dVertexCxformAcEAlphaInv;
        case ST_BatchPosition3dVertexCxformAcEAlphaInv:          return STI_BatchPosition3dVertexCxformAcEAlphaInv;
        case ST_InstancedPosition3dVertexCxformAcEAlphaInv:      return STI_InstancedPosition3dVertexCxformAcEAlphaInv;
        case ST_TexTGTexTG:                                      return STI_TexTGTexTG;
        case ST_TexTGTexTGMul:                                   return STI_TexTGTexTGMul;
        case ST_BatchTexTGTexTG:                                 return STI_BatchTexTGTexTG;
        case ST_BatchTexTGTexTGMul:                              return STI_BatchTexTGTexTGMul;
        case ST_InstancedTexTGTexTG:                             return STI_InstancedTexTGTexTG;
        case ST_InstancedTexTGTexTGMul:                          return STI_InstancedTexTGTexTGMul;
        case ST_Position3dTexTGTexTG:                            return STI_Position3dTexTGTexTG;
        case ST_Position3dTexTGTexTGMul:                         return STI_Position3dTexTGTexTGMul;
        case ST_BatchPosition3dTexTGTexTG:                       return STI_BatchPosition3dTexTGTexTG;
        case ST_BatchPosition3dTexTGTexTGMul:                    return STI_BatchPosition3dTexTGTexTGMul;
        case ST_InstancedPosition3dTexTGTexTG:                   return STI_InstancedPosition3dTexTGTexTG;
        case ST_InstancedPosition3dTexTGTexTGMul:                return STI_InstancedPosition3dTexTGTexTGMul;
        case ST_TexTGTexTGCxform:                                return STI_TexTGTexTGCxform;
        case ST_TexTGTexTGCxformMul:                             return STI_TexTGTexTGCxformMul;
        case ST_BatchTexTGTexTGCxform:                           return STI_BatchTexTGTexTGCxform;
        case ST_BatchTexTGTexTGCxformMul:                        return STI_BatchTexTGTexTGCxformMul;
        case ST_InstancedTexTGTexTGCxform:                       return STI_InstancedTexTGTexTGCxform;
        case ST_InstancedTexTGTexTGCxformMul:                    return STI_InstancedTexTGTexTGCxformMul;
        case ST_Position3dTexTGTexTGCxform:                      return STI_Position3dTexTGTexTGCxform;
        case ST_Position3dTexTGTexTGCxformMul:                   return STI_Position3dTexTGTexTGCxformMul;
        case ST_BatchPosition3dTexTGTexTGCxform:                 return STI_BatchPosition3dTexTGTexTGCxform;
        case ST_BatchPosition3dTexTGTexTGCxformMul:              return STI_BatchPosition3dTexTGTexTGCxformMul;
        case ST_InstancedPosition3dTexTGTexTGCxform:             return STI_InstancedPosition3dTexTGTexTGCxform;
        case ST_InstancedPosition3dTexTGTexTGCxformMul:          return STI_InstancedPosition3dTexTGTexTGCxformMul;
        case ST_TexTGTexTGEAlpha:                                return STI_TexTGTexTGEAlpha;
        case ST_TexTGTexTGEAlphaMul:                             return STI_TexTGTexTGEAlphaMul;
        case ST_BatchTexTGTexTGEAlpha:                           return STI_BatchTexTGTexTGEAlpha;
        case ST_BatchTexTGTexTGEAlphaMul:                        return STI_BatchTexTGTexTGEAlphaMul;
        case ST_InstancedTexTGTexTGEAlpha:                       return STI_InstancedTexTGTexTGEAlpha;
        case ST_InstancedTexTGTexTGEAlphaMul:                    return STI_InstancedTexTGTexTGEAlphaMul;
        case ST_Position3dTexTGTexTGEAlpha:                      return STI_Position3dTexTGTexTGEAlpha;
        case ST_Position3dTexTGTexTGEAlphaMul:                   return STI_Position3dTexTGTexTGEAlphaMul;
        case ST_BatchPosition3dTexTGTexTGEAlpha:                 return STI_BatchPosition3dTexTGTexTGEAlpha;
        case ST_BatchPosition3dTexTGTexTGEAlphaMul:              return STI_BatchPosition3dTexTGTexTGEAlphaMul;
        case ST_InstancedPosition3dTexTGTexTGEAlpha:             return STI_InstancedPosition3dTexTGTexTGEAlpha;
        case ST_InstancedPosition3dTexTGTexTGEAlphaMul:          return STI_InstancedPosition3dTexTGTexTGEAlphaMul;
        case ST_TexTGTexTGCxformEAlpha:                          return STI_TexTGTexTGCxformEAlpha;
        case ST_TexTGTexTGCxformEAlphaMul:                       return STI_TexTGTexTGCxformEAlphaMul;
        case ST_BatchTexTGTexTGCxformEAlpha:                     return STI_BatchTexTGTexTGCxformEAlpha;
        case ST_BatchTexTGTexTGCxformEAlphaMul:                  return STI_BatchTexTGTexTGCxformEAlphaMul;
        case ST_InstancedTexTGTexTGCxformEAlpha:                 return STI_InstancedTexTGTexTGCxformEAlpha;
        case ST_InstancedTexTGTexTGCxformEAlphaMul:              return STI_InstancedTexTGTexTGCxformEAlphaMul;
        case ST_Position3dTexTGTexTGCxformEAlpha:                return STI_Position3dTexTGTexTGCxformEAlpha;
        case ST_Position3dTexTGTexTGCxformEAlphaMul:             return STI_Position3dTexTGTexTGCxformEAlphaMul;
        case ST_BatchPosition3dTexTGTexTGCxformEAlpha:           return STI_BatchPosition3dTexTGTexTGCxformEAlpha;
        case ST_BatchPosition3dTexTGTexTGCxformEAlphaMul:        return STI_BatchPosition3dTexTGTexTGCxformEAlphaMul;
        case ST_InstancedPosition3dTexTGTexTGCxformEAlpha:       return STI_InstancedPosition3dTexTGTexTGCxformEAlpha;
        case ST_InstancedPosition3dTexTGTexTGCxformEAlphaMul:    return STI_InstancedPosition3dTexTGTexTGCxformEAlphaMul;
        case ST_TexTGTexTGInv:                                   return STI_TexTGTexTGInv;
        case ST_BatchTexTGTexTGInv:                              return STI_BatchTexTGTexTGInv;
        case ST_InstancedTexTGTexTGInv:                          return STI_InstancedTexTGTexTGInv;
        case ST_Position3dTexTGTexTGInv:                         return STI_Position3dTexTGTexTGInv;
        case ST_BatchPosition3dTexTGTexTGInv:                    return STI_BatchPosition3dTexTGTexTGInv;
        case ST_InstancedPosition3dTexTGTexTGInv:                return STI_InstancedPosition3dTexTGTexTGInv;
        case ST_TexTGTexTGCxformInv:                             return STI_TexTGTexTGCxformInv;
        case ST_BatchTexTGTexTGCxformInv:                        return STI_BatchTexTGTexTGCxformInv;
        case ST_InstancedTexTGTexTGCxformInv:                    return STI_InstancedTexTGTexTGCxformInv;
        case ST_Position3dTexTGTexTGCxformInv:                   return STI_Position3dTexTGTexTGCxformInv;
        case ST_BatchPosition3dTexTGTexTGCxformInv:              return STI_BatchPosition3dTexTGTexTGCxformInv;
        case ST_InstancedPosition3dTexTGTexTGCxformInv:          return STI_InstancedPosition3dTexTGTexTGCxformInv;
        case ST_TexTGTexTGEAlphaInv:                             return STI_TexTGTexTGEAlphaInv;
        case ST_BatchTexTGTexTGEAlphaInv:                        return STI_BatchTexTGTexTGEAlphaInv;
        case ST_InstancedTexTGTexTGEAlphaInv:                    return STI_InstancedTexTGTexTGEAlphaInv;
        case ST_Position3dTexTGTexTGEAlphaInv:                   return STI_Position3dTexTGTexTGEAlphaInv;
        case ST_BatchPosition3dTexTGTexTGEAlphaInv:              return STI_BatchPosition3dTexTGTexTGEAlphaInv;
        case ST_InstancedPosition3dTexTGTexTGEAlphaInv:          return STI_InstancedPosition3dTexTGTexTGEAlphaInv;
        case ST_TexTGTexTGCxformEAlphaInv:                       return STI_TexTGTexTGCxformEAlphaInv;
        case ST_BatchTexTGTexTGCxformEAlphaInv:                  return STI_BatchTexTGTexTGCxformEAlphaInv;
        case ST_InstancedTexTGTexTGCxformEAlphaInv:              return STI_InstancedTexTGTexTGCxformEAlphaInv;
        case ST_Position3dTexTGTexTGCxformEAlphaInv:             return STI_Position3dTexTGTexTGCxformEAlphaInv;
        case ST_BatchPosition3dTexTGTexTGCxformEAlphaInv:        return STI_BatchPosition3dTexTGTexTGCxformEAlphaInv;
        case ST_InstancedPosition3dTexTGTexTGCxformEAlphaInv:    return STI_InstancedPosition3dTexTGTexTGCxformEAlphaInv;
        case ST_TexTGTexTGCxformAc:                              return STI_TexTGTexTGCxformAc;
        case ST_TexTGTexTGCxformAcMul:                           return STI_TexTGTexTGCxformAcMul;
        case ST_BatchTexTGTexTGCxformAc:                         return STI_BatchTexTGTexTGCxformAc;
        case ST_BatchTexTGTexTGCxformAcMul:                      return STI_BatchTexTGTexTGCxformAcMul;
        case ST_InstancedTexTGTexTGCxformAc:                     return STI_InstancedTexTGTexTGCxformAc;
        case ST_InstancedTexTGTexTGCxformAcMul:                  return STI_InstancedTexTGTexTGCxformAcMul;
        case ST_Position3dTexTGTexTGCxformAc:                    return STI_Position3dTexTGTexTGCxformAc;
        case ST_Position3dTexTGTexTGCxformAcMul:                 return STI_Position3dTexTGTexTGCxformAcMul;
        case ST_BatchPosition3dTexTGTexTGCxformAc:               return STI_BatchPosition3dTexTGTexTGCxformAc;
        case ST_BatchPosition3dTexTGTexTGCxformAcMul:            return STI_BatchPosition3dTexTGTexTGCxformAcMul;
        case ST_InstancedPosition3dTexTGTexTGCxformAc:           return STI_InstancedPosition3dTexTGTexTGCxformAc;
        case ST_InstancedPosition3dTexTGTexTGCxformAcMul:        return STI_InstancedPosition3dTexTGTexTGCxformAcMul;
        case ST_TexTGTexTGCxformAcEAlpha:                        return STI_TexTGTexTGCxformAcEAlpha;
        case ST_TexTGTexTGCxformAcEAlphaMul:                     return STI_TexTGTexTGCxformAcEAlphaMul;
        case ST_BatchTexTGTexTGCxformAcEAlpha:                   return STI_BatchTexTGTexTGCxformAcEAlpha;
        case ST_BatchTexTGTexTGCxformAcEAlphaMul:                return STI_BatchTexTGTexTGCxformAcEAlphaMul;
        case ST_InstancedTexTGTexTGCxformAcEAlpha:               return STI_InstancedTexTGTexTGCxformAcEAlpha;
        case ST_InstancedTexTGTexTGCxformAcEAlphaMul:            return STI_InstancedTexTGTexTGCxformAcEAlphaMul;
        case ST_Position3dTexTGTexTGCxformAcEAlpha:              return STI_Position3dTexTGTexTGCxformAcEAlpha;
        case ST_Position3dTexTGTexTGCxformAcEAlphaMul:           return STI_Position3dTexTGTexTGCxformAcEAlphaMul;
        case ST_BatchPosition3dTexTGTexTGCxformAcEAlpha:         return STI_BatchPosition3dTexTGTexTGCxformAcEAlpha;
        case ST_BatchPosition3dTexTGTexTGCxformAcEAlphaMul:      return STI_BatchPosition3dTexTGTexTGCxformAcEAlphaMul;
        case ST_InstancedPosition3dTexTGTexTGCxformAcEAlpha:     return STI_InstancedPosition3dTexTGTexTGCxformAcEAlpha;
        case ST_InstancedPosition3dTexTGTexTGCxformAcEAlphaMul:  return STI_InstancedPosition3dTexTGTexTGCxformAcEAlphaMul;
        case ST_TexTGTexTGCxformAcInv:                           return STI_TexTGTexTGCxformAcInv;
        case ST_BatchTexTGTexTGCxformAcInv:                      return STI_BatchTexTGTexTGCxformAcInv;
        case ST_InstancedTexTGTexTGCxformAcInv:                  return STI_InstancedTexTGTexTGCxformAcInv;
        case ST_Position3dTexTGTexTGCxformAcInv:                 return STI_Position3dTexTGTexTGCxformAcInv;
        case ST_BatchPosition3dTexTGTexTGCxformAcInv:            return STI_BatchPosition3dTexTGTexTGCxformAcInv;
        case ST_InstancedPosition3dTexTGTexTGCxformAcInv:        return STI_InstancedPosition3dTexTGTexTGCxformAcInv;
        case ST_TexTGTexTGCxformAcEAlphaInv:                     return STI_TexTGTexTGCxformAcEAlphaInv;
        case ST_BatchTexTGTexTGCxformAcEAlphaInv:                return STI_BatchTexTGTexTGCxformAcEAlphaInv;
        case ST_InstancedTexTGTexTGCxformAcEAlphaInv:            return STI_InstancedTexTGTexTGCxformAcEAlphaInv;
        case ST_Position3dTexTGTexTGCxformAcEAlphaInv:           return STI_Position3dTexTGTexTGCxformAcEAlphaInv;
        case ST_BatchPosition3dTexTGTexTGCxformAcEAlphaInv:      return STI_BatchPosition3dTexTGTexTGCxformAcEAlphaInv;
        case ST_InstancedPosition3dTexTGTexTGCxformAcEAlphaInv:  return STI_InstancedPosition3dTexTGTexTGCxformAcEAlphaInv;
        case ST_TexTGVertex:                                     return STI_TexTGVertex;
        case ST_TexTGVertexMul:                                  return STI_TexTGVertexMul;
        case ST_BatchTexTGVertex:                                return STI_BatchTexTGVertex;
        case ST_BatchTexTGVertexMul:                             return STI_BatchTexTGVertexMul;
        case ST_InstancedTexTGVertex:                            return STI_InstancedTexTGVertex;
        case ST_InstancedTexTGVertexMul:                         return STI_InstancedTexTGVertexMul;
        case ST_Position3dTexTGVertex:                           return STI_Position3dTexTGVertex;
        case ST_Position3dTexTGVertexMul:                        return STI_Position3dTexTGVertexMul;
        case ST_BatchPosition3dTexTGVertex:                      return STI_BatchPosition3dTexTGVertex;
        case ST_BatchPosition3dTexTGVertexMul:                   return STI_BatchPosition3dTexTGVertexMul;
        case ST_InstancedPosition3dTexTGVertex:                  return STI_InstancedPosition3dTexTGVertex;
        case ST_InstancedPosition3dTexTGVertexMul:               return STI_InstancedPosition3dTexTGVertexMul;
        case ST_TexTGVertexCxform:                               return STI_TexTGVertexCxform;
        case ST_TexTGVertexCxformMul:                            return STI_TexTGVertexCxformMul;
        case ST_BatchTexTGVertexCxform:                          return STI_BatchTexTGVertexCxform;
        case ST_BatchTexTGVertexCxformMul:                       return STI_BatchTexTGVertexCxformMul;
        case ST_InstancedTexTGVertexCxform:                      return STI_InstancedTexTGVertexCxform;
        case ST_InstancedTexTGVertexCxformMul:                   return STI_InstancedTexTGVertexCxformMul;
        case ST_Position3dTexTGVertexCxform:                     return STI_Position3dTexTGVertexCxform;
        case ST_Position3dTexTGVertexCxformMul:                  return STI_Position3dTexTGVertexCxformMul;
        case ST_BatchPosition3dTexTGVertexCxform:                return STI_BatchPosition3dTexTGVertexCxform;
        case ST_BatchPosition3dTexTGVertexCxformMul:             return STI_BatchPosition3dTexTGVertexCxformMul;
        case ST_InstancedPosition3dTexTGVertexCxform:            return STI_InstancedPosition3dTexTGVertexCxform;
        case ST_InstancedPosition3dTexTGVertexCxformMul:         return STI_InstancedPosition3dTexTGVertexCxformMul;
        case ST_TexTGVertexEAlpha:                               return STI_TexTGVertexEAlpha;
        case ST_TexTGVertexEAlphaMul:                            return STI_TexTGVertexEAlphaMul;
        case ST_BatchTexTGVertexEAlpha:                          return STI_BatchTexTGVertexEAlpha;
        case ST_BatchTexTGVertexEAlphaMul:                       return STI_BatchTexTGVertexEAlphaMul;
        case ST_InstancedTexTGVertexEAlpha:                      return STI_InstancedTexTGVertexEAlpha;
        case ST_InstancedTexTGVertexEAlphaMul:                   return STI_InstancedTexTGVertexEAlphaMul;
        case ST_Position3dTexTGVertexEAlpha:                     return STI_Position3dTexTGVertexEAlpha;
        case ST_Position3dTexTGVertexEAlphaMul:                  return STI_Position3dTexTGVertexEAlphaMul;
        case ST_BatchPosition3dTexTGVertexEAlpha:                return STI_BatchPosition3dTexTGVertexEAlpha;
        case ST_BatchPosition3dTexTGVertexEAlphaMul:             return STI_BatchPosition3dTexTGVertexEAlphaMul;
        case ST_InstancedPosition3dTexTGVertexEAlpha:            return STI_InstancedPosition3dTexTGVertexEAlpha;
        case ST_InstancedPosition3dTexTGVertexEAlphaMul:         return STI_InstancedPosition3dTexTGVertexEAlphaMul;
        case ST_TexTGVertexCxformEAlpha:                         return STI_TexTGVertexCxformEAlpha;
        case ST_TexTGVertexCxformEAlphaMul:                      return STI_TexTGVertexCxformEAlphaMul;
        case ST_BatchTexTGVertexCxformEAlpha:                    return STI_BatchTexTGVertexCxformEAlpha;
        case ST_BatchTexTGVertexCxformEAlphaMul:                 return STI_BatchTexTGVertexCxformEAlphaMul;
        case ST_InstancedTexTGVertexCxformEAlpha:                return STI_InstancedTexTGVertexCxformEAlpha;
        case ST_InstancedTexTGVertexCxformEAlphaMul:             return STI_InstancedTexTGVertexCxformEAlphaMul;
        case ST_Position3dTexTGVertexCxformEAlpha:               return STI_Position3dTexTGVertexCxformEAlpha;
        case ST_Position3dTexTGVertexCxformEAlphaMul:            return STI_Position3dTexTGVertexCxformEAlphaMul;
        case ST_BatchPosition3dTexTGVertexCxformEAlpha:          return STI_BatchPosition3dTexTGVertexCxformEAlpha;
        case ST_BatchPosition3dTexTGVertexCxformEAlphaMul:       return STI_BatchPosition3dTexTGVertexCxformEAlphaMul;
        case ST_InstancedPosition3dTexTGVertexCxformEAlpha:      return STI_InstancedPosition3dTexTGVertexCxformEAlpha;
        case ST_InstancedPosition3dTexTGVertexCxformEAlphaMul:   return STI_InstancedPosition3dTexTGVertexCxformEAlphaMul;
        case ST_TexTGVertexInv:                                  return STI_TexTGVertexInv;
        case ST_BatchTexTGVertexInv:                             return STI_BatchTexTGVertexInv;
        case ST_InstancedTexTGVertexInv:                         return STI_InstancedTexTGVertexInv;
        case ST_Position3dTexTGVertexInv:                        return STI_Position3dTexTGVertexInv;
        case ST_BatchPosition3dTexTGVertexInv:                   return STI_BatchPosition3dTexTGVertexInv;
        case ST_InstancedPosition3dTexTGVertexInv:               return STI_InstancedPosition3dTexTGVertexInv;
        case ST_TexTGVertexCxformInv:                            return STI_TexTGVertexCxformInv;
        case ST_BatchTexTGVertexCxformInv:                       return STI_BatchTexTGVertexCxformInv;
        case ST_InstancedTexTGVertexCxformInv:                   return STI_InstancedTexTGVertexCxformInv;
        case ST_Position3dTexTGVertexCxformInv:                  return STI_Position3dTexTGVertexCxformInv;
        case ST_BatchPosition3dTexTGVertexCxformInv:             return STI_BatchPosition3dTexTGVertexCxformInv;
        case ST_InstancedPosition3dTexTGVertexCxformInv:         return STI_InstancedPosition3dTexTGVertexCxformInv;
        case ST_TexTGVertexEAlphaInv:                            return STI_TexTGVertexEAlphaInv;
        case ST_BatchTexTGVertexEAlphaInv:                       return STI_BatchTexTGVertexEAlphaInv;
        case ST_InstancedTexTGVertexEAlphaInv:                   return STI_InstancedTexTGVertexEAlphaInv;
        case ST_Position3dTexTGVertexEAlphaInv:                  return STI_Position3dTexTGVertexEAlphaInv;
        case ST_BatchPosition3dTexTGVertexEAlphaInv:             return STI_BatchPosition3dTexTGVertexEAlphaInv;
        case ST_InstancedPosition3dTexTGVertexEAlphaInv:         return STI_InstancedPosition3dTexTGVertexEAlphaInv;
        case ST_TexTGVertexCxformEAlphaInv:                      return STI_TexTGVertexCxformEAlphaInv;
        case ST_BatchTexTGVertexCxformEAlphaInv:                 return STI_BatchTexTGVertexCxformEAlphaInv;
        case ST_InstancedTexTGVertexCxformEAlphaInv:             return STI_InstancedTexTGVertexCxformEAlphaInv;
        case ST_Position3dTexTGVertexCxformEAlphaInv:            return STI_Position3dTexTGVertexCxformEAlphaInv;
        case ST_BatchPosition3dTexTGVertexCxformEAlphaInv:       return STI_BatchPosition3dTexTGVertexCxformEAlphaInv;
        case ST_InstancedPosition3dTexTGVertexCxformEAlphaInv:   return STI_InstancedPosition3dTexTGVertexCxformEAlphaInv;
        case ST_TexTGVertexCxformAc:                             return STI_TexTGVertexCxformAc;
        case ST_TexTGVertexCxformAcMul:                          return STI_TexTGVertexCxformAcMul;
        case ST_BatchTexTGVertexCxformAc:                        return STI_BatchTexTGVertexCxformAc;
        case ST_BatchTexTGVertexCxformAcMul:                     return STI_BatchTexTGVertexCxformAcMul;
        case ST_InstancedTexTGVertexCxformAc:                    return STI_InstancedTexTGVertexCxformAc;
        case ST_InstancedTexTGVertexCxformAcMul:                 return STI_InstancedTexTGVertexCxformAcMul;
        case ST_Position3dTexTGVertexCxformAc:                   return STI_Position3dTexTGVertexCxformAc;
        case ST_Position3dTexTGVertexCxformAcMul:                return STI_Position3dTexTGVertexCxformAcMul;
        case ST_BatchPosition3dTexTGVertexCxformAc:              return STI_BatchPosition3dTexTGVertexCxformAc;
        case ST_BatchPosition3dTexTGVertexCxformAcMul:           return STI_BatchPosition3dTexTGVertexCxformAcMul;
        case ST_InstancedPosition3dTexTGVertexCxformAc:          return STI_InstancedPosition3dTexTGVertexCxformAc;
        case ST_InstancedPosition3dTexTGVertexCxformAcMul:       return STI_InstancedPosition3dTexTGVertexCxformAcMul;
        case ST_TexTGVertexCxformAcEAlpha:                       return STI_TexTGVertexCxformAcEAlpha;
        case ST_TexTGVertexCxformAcEAlphaMul:                    return STI_TexTGVertexCxformAcEAlphaMul;
        case ST_BatchTexTGVertexCxformAcEAlpha:                  return STI_BatchTexTGVertexCxformAcEAlpha;
        case ST_BatchTexTGVertexCxformAcEAlphaMul:               return STI_BatchTexTGVertexCxformAcEAlphaMul;
        case ST_InstancedTexTGVertexCxformAcEAlpha:              return STI_InstancedTexTGVertexCxformAcEAlpha;
        case ST_InstancedTexTGVertexCxformAcEAlphaMul:           return STI_InstancedTexTGVertexCxformAcEAlphaMul;
        case ST_Position3dTexTGVertexCxformAcEAlpha:             return STI_Position3dTexTGVertexCxformAcEAlpha;
        case ST_Position3dTexTGVertexCxformAcEAlphaMul:          return STI_Position3dTexTGVertexCxformAcEAlphaMul;
        case ST_BatchPosition3dTexTGVertexCxformAcEAlpha:        return STI_BatchPosition3dTexTGVertexCxformAcEAlpha;
        case ST_BatchPosition3dTexTGVertexCxformAcEAlphaMul:     return STI_BatchPosition3dTexTGVertexCxformAcEAlphaMul;
        case ST_InstancedPosition3dTexTGVertexCxformAcEAlpha:    return STI_InstancedPosition3dTexTGVertexCxformAcEAlpha;
        case ST_InstancedPosition3dTexTGVertexCxformAcEAlphaMul: return STI_InstancedPosition3dTexTGVertexCxformAcEAlphaMul;
        case ST_TexTGVertexCxformAcInv:                          return STI_TexTGVertexCxformAcInv;
        case ST_BatchTexTGVertexCxformAcInv:                     return STI_BatchTexTGVertexCxformAcInv;
        case ST_InstancedTexTGVertexCxformAcInv:                 return STI_InstancedTexTGVertexCxformAcInv;
        case ST_Position3dTexTGVertexCxformAcInv:                return STI_Position3dTexTGVertexCxformAcInv;
        case ST_BatchPosition3dTexTGVertexCxformAcInv:           return STI_BatchPosition3dTexTGVertexCxformAcInv;
        case ST_InstancedPosition3dTexTGVertexCxformAcInv:       return STI_InstancedPosition3dTexTGVertexCxformAcInv;
        case ST_TexTGVertexCxformAcEAlphaInv:                    return STI_TexTGVertexCxformAcEAlphaInv;
        case ST_BatchTexTGVertexCxformAcEAlphaInv:               return STI_BatchTexTGVertexCxformAcEAlphaInv;
        case ST_InstancedTexTGVertexCxformAcEAlphaInv:           return STI_InstancedTexTGVertexCxformAcEAlphaInv;
        case ST_Position3dTexTGVertexCxformAcEAlphaInv:          return STI_Position3dTexTGVertexCxformAcEAlphaInv;
        case ST_BatchPosition3dTexTGVertexCxformAcEAlphaInv:     return STI_BatchPosition3dTexTGVertexCxformAcEAlphaInv;
        case ST_InstancedPosition3dTexTGVertexCxformAcEAlphaInv: return STI_InstancedPosition3dTexTGVertexCxformAcEAlphaInv;
        case ST_Solid:                                           return STI_Solid;
        case ST_SolidMul:                                        return STI_SolidMul;
        case ST_BatchSolid:                                      return STI_BatchSolid;
        case ST_BatchSolidMul:                                   return STI_BatchSolidMul;
        case ST_InstancedSolid:                                  return STI_InstancedSolid;
        case ST_InstancedSolidMul:                               return STI_InstancedSolidMul;
        case ST_Position3dSolid:                                 return STI_Position3dSolid;
        case ST_Position3dSolidMul:                              return STI_Position3dSolidMul;
        case ST_BatchPosition3dSolid:                            return STI_BatchPosition3dSolid;
        case ST_BatchPosition3dSolidMul:                         return STI_BatchPosition3dSolidMul;
        case ST_InstancedPosition3dSolid:                        return STI_InstancedPosition3dSolid;
        case ST_InstancedPosition3dSolidMul:                     return STI_InstancedPosition3dSolidMul;
        case ST_SolidInv:                                        return STI_SolidInv;
        case ST_BatchSolidInv:                                   return STI_BatchSolidInv;
        case ST_InstancedSolidInv:                               return STI_InstancedSolidInv;
        case ST_Position3dSolidInv:                              return STI_Position3dSolidInv;
        case ST_BatchPosition3dSolidInv:                         return STI_BatchPosition3dSolidInv;
        case ST_InstancedPosition3dSolidInv:                     return STI_InstancedPosition3dSolidInv;
        case ST_Text:                                            return STI_Text;
        case ST_TextMul:                                         return STI_TextMul;
        case ST_BatchText:                                       return STI_BatchText;
        case ST_BatchTextMul:                                    return STI_BatchTextMul;
        case ST_InstancedText:                                   return STI_InstancedText;
        case ST_InstancedTextMul:                                return STI_InstancedTextMul;
        case ST_Position3dText:                                  return STI_Position3dText;
        case ST_Position3dTextMul:                               return STI_Position3dTextMul;
        case ST_BatchPosition3dText:                             return STI_BatchPosition3dText;
        case ST_BatchPosition3dTextMul:                          return STI_BatchPosition3dTextMul;
        case ST_InstancedPosition3dText:                         return STI_InstancedPosition3dText;
        case ST_InstancedPosition3dTextMul:                      return STI_InstancedPosition3dTextMul;
        case ST_TextInv:                                         return STI_TextInv;
        case ST_BatchTextInv:                                    return STI_BatchTextInv;
        case ST_InstancedTextInv:                                return STI_InstancedTextInv;
        case ST_Position3dTextInv:                               return STI_Position3dTextInv;
        case ST_BatchPosition3dTextInv:                          return STI_BatchPosition3dTextInv;
        case ST_InstancedPosition3dTextInv:                      return STI_InstancedPosition3dTextInv;
        case ST_TextColor:                                       return STI_TextColor;
        case ST_TextColorMul:                                    return STI_TextColorMul;
        case ST_BatchTextColor:                                  return STI_BatchTextColor;
        case ST_BatchTextColorMul:                               return STI_BatchTextColorMul;
        case ST_InstancedTextColor:                              return STI_InstancedTextColor;
        case ST_InstancedTextColorMul:                           return STI_InstancedTextColorMul;
        case ST_Position3dTextColor:                             return STI_Position3dTextColor;
        case ST_Position3dTextColorMul:                          return STI_Position3dTextColorMul;
        case ST_BatchPosition3dTextColor:                        return STI_BatchPosition3dTextColor;
        case ST_BatchPosition3dTextColorMul:                     return STI_BatchPosition3dTextColorMul;
        case ST_InstancedPosition3dTextColor:                    return STI_InstancedPosition3dTextColor;
        case ST_InstancedPosition3dTextColorMul:                 return STI_InstancedPosition3dTextColorMul;
        case ST_TextColorCxform:                                 return STI_TextColorCxform;
        case ST_TextColorCxformMul:                              return STI_TextColorCxformMul;
        case ST_BatchTextColorCxform:                            return STI_BatchTextColorCxform;
        case ST_BatchTextColorCxformMul:                         return STI_BatchTextColorCxformMul;
        case ST_InstancedTextColorCxform:                        return STI_InstancedTextColorCxform;
        case ST_InstancedTextColorCxformMul:                     return STI_InstancedTextColorCxformMul;
        case ST_Position3dTextColorCxform:                       return STI_Position3dTextColorCxform;
        case ST_Position3dTextColorCxformMul:                    return STI_Position3dTextColorCxformMul;
        case ST_BatchPosition3dTextColorCxform:                  return STI_BatchPosition3dTextColorCxform;
        case ST_BatchPosition3dTextColorCxformMul:               return STI_BatchPosition3dTextColorCxformMul;
        case ST_InstancedPosition3dTextColorCxform:              return STI_InstancedPosition3dTextColorCxform;
        case ST_InstancedPosition3dTextColorCxformMul:           return STI_InstancedPosition3dTextColorCxformMul;
        case ST_TextColorInv:                                    return STI_TextColorInv;
        case ST_BatchTextColorInv:                               return STI_BatchTextColorInv;
        case ST_InstancedTextColorInv:                           return STI_InstancedTextColorInv;
        case ST_Position3dTextColorInv:                          return STI_Position3dTextColorInv;
        case ST_BatchPosition3dTextColorInv:                     return STI_BatchPosition3dTextColorInv;
        case ST_InstancedPosition3dTextColorInv:                 return STI_InstancedPosition3dTextColorInv;
        case ST_TextColorCxformInv:                              return STI_TextColorCxformInv;
        case ST_BatchTextColorCxformInv:                         return STI_BatchTextColorCxformInv;
        case ST_InstancedTextColorCxformInv:                     return STI_InstancedTextColorCxformInv;
        case ST_Position3dTextColorCxformInv:                    return STI_Position3dTextColorCxformInv;
        case ST_BatchPosition3dTextColorCxformInv:               return STI_BatchPosition3dTextColorCxformInv;
        case ST_InstancedPosition3dTextColorCxformInv:           return STI_InstancedPosition3dTextColorCxformInv;
        case ST_YUV:                                             return STI_YUV;
        case ST_YUVMul:                                          return STI_YUVMul;
        case ST_BatchYUV:                                        return STI_BatchYUV;
        case ST_BatchYUVMul:                                     return STI_BatchYUVMul;
        case ST_InstancedYUV:                                    return STI_InstancedYUV;
        case ST_InstancedYUVMul:                                 return STI_InstancedYUVMul;
        case ST_Position3dYUV:                                   return STI_Position3dYUV;
        case ST_Position3dYUVMul:                                return STI_Position3dYUVMul;
        case ST_BatchPosition3dYUV:                              return STI_BatchPosition3dYUV;
        case ST_BatchPosition3dYUVMul:                           return STI_BatchPosition3dYUVMul;
        case ST_InstancedPosition3dYUV:                          return STI_InstancedPosition3dYUV;
        case ST_InstancedPosition3dYUVMul:                       return STI_InstancedPosition3dYUVMul;
        case ST_YUVCxform:                                       return STI_YUVCxform;
        case ST_YUVCxformMul:                                    return STI_YUVCxformMul;
        case ST_BatchYUVCxform:                                  return STI_BatchYUVCxform;
        case ST_BatchYUVCxformMul:                               return STI_BatchYUVCxformMul;
        case ST_InstancedYUVCxform:                              return STI_InstancedYUVCxform;
        case ST_InstancedYUVCxformMul:                           return STI_InstancedYUVCxformMul;
        case ST_Position3dYUVCxform:                             return STI_Position3dYUVCxform;
        case ST_Position3dYUVCxformMul:                          return STI_Position3dYUVCxformMul;
        case ST_BatchPosition3dYUVCxform:                        return STI_BatchPosition3dYUVCxform;
        case ST_BatchPosition3dYUVCxformMul:                     return STI_BatchPosition3dYUVCxformMul;
        case ST_InstancedPosition3dYUVCxform:                    return STI_InstancedPosition3dYUVCxform;
        case ST_InstancedPosition3dYUVCxformMul:                 return STI_InstancedPosition3dYUVCxformMul;
        case ST_YUVEAlpha:                                       return STI_YUVEAlpha;
        case ST_YUVEAlphaMul:                                    return STI_YUVEAlphaMul;
        case ST_BatchYUVEAlpha:                                  return STI_BatchYUVEAlpha;
        case ST_BatchYUVEAlphaMul:                               return STI_BatchYUVEAlphaMul;
        case ST_InstancedYUVEAlpha:                              return STI_InstancedYUVEAlpha;
        case ST_InstancedYUVEAlphaMul:                           return STI_InstancedYUVEAlphaMul;
        case ST_Position3dYUVEAlpha:                             return STI_Position3dYUVEAlpha;
        case ST_Position3dYUVEAlphaMul:                          return STI_Position3dYUVEAlphaMul;
        case ST_BatchPosition3dYUVEAlpha:                        return STI_BatchPosition3dYUVEAlpha;
        case ST_BatchPosition3dYUVEAlphaMul:                     return STI_BatchPosition3dYUVEAlphaMul;
        case ST_InstancedPosition3dYUVEAlpha:                    return STI_InstancedPosition3dYUVEAlpha;
        case ST_InstancedPosition3dYUVEAlphaMul:                 return STI_InstancedPosition3dYUVEAlphaMul;
        case ST_YUVCxformEAlpha:                                 return STI_YUVCxformEAlpha;
        case ST_YUVCxformEAlphaMul:                              return STI_YUVCxformEAlphaMul;
        case ST_BatchYUVCxformEAlpha:                            return STI_BatchYUVCxformEAlpha;
        case ST_BatchYUVCxformEAlphaMul:                         return STI_BatchYUVCxformEAlphaMul;
        case ST_InstancedYUVCxformEAlpha:                        return STI_InstancedYUVCxformEAlpha;
        case ST_InstancedYUVCxformEAlphaMul:                     return STI_InstancedYUVCxformEAlphaMul;
        case ST_Position3dYUVCxformEAlpha:                       return STI_Position3dYUVCxformEAlpha;
        case ST_Position3dYUVCxformEAlphaMul:                    return STI_Position3dYUVCxformEAlphaMul;
        case ST_BatchPosition3dYUVCxformEAlpha:                  return STI_BatchPosition3dYUVCxformEAlpha;
        case ST_BatchPosition3dYUVCxformEAlphaMul:               return STI_BatchPosition3dYUVCxformEAlphaMul;
        case ST_InstancedPosition3dYUVCxformEAlpha:              return STI_InstancedPosition3dYUVCxformEAlpha;
        case ST_InstancedPosition3dYUVCxformEAlphaMul:           return STI_InstancedPosition3dYUVCxformEAlphaMul;
        case ST_YUVCxformAc:                                     return STI_YUVCxformAc;
        case ST_YUVCxformAcMul:                                  return STI_YUVCxformAcMul;
        case ST_BatchYUVCxformAc:                                return STI_BatchYUVCxformAc;
        case ST_BatchYUVCxformAcMul:                             return STI_BatchYUVCxformAcMul;
        case ST_InstancedYUVCxformAc:                            return STI_InstancedYUVCxformAc;
        case ST_InstancedYUVCxformAcMul:                         return STI_InstancedYUVCxformAcMul;
        case ST_Position3dYUVCxformAc:                           return STI_Position3dYUVCxformAc;
        case ST_Position3dYUVCxformAcMul:                        return STI_Position3dYUVCxformAcMul;
        case ST_BatchPosition3dYUVCxformAc:                      return STI_BatchPosition3dYUVCxformAc;
        case ST_BatchPosition3dYUVCxformAcMul:                   return STI_BatchPosition3dYUVCxformAcMul;
        case ST_InstancedPosition3dYUVCxformAc:                  return STI_InstancedPosition3dYUVCxformAc;
        case ST_InstancedPosition3dYUVCxformAcMul:               return STI_InstancedPosition3dYUVCxformAcMul;
        case ST_YUVCxformAcEAlpha:                               return STI_YUVCxformAcEAlpha;
        case ST_YUVCxformAcEAlphaMul:                            return STI_YUVCxformAcEAlphaMul;
        case ST_BatchYUVCxformAcEAlpha:                          return STI_BatchYUVCxformAcEAlpha;
        case ST_BatchYUVCxformAcEAlphaMul:                       return STI_BatchYUVCxformAcEAlphaMul;
        case ST_InstancedYUVCxformAcEAlpha:                      return STI_InstancedYUVCxformAcEAlpha;
        case ST_InstancedYUVCxformAcEAlphaMul:                   return STI_InstancedYUVCxformAcEAlphaMul;
        case ST_Position3dYUVCxformAcEAlpha:                     return STI_Position3dYUVCxformAcEAlpha;
        case ST_Position3dYUVCxformAcEAlphaMul:                  return STI_Position3dYUVCxformAcEAlphaMul;
        case ST_BatchPosition3dYUVCxformAcEAlpha:                return STI_BatchPosition3dYUVCxformAcEAlpha;
        case ST_BatchPosition3dYUVCxformAcEAlphaMul:             return STI_BatchPosition3dYUVCxformAcEAlphaMul;
        case ST_InstancedPosition3dYUVCxformAcEAlpha:            return STI_InstancedPosition3dYUVCxformAcEAlpha;
        case ST_InstancedPosition3dYUVCxformAcEAlphaMul:         return STI_InstancedPosition3dYUVCxformAcEAlphaMul;
        case ST_YUVA:                                            return STI_YUVA;
        case ST_YUVAMul:                                         return STI_YUVAMul;
        case ST_BatchYUVA:                                       return STI_BatchYUVA;
        case ST_BatchYUVAMul:                                    return STI_BatchYUVAMul;
        case ST_InstancedYUVA:                                   return STI_InstancedYUVA;
        case ST_InstancedYUVAMul:                                return STI_InstancedYUVAMul;
        case ST_Position3dYUVA:                                  return STI_Position3dYUVA;
        case ST_Position3dYUVAMul:                               return STI_Position3dYUVAMul;
        case ST_BatchPosition3dYUVA:                             return STI_BatchPosition3dYUVA;
        case ST_BatchPosition3dYUVAMul:                          return STI_BatchPosition3dYUVAMul;
        case ST_InstancedPosition3dYUVA:                         return STI_InstancedPosition3dYUVA;
        case ST_InstancedPosition3dYUVAMul:                      return STI_InstancedPosition3dYUVAMul;
        case ST_YUVACxform:                                      return STI_YUVACxform;
        case ST_YUVACxformMul:                                   return STI_YUVACxformMul;
        case ST_BatchYUVACxform:                                 return STI_BatchYUVACxform;
        case ST_BatchYUVACxformMul:                              return STI_BatchYUVACxformMul;
        case ST_InstancedYUVACxform:                             return STI_InstancedYUVACxform;
        case ST_InstancedYUVACxformMul:                          return STI_InstancedYUVACxformMul;
        case ST_Position3dYUVACxform:                            return STI_Position3dYUVACxform;
        case ST_Position3dYUVACxformMul:                         return STI_Position3dYUVACxformMul;
        case ST_BatchPosition3dYUVACxform:                       return STI_BatchPosition3dYUVACxform;
        case ST_BatchPosition3dYUVACxformMul:                    return STI_BatchPosition3dYUVACxformMul;
        case ST_InstancedPosition3dYUVACxform:                   return STI_InstancedPosition3dYUVACxform;
        case ST_InstancedPosition3dYUVACxformMul:                return STI_InstancedPosition3dYUVACxformMul;
        case ST_YUVAEAlpha:                                      return STI_YUVAEAlpha;
        case ST_YUVAEAlphaMul:                                   return STI_YUVAEAlphaMul;
        case ST_BatchYUVAEAlpha:                                 return STI_BatchYUVAEAlpha;
        case ST_BatchYUVAEAlphaMul:                              return STI_BatchYUVAEAlphaMul;
        case ST_InstancedYUVAEAlpha:                             return STI_InstancedYUVAEAlpha;
        case ST_InstancedYUVAEAlphaMul:                          return STI_InstancedYUVAEAlphaMul;
        case ST_Position3dYUVAEAlpha:                            return STI_Position3dYUVAEAlpha;
        case ST_Position3dYUVAEAlphaMul:                         return STI_Position3dYUVAEAlphaMul;
        case ST_BatchPosition3dYUVAEAlpha:                       return STI_BatchPosition3dYUVAEAlpha;
        case ST_BatchPosition3dYUVAEAlphaMul:                    return STI_BatchPosition3dYUVAEAlphaMul;
        case ST_InstancedPosition3dYUVAEAlpha:                   return STI_InstancedPosition3dYUVAEAlpha;
        case ST_InstancedPosition3dYUVAEAlphaMul:                return STI_InstancedPosition3dYUVAEAlphaMul;
        case ST_YUVACxformEAlpha:                                return STI_YUVACxformEAlpha;
        case ST_YUVACxformEAlphaMul:                             return STI_YUVACxformEAlphaMul;
        case ST_BatchYUVACxformEAlpha:                           return STI_BatchYUVACxformEAlpha;
        case ST_BatchYUVACxformEAlphaMul:                        return STI_BatchYUVACxformEAlphaMul;
        case ST_InstancedYUVACxformEAlpha:                       return STI_InstancedYUVACxformEAlpha;
        case ST_InstancedYUVACxformEAlphaMul:                    return STI_InstancedYUVACxformEAlphaMul;
        case ST_Position3dYUVACxformEAlpha:                      return STI_Position3dYUVACxformEAlpha;
        case ST_Position3dYUVACxformEAlphaMul:                   return STI_Position3dYUVACxformEAlphaMul;
        case ST_BatchPosition3dYUVACxformEAlpha:                 return STI_BatchPosition3dYUVACxformEAlpha;
        case ST_BatchPosition3dYUVACxformEAlphaMul:              return STI_BatchPosition3dYUVACxformEAlphaMul;
        case ST_InstancedPosition3dYUVACxformEAlpha:             return STI_InstancedPosition3dYUVACxformEAlpha;
        case ST_InstancedPosition3dYUVACxformEAlphaMul:          return STI_InstancedPosition3dYUVACxformEAlphaMul;
        case ST_YUVAInv:                                         return STI_YUVAInv;
        case ST_BatchYUVAInv:                                    return STI_BatchYUVAInv;
        case ST_InstancedYUVAInv:                                return STI_InstancedYUVAInv;
        case ST_Position3dYUVAInv:                               return STI_Position3dYUVAInv;
        case ST_BatchPosition3dYUVAInv:                          return STI_BatchPosition3dYUVAInv;
        case ST_InstancedPosition3dYUVAInv:                      return STI_InstancedPosition3dYUVAInv;
        case ST_YUVACxformInv:                                   return STI_YUVACxformInv;
        case ST_BatchYUVACxformInv:                              return STI_BatchYUVACxformInv;
        case ST_InstancedYUVACxformInv:                          return STI_InstancedYUVACxformInv;
        case ST_Position3dYUVACxformInv:                         return STI_Position3dYUVACxformInv;
        case ST_BatchPosition3dYUVACxformInv:                    return STI_BatchPosition3dYUVACxformInv;
        case ST_InstancedPosition3dYUVACxformInv:                return STI_InstancedPosition3dYUVACxformInv;
        case ST_YUVAEAlphaInv:                                   return STI_YUVAEAlphaInv;
        case ST_BatchYUVAEAlphaInv:                              return STI_BatchYUVAEAlphaInv;
        case ST_InstancedYUVAEAlphaInv:                          return STI_InstancedYUVAEAlphaInv;
        case ST_Position3dYUVAEAlphaInv:                         return STI_Position3dYUVAEAlphaInv;
        case ST_BatchPosition3dYUVAEAlphaInv:                    return STI_BatchPosition3dYUVAEAlphaInv;
        case ST_InstancedPosition3dYUVAEAlphaInv:                return STI_InstancedPosition3dYUVAEAlphaInv;
        case ST_YUVACxformEAlphaInv:                             return STI_YUVACxformEAlphaInv;
        case ST_BatchYUVACxformEAlphaInv:                        return STI_BatchYUVACxformEAlphaInv;
        case ST_InstancedYUVACxformEAlphaInv:                    return STI_InstancedYUVACxformEAlphaInv;
        case ST_Position3dYUVACxformEAlphaInv:                   return STI_Position3dYUVACxformEAlphaInv;
        case ST_BatchPosition3dYUVACxformEAlphaInv:              return STI_BatchPosition3dYUVACxformEAlphaInv;
        case ST_InstancedPosition3dYUVACxformEAlphaInv:          return STI_InstancedPosition3dYUVACxformEAlphaInv;
        case ST_YUVACxformAc:                                    return STI_YUVACxformAc;
        case ST_YUVACxformAcMul:                                 return STI_YUVACxformAcMul;
        case ST_BatchYUVACxformAc:                               return STI_BatchYUVACxformAc;
        case ST_BatchYUVACxformAcMul:                            return STI_BatchYUVACxformAcMul;
        case ST_InstancedYUVACxformAc:                           return STI_InstancedYUVACxformAc;
        case ST_InstancedYUVACxformAcMul:                        return STI_InstancedYUVACxformAcMul;
        case ST_Position3dYUVACxformAc:                          return STI_Position3dYUVACxformAc;
        case ST_Position3dYUVACxformAcMul:                       return STI_Position3dYUVACxformAcMul;
        case ST_BatchPosition3dYUVACxformAc:                     return STI_BatchPosition3dYUVACxformAc;
        case ST_BatchPosition3dYUVACxformAcMul:                  return STI_BatchPosition3dYUVACxformAcMul;
        case ST_InstancedPosition3dYUVACxformAc:                 return STI_InstancedPosition3dYUVACxformAc;
        case ST_InstancedPosition3dYUVACxformAcMul:              return STI_InstancedPosition3dYUVACxformAcMul;
        case ST_YUVACxformAcEAlpha:                              return STI_YUVACxformAcEAlpha;
        case ST_YUVACxformAcEAlphaMul:                           return STI_YUVACxformAcEAlphaMul;
        case ST_BatchYUVACxformAcEAlpha:                         return STI_BatchYUVACxformAcEAlpha;
        case ST_BatchYUVACxformAcEAlphaMul:                      return STI_BatchYUVACxformAcEAlphaMul;
        case ST_InstancedYUVACxformAcEAlpha:                     return STI_InstancedYUVACxformAcEAlpha;
        case ST_InstancedYUVACxformAcEAlphaMul:                  return STI_InstancedYUVACxformAcEAlphaMul;
        case ST_Position3dYUVACxformAcEAlpha:                    return STI_Position3dYUVACxformAcEAlpha;
        case ST_Position3dYUVACxformAcEAlphaMul:                 return STI_Position3dYUVACxformAcEAlphaMul;
        case ST_BatchPosition3dYUVACxformAcEAlpha:               return STI_BatchPosition3dYUVACxformAcEAlpha;
        case ST_BatchPosition3dYUVACxformAcEAlphaMul:            return STI_BatchPosition3dYUVACxformAcEAlphaMul;
        case ST_InstancedPosition3dYUVACxformAcEAlpha:           return STI_InstancedPosition3dYUVACxformAcEAlpha;
        case ST_InstancedPosition3dYUVACxformAcEAlphaMul:        return STI_InstancedPosition3dYUVACxformAcEAlphaMul;
        case ST_YUVACxformAcInv:                                 return STI_YUVACxformAcInv;
        case ST_BatchYUVACxformAcInv:                            return STI_BatchYUVACxformAcInv;
        case ST_InstancedYUVACxformAcInv:                        return STI_InstancedYUVACxformAcInv;
        case ST_Position3dYUVACxformAcInv:                       return STI_Position3dYUVACxformAcInv;
        case ST_BatchPosition3dYUVACxformAcInv:                  return STI_BatchPosition3dYUVACxformAcInv;
        case ST_InstancedPosition3dYUVACxformAcInv:              return STI_InstancedPosition3dYUVACxformAcInv;
        case ST_YUVACxformAcEAlphaInv:                           return STI_YUVACxformAcEAlphaInv;
        case ST_BatchYUVACxformAcEAlphaInv:                      return STI_BatchYUVACxformAcEAlphaInv;
        case ST_InstancedYUVACxformAcEAlphaInv:                  return STI_InstancedYUVACxformAcEAlphaInv;
        case ST_Position3dYUVACxformAcEAlphaInv:                 return STI_Position3dYUVACxformAcEAlphaInv;
        case ST_BatchPosition3dYUVACxformAcEAlphaInv:            return STI_BatchPosition3dYUVACxformAcEAlphaInv;
        case ST_InstancedPosition3dYUVACxformAcEAlphaInv:        return STI_InstancedPosition3dYUVACxformAcEAlphaInv;
        case ST_TexTGCMatrixAc:                                  return STI_TexTGCMatrixAc;
        case ST_TexTGCMatrixAcMul:                               return STI_TexTGCMatrixAcMul;
        case ST_TexTGCMatrixAcEAlpha:                            return STI_TexTGCMatrixAcEAlpha;
        case ST_TexTGCMatrixAcEAlphaMul:                         return STI_TexTGCMatrixAcEAlphaMul;
        case ST_Box1Blur:                                        return STI_Box1Blur;
        case ST_Box1BlurMul:                                     return STI_Box1BlurMul;
        case ST_Box2Blur:                                        return STI_Box2Blur;
        case ST_Box2BlurMul:                                     return STI_Box2BlurMul;
        case ST_Box2Shadow:                                      return STI_Box2Shadow;
        case ST_Box2ShadowMul:                                   return STI_Box2ShadowMul;
        case ST_Box2ShadowKnockout:                              return STI_Box2ShadowKnockout;
        case ST_Box2ShadowKnockoutMul:                           return STI_Box2ShadowKnockoutMul;
        case ST_Box2Shadowonly:                                  return STI_Box2Shadowonly;
        case ST_Box2ShadowonlyMul:                               return STI_Box2ShadowonlyMul;
        case ST_Box2InnerShadow:                                 return STI_Box2InnerShadow;
        case ST_Box2InnerShadowMul:                              return STI_Box2InnerShadowMul;
        case ST_Box2InnerShadowKnockout:                         return STI_Box2InnerShadowKnockout;
        case ST_Box2InnerShadowKnockoutMul:                      return STI_Box2InnerShadowKnockoutMul;
        case ST_Box2ShadowonlyHighlight:                         return STI_Box2ShadowonlyHighlight;
        case ST_Box2ShadowonlyHighlightMul:                      return STI_Box2ShadowonlyHighlightMul;
        case ST_Box2FullShadowHighlight:                         return STI_Box2FullShadowHighlight;
        case ST_Box2FullShadowHighlightMul:                      return STI_Box2FullShadowHighlightMul;
        case ST_Box2InnerShadowHighlight:                        return STI_Box2InnerShadowHighlight;
        case ST_Box2InnerShadowHighlightMul:                     return STI_Box2InnerShadowHighlightMul;
        case ST_Box2InnerShadowHighlightKnockout:                return STI_Box2InnerShadowHighlightKnockout;
        case ST_Box2InnerShadowHighlightKnockoutMul:             return STI_Box2InnerShadowHighlightKnockoutMul;
        case ST_Box2ShadowHighlight:                             return STI_Box2ShadowHighlight;
        case ST_Box2ShadowHighlightMul:                          return STI_Box2ShadowHighlightMul;
        case ST_Box2ShadowHighlightKnockout:                     return STI_Box2ShadowHighlightKnockout;
        case ST_Box2ShadowHighlightKnockoutMul:                  return STI_Box2ShadowHighlightKnockoutMul;
        case ST_DrawableCopyPixels:                              return STI_DrawableCopyPixels;
        case ST_DrawableCopyPixelsNoDestAlpha:                   return STI_DrawableCopyPixelsNoDestAlpha;
        case ST_DrawableCopyPixelsMergeAlpha:                    return STI_DrawableCopyPixelsMergeAlpha;
        case ST_DrawableCopyPixelsAlpha:                         return STI_DrawableCopyPixelsAlpha;
        case ST_DrawableCopyPixelsAlphaNoDestAlpha:              return STI_DrawableCopyPixelsAlphaNoDestAlpha;
        case ST_DrawableCopyPixelsAlphaMergeAlpha:               return STI_DrawableCopyPixelsAlphaMergeAlpha;
        case ST_DrawableMerge:                                   return STI_DrawableMerge;
        case ST_DrawableCompare:                                 return STI_DrawableCompare;
        case ST_DrawablePaletteMap:                              return STI_DrawablePaletteMap;
        default: SF_DEBUG_ASSERT1(1, "Invalid ShaderType (%d)", type);
    }
    return STI_None;
}

struct ShaderIndexEntry
{
    VertexShaderDesc::ShaderIndex VertexIndex;
    FragShaderDesc::ShaderIndex FragIndex;
};

static const ShaderIndexEntry ShaderIndexingData[ShaderDesc::STI_Count][ShaderDesc::ShaderVersion_Count] = 
{
    { // ST_None
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
    },
    { // ST_TexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTG,
            FragShaderDesc::FSI_D3D9SM20_FTexTG
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTG,
            FragShaderDesc::FSI_D3D9SM30_FTexTG
        },
    },
    { // ST_TexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGMul
        },
    },
    { // ST_BatchTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTG,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTG
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTG,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTG
        },
    },
    { // ST_BatchTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGMul
        },
    },
    { // ST_InstancedTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTG,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTG
        },
    },
    { // ST_InstancedTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGMul
        },
    },
    { // ST_Position3dTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTG,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTG
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTG,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTG
        },
    },
    { // ST_Position3dTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGMul
        },
    },
    { // ST_BatchPosition3dTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTG,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTG
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTG,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTG
        },
    },
    { // ST_BatchPosition3dTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGMul
        },
    },
    { // ST_InstancedPosition3dTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTG,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTG
        },
    },
    { // ST_InstancedPosition3dTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGMul
        },
    },
    { // ST_TexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxform,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxform
        },
    },
    { // ST_TexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformMul
        },
    },
    { // ST_BatchTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxform
        },
    },
    { // ST_BatchTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformMul
        },
    },
    { // ST_InstancedTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxform
        },
    },
    { // ST_InstancedTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformMul
        },
    },
    { // ST_Position3dTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxform,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxform
        },
    },
    { // ST_Position3dTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformMul
        },
    },
    { // ST_BatchPosition3dTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxform
        },
    },
    { // ST_BatchPosition3dTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformMul
        },
    },
    { // ST_InstancedPosition3dTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxform
        },
    },
    { // ST_InstancedPosition3dTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformMul
        },
    },
    { // ST_TexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGEAlpha
        },
    },
    { // ST_TexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGEAlphaMul
        },
    },
    { // ST_BatchTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGEAlpha
        },
    },
    { // ST_BatchTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGEAlphaMul
        },
    },
    { // ST_InstancedTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGEAlpha
        },
    },
    { // ST_InstancedTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGEAlphaMul
        },
    },
    { // ST_Position3dTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGEAlpha
        },
    },
    { // ST_Position3dTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGEAlphaMul
        },
    },
    { // ST_TexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformEAlpha
        },
    },
    { // ST_TexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformEAlphaMul
        },
    },
    { // ST_BatchTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformEAlpha
        },
    },
    { // ST_BatchTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformEAlphaMul
        },
    },
    { // ST_InstancedTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformEAlpha
        },
    },
    { // ST_InstancedTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformEAlphaMul
        },
    },
    { // ST_Position3dTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformEAlpha
        },
    },
    { // ST_Position3dTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformEAlphaMul
        },
    },
    { // ST_TexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGInv
        },
    },
    { // ST_BatchTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGInv
        },
    },
    { // ST_InstancedTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGInv
        },
    },
    { // ST_Position3dTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGInv
        },
    },
    { // ST_BatchPosition3dTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGInv
        },
    },
    { // ST_InstancedPosition3dTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGInv
        },
    },
    { // ST_TexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformInv
        },
    },
    { // ST_BatchTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformInv
        },
    },
    { // ST_InstancedTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformInv
        },
    },
    { // ST_Position3dTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformInv
        },
    },
    { // ST_BatchPosition3dTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformInv
        },
    },
    { // ST_InstancedPosition3dTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformInv
        },
    },
    { // ST_TexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGEAlphaInv
        },
    },
    { // ST_BatchTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGEAlphaInv
        },
    },
    { // ST_InstancedTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGEAlphaInv
        },
    },
    { // ST_Position3dTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGEAlphaInv
        },
    },
    { // ST_TexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformEAlphaInv
        },
    },
    { // ST_BatchTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformEAlphaInv
        },
    },
    { // ST_InstancedTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformEAlphaInv
        },
    },
    { // ST_Position3dTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformEAlphaInv
        },
    },
    { // ST_TexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformAc
        },
    },
    { // ST_TexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformAcMul
        },
    },
    { // ST_BatchTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformAc
        },
    },
    { // ST_BatchTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformAcMul
        },
    },
    { // ST_InstancedTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformAc
        },
    },
    { // ST_InstancedTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformAcMul
        },
    },
    { // ST_Position3dTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformAc
        },
    },
    { // ST_Position3dTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformAcMul
        },
    },
    { // ST_BatchPosition3dTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformAc
        },
    },
    { // ST_BatchPosition3dTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformAcMul
        },
    },
    { // ST_InstancedPosition3dTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformAc
        },
    },
    { // ST_InstancedPosition3dTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformAcMul
        },
    },
    { // ST_TexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformAcEAlpha
        },
    },
    { // ST_TexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformAcEAlphaMul
        },
    },
    { // ST_BatchTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformAcEAlpha
        },
    },
    { // ST_BatchTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformAcEAlphaMul
        },
    },
    { // ST_InstancedTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformAcEAlpha
        },
    },
    { // ST_InstancedTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformAcEAlphaMul
        },
    },
    { // ST_Position3dTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformAcEAlpha
        },
    },
    { // ST_Position3dTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformAcEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformAcEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformAcEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformAcEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformAcEAlphaMul
        },
    },
    { // ST_TexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformAcInv
        },
    },
    { // ST_BatchTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformAcInv
        },
    },
    { // ST_InstancedTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformAcInv
        },
    },
    { // ST_Position3dTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformAcInv
        },
    },
    { // ST_BatchPosition3dTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformAcInv
        },
    },
    { // ST_InstancedPosition3dTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformAcInv
        },
    },
    { // ST_TexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCxformAcEAlphaInv
        },
    },
    { // ST_BatchTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGCxformAcEAlphaInv
        },
    },
    { // ST_InstancedTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGCxformAcEAlphaInv
        },
    },
    { // ST_Position3dTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGCxformAcEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGCxformAcEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGCxformAcEAlphaInv
        },
    },
    { // ST_Vertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertex,
            FragShaderDesc::FSI_D3D9SM20_FVertex
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertex,
            FragShaderDesc::FSI_D3D9SM30_FVertex
        },
    },
    { // ST_VertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexMul,
            FragShaderDesc::FSI_D3D9SM20_FVertexMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FVertexMul
        },
    },
    { // ST_BatchVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertex,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertex
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertex,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertex
        },
    },
    { // ST_BatchVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexMul
        },
    },
    { // ST_InstancedVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertex,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertex
        },
    },
    { // ST_InstancedVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexMul
        },
    },
    { // ST_Position3dVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertex,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertex
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertex,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertex
        },
    },
    { // ST_Position3dVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexMul
        },
    },
    { // ST_BatchPosition3dVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertex,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertex
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertex,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertex
        },
    },
    { // ST_BatchPosition3dVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexMul
        },
    },
    { // ST_InstancedPosition3dVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertex,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertex
        },
    },
    { // ST_InstancedPosition3dVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexMul
        },
    },
    { // ST_VertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxform,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxform
        },
    },
    { // ST_VertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformMul
        },
    },
    { // ST_BatchVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxform
        },
    },
    { // ST_BatchVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformMul
        },
    },
    { // ST_InstancedVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxform
        },
    },
    { // ST_InstancedVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformMul
        },
    },
    { // ST_Position3dVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxform,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxform
        },
    },
    { // ST_Position3dVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformMul
        },
    },
    { // ST_BatchPosition3dVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxform
        },
    },
    { // ST_BatchPosition3dVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformMul
        },
    },
    { // ST_InstancedPosition3dVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxform
        },
    },
    { // ST_InstancedPosition3dVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformMul
        },
    },
    { // ST_VertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FVertexEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FVertexEAlpha
        },
    },
    { // ST_VertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FVertexEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FVertexEAlphaMul
        },
    },
    { // ST_BatchVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexEAlpha
        },
    },
    { // ST_BatchVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexEAlphaMul
        },
    },
    { // ST_InstancedVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexEAlpha
        },
    },
    { // ST_InstancedVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexEAlphaMul
        },
    },
    { // ST_Position3dVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexEAlpha
        },
    },
    { // ST_Position3dVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexEAlphaMul
        },
    },
    { // ST_BatchPosition3dVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexEAlpha
        },
    },
    { // ST_BatchPosition3dVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexEAlphaMul
        },
    },
    { // ST_InstancedPosition3dVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexEAlpha
        },
    },
    { // ST_InstancedPosition3dVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexEAlphaMul
        },
    },
    { // ST_VertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformEAlpha
        },
    },
    { // ST_VertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformEAlphaMul
        },
    },
    { // ST_BatchVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformEAlpha
        },
    },
    { // ST_BatchVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformEAlphaMul
        },
    },
    { // ST_InstancedVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformEAlpha
        },
    },
    { // ST_InstancedVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformEAlphaMul
        },
    },
    { // ST_Position3dVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformEAlpha
        },
    },
    { // ST_Position3dVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformEAlphaMul
        },
    },
    { // ST_BatchPosition3dVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformEAlpha
        },
    },
    { // ST_BatchPosition3dVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformEAlphaMul
        },
    },
    { // ST_InstancedPosition3dVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformEAlpha
        },
    },
    { // ST_InstancedPosition3dVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformEAlphaMul
        },
    },
    { // ST_VertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexInv,
            FragShaderDesc::FSI_D3D9SM20_FVertexInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FVertexInv
        },
    },
    { // ST_BatchVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexInv
        },
    },
    { // ST_InstancedVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexInv
        },
    },
    { // ST_Position3dVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexInv
        },
    },
    { // ST_BatchPosition3dVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexInv
        },
    },
    { // ST_InstancedPosition3dVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexInv
        },
    },
    { // ST_VertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformInv
        },
    },
    { // ST_BatchVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformInv
        },
    },
    { // ST_InstancedVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformInv
        },
    },
    { // ST_Position3dVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformInv
        },
    },
    { // ST_BatchPosition3dVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformInv
        },
    },
    { // ST_InstancedPosition3dVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformInv
        },
    },
    { // ST_VertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FVertexEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FVertexEAlphaInv
        },
    },
    { // ST_BatchVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexEAlphaInv
        },
    },
    { // ST_InstancedVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexEAlphaInv
        },
    },
    { // ST_Position3dVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexEAlphaInv
        },
    },
    { // ST_BatchPosition3dVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexEAlphaInv
        },
    },
    { // ST_InstancedPosition3dVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexEAlphaInv
        },
    },
    { // ST_VertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformEAlphaInv
        },
    },
    { // ST_BatchVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformEAlphaInv
        },
    },
    { // ST_InstancedVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformEAlphaInv
        },
    },
    { // ST_Position3dVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformEAlphaInv
        },
    },
    { // ST_BatchPosition3dVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformEAlphaInv
        },
    },
    { // ST_InstancedPosition3dVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformEAlphaInv
        },
    },
    { // ST_VertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformAc
        },
    },
    { // ST_VertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformAcMul
        },
    },
    { // ST_BatchVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformAc
        },
    },
    { // ST_BatchVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformAcMul
        },
    },
    { // ST_InstancedVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformAc
        },
    },
    { // ST_InstancedVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformAcMul
        },
    },
    { // ST_Position3dVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformAc
        },
    },
    { // ST_Position3dVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformAcMul
        },
    },
    { // ST_BatchPosition3dVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformAc
        },
    },
    { // ST_BatchPosition3dVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformAcMul
        },
    },
    { // ST_InstancedPosition3dVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformAc
        },
    },
    { // ST_InstancedPosition3dVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformAcMul
        },
    },
    { // ST_VertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformAcEAlpha
        },
    },
    { // ST_VertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformAcEAlphaMul
        },
    },
    { // ST_BatchVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformAcEAlpha
        },
    },
    { // ST_BatchVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformAcEAlphaMul
        },
    },
    { // ST_InstancedVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformAcEAlpha
        },
    },
    { // ST_InstancedVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformAcEAlphaMul
        },
    },
    { // ST_Position3dVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformAcEAlpha
        },
    },
    { // ST_Position3dVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformAcEAlphaMul
        },
    },
    { // ST_BatchPosition3dVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformAcEAlpha
        },
    },
    { // ST_BatchPosition3dVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformAcEAlphaMul
        },
    },
    { // ST_InstancedPosition3dVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformAcEAlpha
        },
    },
    { // ST_InstancedPosition3dVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformAcEAlphaMul
        },
    },
    { // ST_VertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformAcInv
        },
    },
    { // ST_BatchVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformAcInv
        },
    },
    { // ST_InstancedVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformAcInv
        },
    },
    { // ST_Position3dVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformAcInv
        },
    },
    { // ST_BatchPosition3dVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformAcInv
        },
    },
    { // ST_InstancedPosition3dVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformAcInv
        },
    },
    { // ST_VertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FVertexCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FVertexCxformAcEAlphaInv
        },
    },
    { // ST_BatchVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchVertexCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchVertexCxformAcEAlphaInv
        },
    },
    { // ST_InstancedVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedVertexCxformAcEAlphaInv
        },
    },
    { // ST_Position3dVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dVertexCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dVertexCxformAcEAlphaInv
        },
    },
    { // ST_BatchPosition3dVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dVertexCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dVertexCxformAcEAlphaInv
        },
    },
    { // ST_InstancedPosition3dVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dVertexCxformAcEAlphaInv
        },
    },
    { // ST_TexTGTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTG
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTG
        },
    },
    { // ST_TexTGTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGMul
        },
    },
    { // ST_BatchTexTGTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTG
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTG
        },
    },
    { // ST_BatchTexTGTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGMul
        },
    },
    { // ST_InstancedTexTGTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTG
        },
    },
    { // ST_InstancedTexTGTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGMul
        },
    },
    { // ST_Position3dTexTGTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTG
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTG
        },
    },
    { // ST_Position3dTexTGTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGMul
        },
    },
    { // ST_BatchPosition3dTexTGTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTG
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTG
        },
    },
    { // ST_BatchPosition3dTexTGTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGMul
        },
    },
    { // ST_InstancedPosition3dTexTGTexTG
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTG,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTG
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGMul
        },
    },
    { // ST_TexTGTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxform
        },
    },
    { // ST_TexTGTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformMul
        },
    },
    { // ST_BatchTexTGTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxform
        },
    },
    { // ST_BatchTexTGTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformMul
        },
    },
    { // ST_InstancedTexTGTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxform
        },
    },
    { // ST_InstancedTexTGTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformMul
        },
    },
    { // ST_Position3dTexTGTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxform
        },
    },
    { // ST_Position3dTexTGTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformMul
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxform
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformMul
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxform
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformMul
        },
    },
    { // ST_TexTGTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGEAlpha
        },
    },
    { // ST_TexTGTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGEAlphaMul
        },
    },
    { // ST_BatchTexTGTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGEAlpha
        },
    },
    { // ST_BatchTexTGTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGEAlphaMul
        },
    },
    { // ST_InstancedTexTGTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGEAlpha
        },
    },
    { // ST_InstancedTexTGTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGEAlphaMul
        },
    },
    { // ST_Position3dTexTGTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGEAlpha
        },
    },
    { // ST_Position3dTexTGTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGEAlphaMul
        },
    },
    { // ST_TexTGTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformEAlpha
        },
    },
    { // ST_TexTGTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformEAlphaMul
        },
    },
    { // ST_BatchTexTGTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformEAlpha
        },
    },
    { // ST_BatchTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformEAlphaMul
        },
    },
    { // ST_InstancedTexTGTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformEAlpha
        },
    },
    { // ST_InstancedTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformEAlphaMul
        },
    },
    { // ST_Position3dTexTGTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformEAlpha
        },
    },
    { // ST_Position3dTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformEAlphaMul
        },
    },
    { // ST_TexTGTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGInv
        },
    },
    { // ST_BatchTexTGTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGInv
        },
    },
    { // ST_InstancedTexTGTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGInv
        },
    },
    { // ST_Position3dTexTGTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGInv
        },
    },
    { // ST_BatchPosition3dTexTGTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGInv
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGInv
        },
    },
    { // ST_TexTGTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformInv
        },
    },
    { // ST_BatchTexTGTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformInv
        },
    },
    { // ST_InstancedTexTGTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformInv
        },
    },
    { // ST_Position3dTexTGTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformInv
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformInv
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformInv
        },
    },
    { // ST_TexTGTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGEAlphaInv
        },
    },
    { // ST_BatchTexTGTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGEAlphaInv
        },
    },
    { // ST_InstancedTexTGTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGEAlphaInv
        },
    },
    { // ST_Position3dTexTGTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGEAlphaInv
        },
    },
    { // ST_TexTGTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformEAlphaInv
        },
    },
    { // ST_BatchTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformEAlphaInv
        },
    },
    { // ST_InstancedTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformEAlphaInv
        },
    },
    { // ST_Position3dTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformEAlphaInv
        },
    },
    { // ST_TexTGTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformAc
        },
    },
    { // ST_TexTGTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformAcMul
        },
    },
    { // ST_BatchTexTGTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformAc
        },
    },
    { // ST_BatchTexTGTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformAcMul
        },
    },
    { // ST_InstancedTexTGTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformAc
        },
    },
    { // ST_InstancedTexTGTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformAcMul
        },
    },
    { // ST_Position3dTexTGTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformAc
        },
    },
    { // ST_Position3dTexTGTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformAcMul
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformAc
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformAcMul
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformAc
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformAcMul
        },
    },
    { // ST_TexTGTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformAcEAlpha
        },
    },
    { // ST_TexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul
        },
    },
    { // ST_BatchTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformAcEAlpha
        },
    },
    { // ST_BatchTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformAcEAlphaMul
        },
    },
    { // ST_InstancedTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlpha
        },
    },
    { // ST_InstancedTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlphaMul
        },
    },
    { // ST_Position3dTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformAcEAlpha
        },
    },
    { // ST_Position3dTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformAcEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformAcEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformAcEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformAcEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformAcEAlphaMul
        },
    },
    { // ST_TexTGTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformAcInv
        },
    },
    { // ST_BatchTexTGTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformAcInv
        },
    },
    { // ST_InstancedTexTGTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformAcInv
        },
    },
    { // ST_Position3dTexTGTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformAcInv
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformAcInv
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformAcInv
        },
    },
    { // ST_TexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGTexTGCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv
        },
    },
    { // ST_BatchTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGTexTGCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGTexTGCxformAcEAlphaInv
        },
    },
    { // ST_InstancedTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlphaInv
        },
    },
    { // ST_Position3dTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGTexTGCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGTexTGCxformAcEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGTexTGCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGTexTGCxformAcEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGTexTGCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGTexTGCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGTexTGCxformAcEAlphaInv
        },
    },
    { // ST_TexTGVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertex,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertex
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertex,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertex
        },
    },
    { // ST_TexTGVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexMul
        },
    },
    { // ST_BatchTexTGVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertex,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertex
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertex,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertex
        },
    },
    { // ST_BatchTexTGVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexMul
        },
    },
    { // ST_InstancedTexTGVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertex,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertex
        },
    },
    { // ST_InstancedTexTGVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexMul
        },
    },
    { // ST_Position3dTexTGVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertex,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertex
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertex,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertex
        },
    },
    { // ST_Position3dTexTGVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexMul
        },
    },
    { // ST_BatchPosition3dTexTGVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertex,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertex
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertex,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertex
        },
    },
    { // ST_BatchPosition3dTexTGVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexMul
        },
    },
    { // ST_InstancedPosition3dTexTGVertex
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertex,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertex
        },
    },
    { // ST_InstancedPosition3dTexTGVertexMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexMul
        },
    },
    { // ST_TexTGVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxform
        },
    },
    { // ST_TexTGVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformMul
        },
    },
    { // ST_BatchTexTGVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxform
        },
    },
    { // ST_BatchTexTGVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformMul
        },
    },
    { // ST_InstancedTexTGVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxform
        },
    },
    { // ST_InstancedTexTGVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformMul
        },
    },
    { // ST_Position3dTexTGVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxform
        },
    },
    { // ST_Position3dTexTGVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformMul
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxform
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformMul
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxform
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformMul
        },
    },
    { // ST_TexTGVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexEAlpha
        },
    },
    { // ST_TexTGVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexEAlphaMul
        },
    },
    { // ST_BatchTexTGVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexEAlpha
        },
    },
    { // ST_BatchTexTGVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexEAlphaMul
        },
    },
    { // ST_InstancedTexTGVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexEAlpha
        },
    },
    { // ST_InstancedTexTGVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexEAlphaMul
        },
    },
    { // ST_Position3dTexTGVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexEAlpha
        },
    },
    { // ST_Position3dTexTGVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGVertexEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGVertexEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexEAlphaMul
        },
    },
    { // ST_TexTGVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformEAlpha
        },
    },
    { // ST_TexTGVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformEAlphaMul
        },
    },
    { // ST_BatchTexTGVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformEAlpha
        },
    },
    { // ST_BatchTexTGVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformEAlphaMul
        },
    },
    { // ST_InstancedTexTGVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformEAlpha
        },
    },
    { // ST_InstancedTexTGVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformEAlphaMul
        },
    },
    { // ST_Position3dTexTGVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformEAlpha
        },
    },
    { // ST_Position3dTexTGVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformEAlphaMul
        },
    },
    { // ST_TexTGVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexInv
        },
    },
    { // ST_BatchTexTGVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexInv
        },
    },
    { // ST_InstancedTexTGVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexInv
        },
    },
    { // ST_Position3dTexTGVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexInv
        },
    },
    { // ST_BatchPosition3dTexTGVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexInv
        },
    },
    { // ST_InstancedPosition3dTexTGVertexInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexInv
        },
    },
    { // ST_TexTGVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformInv
        },
    },
    { // ST_BatchTexTGVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformInv
        },
    },
    { // ST_InstancedTexTGVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformInv
        },
    },
    { // ST_Position3dTexTGVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformInv
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformInv
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformInv
        },
    },
    { // ST_TexTGVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexEAlphaInv
        },
    },
    { // ST_BatchTexTGVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexEAlphaInv
        },
    },
    { // ST_InstancedTexTGVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexEAlphaInv
        },
    },
    { // ST_Position3dTexTGVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGVertexEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexEAlphaInv
        },
    },
    { // ST_TexTGVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformEAlphaInv
        },
    },
    { // ST_BatchTexTGVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformEAlphaInv
        },
    },
    { // ST_InstancedTexTGVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformEAlphaInv
        },
    },
    { // ST_Position3dTexTGVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformEAlphaInv
        },
    },
    { // ST_TexTGVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformAc
        },
    },
    { // ST_TexTGVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformAcMul
        },
    },
    { // ST_BatchTexTGVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformAc
        },
    },
    { // ST_BatchTexTGVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformAcMul
        },
    },
    { // ST_InstancedTexTGVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformAc
        },
    },
    { // ST_InstancedTexTGVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformAcMul
        },
    },
    { // ST_Position3dTexTGVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformAc
        },
    },
    { // ST_Position3dTexTGVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformAcMul
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformAc
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformAcMul
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformAc
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformAcMul
        },
    },
    { // ST_TexTGVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformAcEAlpha
        },
    },
    { // ST_TexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformAcEAlphaMul
        },
    },
    { // ST_BatchTexTGVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformAcEAlpha
        },
    },
    { // ST_BatchTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformAcEAlphaMul
        },
    },
    { // ST_InstancedTexTGVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformAcEAlpha
        },
    },
    { // ST_InstancedTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformAcEAlphaMul
        },
    },
    { // ST_Position3dTexTGVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformAcEAlpha
        },
    },
    { // ST_Position3dTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformAcEAlphaMul
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformAcEAlpha
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformAcEAlphaMul
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformAcEAlpha
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformAcEAlphaMul
        },
    },
    { // ST_TexTGVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformAcInv
        },
    },
    { // ST_BatchTexTGVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformAcInv
        },
    },
    { // ST_InstancedTexTGVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformAcInv
        },
    },
    { // ST_Position3dTexTGVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformAcInv
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformAcInv
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformAcInv
        },
    },
    { // ST_TexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FTexTGVertexCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FTexTGVertexCxformAcEAlphaInv
        },
    },
    { // ST_BatchTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTexTGVertexCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTexTGVertexCxformAcEAlphaInv
        },
    },
    { // ST_InstancedTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTexTGVertexCxformAcEAlphaInv
        },
    },
    { // ST_Position3dTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTexTGVertexCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTexTGVertexCxformAcEAlphaInv
        },
    },
    { // ST_BatchPosition3dTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTexTGVertexCxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTexTGVertexCxformAcEAlphaInv
        },
    },
    { // ST_InstancedPosition3dTexTGVertexCxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTexTGVertexCxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTexTGVertexCxformAcEAlphaInv
        },
    },
    { // ST_Solid
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VSolid,
            FragShaderDesc::FSI_D3D9SM20_FSolid
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VSolid,
            FragShaderDesc::FSI_D3D9SM30_FSolid
        },
    },
    { // ST_SolidMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VSolidMul,
            FragShaderDesc::FSI_D3D9SM20_FSolidMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VSolidMul,
            FragShaderDesc::FSI_D3D9SM30_FSolidMul
        },
    },
    { // ST_BatchSolid
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchSolid,
            FragShaderDesc::FSI_D3D9SM20_FBatchSolid
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchSolid,
            FragShaderDesc::FSI_D3D9SM30_FBatchSolid
        },
    },
    { // ST_BatchSolidMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchSolidMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchSolidMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchSolidMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchSolidMul
        },
    },
    { // ST_InstancedSolid
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedSolid,
            FragShaderDesc::FSI_D3D9SM30_FInstancedSolid
        },
    },
    { // ST_InstancedSolidMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedSolidMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedSolidMul
        },
    },
    { // ST_Position3dSolid
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dSolid,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dSolid
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dSolid,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dSolid
        },
    },
    { // ST_Position3dSolidMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dSolidMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dSolidMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dSolidMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dSolidMul
        },
    },
    { // ST_BatchPosition3dSolid
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dSolid,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dSolid
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dSolid,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dSolid
        },
    },
    { // ST_BatchPosition3dSolidMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dSolidMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dSolidMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dSolidMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dSolidMul
        },
    },
    { // ST_InstancedPosition3dSolid
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dSolid,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dSolid
        },
    },
    { // ST_InstancedPosition3dSolidMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dSolidMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dSolidMul
        },
    },
    { // ST_SolidInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VSolidInv,
            FragShaderDesc::FSI_D3D9SM20_FSolidInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VSolidInv,
            FragShaderDesc::FSI_D3D9SM30_FSolidInv
        },
    },
    { // ST_BatchSolidInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchSolidInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchSolidInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchSolidInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchSolidInv
        },
    },
    { // ST_InstancedSolidInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedSolidInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedSolidInv
        },
    },
    { // ST_Position3dSolidInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dSolidInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dSolidInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dSolidInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dSolidInv
        },
    },
    { // ST_BatchPosition3dSolidInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dSolidInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dSolidInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dSolidInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dSolidInv
        },
    },
    { // ST_InstancedPosition3dSolidInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dSolidInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dSolidInv
        },
    },
    { // ST_Text
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VText,
            FragShaderDesc::FSI_D3D9SM20_FText
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VText,
            FragShaderDesc::FSI_D3D9SM30_FText
        },
    },
    { // ST_TextMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTextMul,
            FragShaderDesc::FSI_D3D9SM20_FTextMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTextMul,
            FragShaderDesc::FSI_D3D9SM30_FTextMul
        },
    },
    { // ST_BatchText
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchText,
            FragShaderDesc::FSI_D3D9SM20_FBatchText
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchText,
            FragShaderDesc::FSI_D3D9SM30_FBatchText
        },
    },
    { // ST_BatchTextMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTextMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTextMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTextMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTextMul
        },
    },
    { // ST_InstancedText
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedText,
            FragShaderDesc::FSI_D3D9SM30_FInstancedText
        },
    },
    { // ST_InstancedTextMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTextMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTextMul
        },
    },
    { // ST_Position3dText
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dText,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dText
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dText,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dText
        },
    },
    { // ST_Position3dTextMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTextMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTextMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTextMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTextMul
        },
    },
    { // ST_BatchPosition3dText
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dText,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dText
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dText,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dText
        },
    },
    { // ST_BatchPosition3dTextMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTextMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTextMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTextMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTextMul
        },
    },
    { // ST_InstancedPosition3dText
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dText,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dText
        },
    },
    { // ST_InstancedPosition3dTextMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTextMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTextMul
        },
    },
    { // ST_TextInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTextInv,
            FragShaderDesc::FSI_D3D9SM20_FTextInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTextInv,
            FragShaderDesc::FSI_D3D9SM30_FTextInv
        },
    },
    { // ST_BatchTextInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTextInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTextInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTextInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTextInv
        },
    },
    { // ST_InstancedTextInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTextInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTextInv
        },
    },
    { // ST_Position3dTextInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTextInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTextInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTextInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTextInv
        },
    },
    { // ST_BatchPosition3dTextInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTextInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTextInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTextInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTextInv
        },
    },
    { // ST_InstancedPosition3dTextInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTextInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTextInv
        },
    },
    { // ST_TextColor
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTextColor,
            FragShaderDesc::FSI_D3D9SM20_FTextColor
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTextColor,
            FragShaderDesc::FSI_D3D9SM30_FTextColor
        },
    },
    { // ST_TextColorMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTextColorMul,
            FragShaderDesc::FSI_D3D9SM20_FTextColorMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTextColorMul,
            FragShaderDesc::FSI_D3D9SM30_FTextColorMul
        },
    },
    { // ST_BatchTextColor
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTextColor,
            FragShaderDesc::FSI_D3D9SM20_FBatchTextColor
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTextColor,
            FragShaderDesc::FSI_D3D9SM30_FBatchTextColor
        },
    },
    { // ST_BatchTextColorMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTextColorMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTextColorMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTextColorMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTextColorMul
        },
    },
    { // ST_InstancedTextColor
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTextColor,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTextColor
        },
    },
    { // ST_InstancedTextColorMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTextColorMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTextColorMul
        },
    },
    { // ST_Position3dTextColor
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTextColor,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTextColor
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTextColor,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTextColor
        },
    },
    { // ST_Position3dTextColorMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTextColorMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTextColorMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTextColorMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTextColorMul
        },
    },
    { // ST_BatchPosition3dTextColor
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTextColor,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTextColor
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTextColor,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTextColor
        },
    },
    { // ST_BatchPosition3dTextColorMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTextColorMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTextColorMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTextColorMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTextColorMul
        },
    },
    { // ST_InstancedPosition3dTextColor
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTextColor,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTextColor
        },
    },
    { // ST_InstancedPosition3dTextColorMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTextColorMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTextColorMul
        },
    },
    { // ST_TextColorCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTextColorCxform,
            FragShaderDesc::FSI_D3D9SM20_FTextColorCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTextColorCxform,
            FragShaderDesc::FSI_D3D9SM30_FTextColorCxform
        },
    },
    { // ST_TextColorCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FTextColorCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FTextColorCxformMul
        },
    },
    { // ST_BatchTextColorCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTextColorCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchTextColorCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTextColorCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchTextColorCxform
        },
    },
    { // ST_BatchTextColorCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchTextColorCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchTextColorCxformMul
        },
    },
    { // ST_InstancedTextColorCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTextColorCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTextColorCxform
        },
    },
    { // ST_InstancedTextColorCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTextColorCxformMul
        },
    },
    { // ST_Position3dTextColorCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTextColorCxform,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTextColorCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTextColorCxform,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTextColorCxform
        },
    },
    { // ST_Position3dTextColorCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTextColorCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTextColorCxformMul
        },
    },
    { // ST_BatchPosition3dTextColorCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTextColorCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTextColorCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTextColorCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTextColorCxform
        },
    },
    { // ST_BatchPosition3dTextColorCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTextColorCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTextColorCxformMul
        },
    },
    { // ST_InstancedPosition3dTextColorCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTextColorCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTextColorCxform
        },
    },
    { // ST_InstancedPosition3dTextColorCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTextColorCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTextColorCxformMul
        },
    },
    { // ST_TextColorInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTextColorInv,
            FragShaderDesc::FSI_D3D9SM20_FTextColorInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTextColorInv,
            FragShaderDesc::FSI_D3D9SM30_FTextColorInv
        },
    },
    { // ST_BatchTextColorInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTextColorInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTextColorInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTextColorInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTextColorInv
        },
    },
    { // ST_InstancedTextColorInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTextColorInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTextColorInv
        },
    },
    { // ST_Position3dTextColorInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTextColorInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTextColorInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTextColorInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTextColorInv
        },
    },
    { // ST_BatchPosition3dTextColorInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTextColorInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTextColorInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTextColorInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTextColorInv
        },
    },
    { // ST_InstancedPosition3dTextColorInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTextColorInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTextColorInv
        },
    },
    { // ST_TextColorCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FTextColorCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FTextColorCxformInv
        },
    },
    { // ST_BatchTextColorCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchTextColorCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchTextColorCxformInv
        },
    },
    { // ST_InstancedTextColorCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedTextColorCxformInv
        },
    },
    { // ST_Position3dTextColorCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dTextColorCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dTextColorCxformInv
        },
    },
    { // ST_BatchPosition3dTextColorCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dTextColorCxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dTextColorCxformInv
        },
    },
    { // ST_InstancedPosition3dTextColorCxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dTextColorCxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dTextColorCxformInv
        },
    },
    { // ST_YUV
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUV,
            FragShaderDesc::FSI_D3D9SM20_FYUV
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUV,
            FragShaderDesc::FSI_D3D9SM30_FYUV
        },
    },
    { // ST_YUVMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVMul
        },
    },
    { // ST_BatchYUV
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUV,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUV
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUV,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUV
        },
    },
    { // ST_BatchYUVMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVMul
        },
    },
    { // ST_InstancedYUV
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUV,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUV
        },
    },
    { // ST_InstancedYUVMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVMul
        },
    },
    { // ST_Position3dYUV
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUV,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUV
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUV,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUV
        },
    },
    { // ST_Position3dYUVMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVMul
        },
    },
    { // ST_BatchPosition3dYUV
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUV,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUV
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUV,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUV
        },
    },
    { // ST_BatchPosition3dYUVMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVMul
        },
    },
    { // ST_InstancedPosition3dYUV
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUV,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUV
        },
    },
    { // ST_InstancedPosition3dYUVMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVMul
        },
    },
    { // ST_YUVCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVCxform,
            FragShaderDesc::FSI_D3D9SM20_FYUVCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVCxform,
            FragShaderDesc::FSI_D3D9SM30_FYUVCxform
        },
    },
    { // ST_YUVCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVCxformMul
        },
    },
    { // ST_BatchYUVCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVCxform
        },
    },
    { // ST_BatchYUVCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVCxformMul
        },
    },
    { // ST_InstancedYUVCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVCxform
        },
    },
    { // ST_InstancedYUVCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVCxformMul
        },
    },
    { // ST_Position3dYUVCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVCxform,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVCxform,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVCxform
        },
    },
    { // ST_Position3dYUVCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVCxformMul
        },
    },
    { // ST_BatchPosition3dYUVCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVCxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVCxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVCxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVCxform
        },
    },
    { // ST_BatchPosition3dYUVCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVCxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVCxformMul
        },
    },
    { // ST_InstancedPosition3dYUVCxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVCxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVCxform
        },
    },
    { // ST_InstancedPosition3dYUVCxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVCxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVCxformMul
        },
    },
    { // ST_YUVEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FYUVEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FYUVEAlpha
        },
    },
    { // ST_YUVEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVEAlphaMul
        },
    },
    { // ST_BatchYUVEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVEAlpha
        },
    },
    { // ST_BatchYUVEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVEAlphaMul
        },
    },
    { // ST_InstancedYUVEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVEAlpha
        },
    },
    { // ST_InstancedYUVEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVEAlphaMul
        },
    },
    { // ST_Position3dYUVEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVEAlpha
        },
    },
    { // ST_Position3dYUVEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVEAlphaMul
        },
    },
    { // ST_BatchPosition3dYUVEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVEAlpha
        },
    },
    { // ST_BatchPosition3dYUVEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVEAlphaMul
        },
    },
    { // ST_InstancedPosition3dYUVEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVEAlpha
        },
    },
    { // ST_InstancedPosition3dYUVEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVEAlphaMul
        },
    },
    { // ST_YUVCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FYUVCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FYUVCxformEAlpha
        },
    },
    { // ST_YUVCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVCxformEAlphaMul
        },
    },
    { // ST_BatchYUVCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVCxformEAlpha
        },
    },
    { // ST_BatchYUVCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVCxformEAlphaMul
        },
    },
    { // ST_InstancedYUVCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVCxformEAlpha
        },
    },
    { // ST_InstancedYUVCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVCxformEAlphaMul
        },
    },
    { // ST_Position3dYUVCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVCxformEAlpha
        },
    },
    { // ST_Position3dYUVCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVCxformEAlphaMul
        },
    },
    { // ST_BatchPosition3dYUVCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVCxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVCxformEAlpha
        },
    },
    { // ST_BatchPosition3dYUVCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVCxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVCxformEAlphaMul
        },
    },
    { // ST_InstancedPosition3dYUVCxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVCxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVCxformEAlpha
        },
    },
    { // ST_InstancedPosition3dYUVCxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVCxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVCxformEAlphaMul
        },
    },
    { // ST_YUVCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FYUVCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FYUVCxformAc
        },
    },
    { // ST_YUVCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVCxformAcMul
        },
    },
    { // ST_BatchYUVCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVCxformAc
        },
    },
    { // ST_BatchYUVCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVCxformAcMul
        },
    },
    { // ST_InstancedYUVCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVCxformAc
        },
    },
    { // ST_InstancedYUVCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVCxformAcMul
        },
    },
    { // ST_Position3dYUVCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVCxformAc
        },
    },
    { // ST_Position3dYUVCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVCxformAcMul
        },
    },
    { // ST_BatchPosition3dYUVCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVCxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVCxformAc
        },
    },
    { // ST_BatchPosition3dYUVCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVCxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVCxformAcMul
        },
    },
    { // ST_InstancedPosition3dYUVCxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVCxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVCxformAc
        },
    },
    { // ST_InstancedPosition3dYUVCxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVCxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVCxformAcMul
        },
    },
    { // ST_YUVCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FYUVCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FYUVCxformAcEAlpha
        },
    },
    { // ST_YUVCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVCxformAcEAlphaMul
        },
    },
    { // ST_BatchYUVCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVCxformAcEAlpha
        },
    },
    { // ST_BatchYUVCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVCxformAcEAlphaMul
        },
    },
    { // ST_InstancedYUVCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVCxformAcEAlpha
        },
    },
    { // ST_InstancedYUVCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVCxformAcEAlphaMul
        },
    },
    { // ST_Position3dYUVCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVCxformAcEAlpha
        },
    },
    { // ST_Position3dYUVCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVCxformAcEAlphaMul
        },
    },
    { // ST_BatchPosition3dYUVCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVCxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVCxformAcEAlpha
        },
    },
    { // ST_BatchPosition3dYUVCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVCxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVCxformAcEAlphaMul
        },
    },
    { // ST_InstancedPosition3dYUVCxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVCxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVCxformAcEAlpha
        },
    },
    { // ST_InstancedPosition3dYUVCxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVCxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVCxformAcEAlphaMul
        },
    },
    { // ST_YUVA
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVA,
            FragShaderDesc::FSI_D3D9SM20_FYUVA
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVA,
            FragShaderDesc::FSI_D3D9SM30_FYUVA
        },
    },
    { // ST_YUVAMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVAMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVAMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVAMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVAMul
        },
    },
    { // ST_BatchYUVA
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVA,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVA
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVA,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVA
        },
    },
    { // ST_BatchYUVAMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVAMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVAMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVAMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVAMul
        },
    },
    { // ST_InstancedYUVA
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVA,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVA
        },
    },
    { // ST_InstancedYUVAMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVAMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVAMul
        },
    },
    { // ST_Position3dYUVA
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVA,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVA
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVA,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVA
        },
    },
    { // ST_Position3dYUVAMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVAMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVAMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVAMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVAMul
        },
    },
    { // ST_BatchPosition3dYUVA
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVA,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVA
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVA,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVA
        },
    },
    { // ST_BatchPosition3dYUVAMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVAMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVAMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVAMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVAMul
        },
    },
    { // ST_InstancedPosition3dYUVA
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVA,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVA
        },
    },
    { // ST_InstancedPosition3dYUVAMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVAMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVAMul
        },
    },
    { // ST_YUVACxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxform,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxform,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxform
        },
    },
    { // ST_YUVACxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformMul
        },
    },
    { // ST_BatchYUVACxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxform
        },
    },
    { // ST_BatchYUVACxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformMul
        },
    },
    { // ST_InstancedYUVACxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxform
        },
    },
    { // ST_InstancedYUVACxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformMul
        },
    },
    { // ST_Position3dYUVACxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxform,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxform,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxform
        },
    },
    { // ST_Position3dYUVACxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformMul
        },
    },
    { // ST_BatchPosition3dYUVACxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxform,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxform
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxform,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxform
        },
    },
    { // ST_BatchPosition3dYUVACxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformMul
        },
    },
    { // ST_InstancedPosition3dYUVACxform
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxform,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxform
        },
    },
    { // ST_InstancedPosition3dYUVACxformMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformMul
        },
    },
    { // ST_YUVAEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FYUVAEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FYUVAEAlpha
        },
    },
    { // ST_YUVAEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVAEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVAEAlphaMul
        },
    },
    { // ST_BatchYUVAEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVAEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVAEAlpha
        },
    },
    { // ST_BatchYUVAEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVAEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVAEAlphaMul
        },
    },
    { // ST_InstancedYUVAEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVAEAlpha
        },
    },
    { // ST_InstancedYUVAEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVAEAlphaMul
        },
    },
    { // ST_Position3dYUVAEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVAEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVAEAlpha
        },
    },
    { // ST_Position3dYUVAEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVAEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVAEAlphaMul
        },
    },
    { // ST_BatchPosition3dYUVAEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVAEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVAEAlpha
        },
    },
    { // ST_BatchPosition3dYUVAEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVAEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVAEAlphaMul
        },
    },
    { // ST_InstancedPosition3dYUVAEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVAEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVAEAlpha
        },
    },
    { // ST_InstancedPosition3dYUVAEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVAEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVAEAlphaMul
        },
    },
    { // ST_YUVACxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformEAlpha
        },
    },
    { // ST_YUVACxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformEAlphaMul
        },
    },
    { // ST_BatchYUVACxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformEAlpha
        },
    },
    { // ST_BatchYUVACxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformEAlphaMul
        },
    },
    { // ST_InstancedYUVACxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformEAlpha
        },
    },
    { // ST_InstancedYUVACxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformEAlphaMul
        },
    },
    { // ST_Position3dYUVACxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformEAlpha
        },
    },
    { // ST_Position3dYUVACxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformEAlphaMul
        },
    },
    { // ST_BatchPosition3dYUVACxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformEAlpha
        },
    },
    { // ST_BatchPosition3dYUVACxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformEAlphaMul
        },
    },
    { // ST_InstancedPosition3dYUVACxformEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformEAlpha
        },
    },
    { // ST_InstancedPosition3dYUVACxformEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformEAlphaMul
        },
    },
    { // ST_YUVAInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVAInv,
            FragShaderDesc::FSI_D3D9SM20_FYUVAInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVAInv,
            FragShaderDesc::FSI_D3D9SM30_FYUVAInv
        },
    },
    { // ST_BatchYUVAInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVAInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVAInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVAInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVAInv
        },
    },
    { // ST_InstancedYUVAInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVAInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVAInv
        },
    },
    { // ST_Position3dYUVAInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVAInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVAInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVAInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVAInv
        },
    },
    { // ST_BatchPosition3dYUVAInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVAInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVAInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVAInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVAInv
        },
    },
    { // ST_InstancedPosition3dYUVAInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVAInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVAInv
        },
    },
    { // ST_YUVACxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformInv
        },
    },
    { // ST_BatchYUVACxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformInv
        },
    },
    { // ST_InstancedYUVACxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformInv
        },
    },
    { // ST_Position3dYUVACxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformInv
        },
    },
    { // ST_BatchPosition3dYUVACxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformInv
        },
    },
    { // ST_InstancedPosition3dYUVACxformInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformInv
        },
    },
    { // ST_YUVAEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FYUVAEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FYUVAEAlphaInv
        },
    },
    { // ST_BatchYUVAEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVAEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVAEAlphaInv
        },
    },
    { // ST_InstancedYUVAEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVAEAlphaInv
        },
    },
    { // ST_Position3dYUVAEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVAEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVAEAlphaInv
        },
    },
    { // ST_BatchPosition3dYUVAEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVAEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVAEAlphaInv
        },
    },
    { // ST_InstancedPosition3dYUVAEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVAEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVAEAlphaInv
        },
    },
    { // ST_YUVACxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformEAlphaInv
        },
    },
    { // ST_BatchYUVACxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformEAlphaInv
        },
    },
    { // ST_InstancedYUVACxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformEAlphaInv
        },
    },
    { // ST_Position3dYUVACxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformEAlphaInv
        },
    },
    { // ST_BatchPosition3dYUVACxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformEAlphaInv
        },
    },
    { // ST_InstancedPosition3dYUVACxformEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformEAlphaInv
        },
    },
    { // ST_YUVACxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformAc
        },
    },
    { // ST_YUVACxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformAcMul
        },
    },
    { // ST_BatchYUVACxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformAc
        },
    },
    { // ST_BatchYUVACxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformAcMul
        },
    },
    { // ST_InstancedYUVACxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformAc
        },
    },
    { // ST_InstancedYUVACxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformAcMul
        },
    },
    { // ST_Position3dYUVACxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformAc
        },
    },
    { // ST_Position3dYUVACxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformAcMul
        },
    },
    { // ST_BatchPosition3dYUVACxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformAc
        },
    },
    { // ST_BatchPosition3dYUVACxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformAcMul
        },
    },
    { // ST_InstancedPosition3dYUVACxformAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformAc,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformAc
        },
    },
    { // ST_InstancedPosition3dYUVACxformAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformAcMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformAcMul
        },
    },
    { // ST_YUVACxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformAcEAlpha
        },
    },
    { // ST_YUVACxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformAcEAlphaMul
        },
    },
    { // ST_BatchYUVACxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformAcEAlpha
        },
    },
    { // ST_BatchYUVACxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformAcEAlphaMul
        },
    },
    { // ST_InstancedYUVACxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformAcEAlpha
        },
    },
    { // ST_InstancedYUVACxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformAcEAlphaMul
        },
    },
    { // ST_Position3dYUVACxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformAcEAlpha
        },
    },
    { // ST_Position3dYUVACxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformAcEAlphaMul
        },
    },
    { // ST_BatchPosition3dYUVACxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformAcEAlpha
        },
    },
    { // ST_BatchPosition3dYUVACxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformAcEAlphaMul
        },
    },
    { // ST_InstancedPosition3dYUVACxformAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformAcEAlpha
        },
    },
    { // ST_InstancedPosition3dYUVACxformAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformAcEAlphaMul
        },
    },
    { // ST_YUVACxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformAcInv
        },
    },
    { // ST_BatchYUVACxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformAcInv
        },
    },
    { // ST_InstancedYUVACxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformAcInv
        },
    },
    { // ST_Position3dYUVACxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformAcInv
        },
    },
    { // ST_BatchPosition3dYUVACxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformAcInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformAcInv
        },
    },
    { // ST_InstancedPosition3dYUVACxformAcInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformAcInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformAcInv
        },
    },
    { // ST_YUVACxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FYUVACxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FYUVACxformAcEAlphaInv
        },
    },
    { // ST_BatchYUVACxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchYUVACxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchYUVACxformAcEAlphaInv
        },
    },
    { // ST_InstancedYUVACxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedYUVACxformAcEAlphaInv
        },
    },
    { // ST_Position3dYUVACxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VPosition3dYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FPosition3dYUVACxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VPosition3dYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FPosition3dYUVACxformAcEAlphaInv
        },
    },
    { // ST_BatchPosition3dYUVACxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VBatchPosition3dYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM20_FBatchPosition3dYUVACxformAcEAlphaInv
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBatchPosition3dYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FBatchPosition3dYUVACxformAcEAlphaInv
        },
    },
    { // ST_InstancedPosition3dYUVACxformAcEAlphaInv
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VInstancedPosition3dYUVACxformAcEAlphaInv,
            FragShaderDesc::FSI_D3D9SM30_FInstancedPosition3dYUVACxformAcEAlphaInv
        },
    },
    { // ST_TexTGCMatrixAc
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCMatrixAc,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCMatrixAc
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCMatrixAc,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCMatrixAc
        },
    },
    { // ST_TexTGCMatrixAcMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCMatrixAcMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCMatrixAcMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCMatrixAcMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCMatrixAcMul
        },
    },
    { // ST_TexTGCMatrixAcEAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCMatrixAcEAlpha,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCMatrixAcEAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCMatrixAcEAlpha,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCMatrixAcEAlpha
        },
    },
    { // ST_TexTGCMatrixAcEAlphaMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VTexTGCMatrixAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM20_FTexTGCMatrixAcEAlphaMul
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VTexTGCMatrixAcEAlphaMul,
            FragShaderDesc::FSI_D3D9SM30_FTexTGCMatrixAcEAlphaMul
        },
    },
    { // ST_Box1Blur
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox1Blur,
            FragShaderDesc::FSI_D3D9SM30_FBox1Blur
        },
    },
    { // ST_Box1BlurMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox1BlurMul,
            FragShaderDesc::FSI_D3D9SM30_FBox1BlurMul
        },
    },
    { // ST_Box2Blur
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2Blur,
            FragShaderDesc::FSI_D3D9SM30_FBox2Blur
        },
    },
    { // ST_Box2BlurMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2BlurMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2BlurMul
        },
    },
    { // ST_Box2Shadow
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2Shadow,
            FragShaderDesc::FSI_D3D9SM30_FBox2Shadow
        },
    },
    { // ST_Box2ShadowMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowMul
        },
    },
    { // ST_Box2ShadowKnockout
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowKnockout,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowKnockout
        },
    },
    { // ST_Box2ShadowKnockoutMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowKnockoutMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowKnockoutMul
        },
    },
    { // ST_Box2Shadowonly
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2Shadowonly,
            FragShaderDesc::FSI_D3D9SM30_FBox2Shadowonly
        },
    },
    { // ST_Box2ShadowonlyMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowonlyMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowonlyMul
        },
    },
    { // ST_Box2InnerShadow
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2InnerShadow,
            FragShaderDesc::FSI_D3D9SM30_FBox2InnerShadow
        },
    },
    { // ST_Box2InnerShadowMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2InnerShadowMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2InnerShadowMul
        },
    },
    { // ST_Box2InnerShadowKnockout
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2InnerShadowKnockout,
            FragShaderDesc::FSI_D3D9SM30_FBox2InnerShadowKnockout
        },
    },
    { // ST_Box2InnerShadowKnockoutMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2InnerShadowKnockoutMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2InnerShadowKnockoutMul
        },
    },
    { // ST_Box2ShadowonlyHighlight
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowonlyHighlight,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowonlyHighlight
        },
    },
    { // ST_Box2ShadowonlyHighlightMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowonlyHighlightMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowonlyHighlightMul
        },
    },
    { // ST_Box2FullShadowHighlight
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2FullShadowHighlight,
            FragShaderDesc::FSI_D3D9SM30_FBox2FullShadowHighlight
        },
    },
    { // ST_Box2FullShadowHighlightMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2FullShadowHighlightMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2FullShadowHighlightMul
        },
    },
    { // ST_Box2InnerShadowHighlight
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2InnerShadowHighlight,
            FragShaderDesc::FSI_D3D9SM30_FBox2InnerShadowHighlight
        },
    },
    { // ST_Box2InnerShadowHighlightMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2InnerShadowHighlightMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2InnerShadowHighlightMul
        },
    },
    { // ST_Box2InnerShadowHighlightKnockout
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2InnerShadowHighlightKnockout,
            FragShaderDesc::FSI_D3D9SM30_FBox2InnerShadowHighlightKnockout
        },
    },
    { // ST_Box2InnerShadowHighlightKnockoutMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2InnerShadowHighlightKnockoutMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2InnerShadowHighlightKnockoutMul
        },
    },
    { // ST_Box2ShadowHighlight
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowHighlight,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowHighlight
        },
    },
    { // ST_Box2ShadowHighlightMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowHighlightMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowHighlightMul
        },
    },
    { // ST_Box2ShadowHighlightKnockout
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowHighlightKnockout,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowHighlightKnockout
        },
    },
    { // ST_Box2ShadowHighlightKnockoutMul
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_None,
            FragShaderDesc::FSI_None
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VBox2ShadowHighlightKnockoutMul,
            FragShaderDesc::FSI_D3D9SM30_FBox2ShadowHighlightKnockoutMul
        },
    },
    { // ST_DrawableCopyPixels
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawableCopyPixels,
            FragShaderDesc::FSI_D3D9SM20_FDrawableCopyPixels
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawableCopyPixels,
            FragShaderDesc::FSI_D3D9SM30_FDrawableCopyPixels
        },
    },
    { // ST_DrawableCopyPixelsNoDestAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawableCopyPixelsNoDestAlpha,
            FragShaderDesc::FSI_D3D9SM20_FDrawableCopyPixelsNoDestAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawableCopyPixelsNoDestAlpha,
            FragShaderDesc::FSI_D3D9SM30_FDrawableCopyPixelsNoDestAlpha
        },
    },
    { // ST_DrawableCopyPixelsMergeAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawableCopyPixelsMergeAlpha,
            FragShaderDesc::FSI_D3D9SM20_FDrawableCopyPixelsMergeAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawableCopyPixelsMergeAlpha,
            FragShaderDesc::FSI_D3D9SM30_FDrawableCopyPixelsMergeAlpha
        },
    },
    { // ST_DrawableCopyPixelsAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawableCopyPixelsAlpha,
            FragShaderDesc::FSI_D3D9SM20_FDrawableCopyPixelsAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawableCopyPixelsAlpha,
            FragShaderDesc::FSI_D3D9SM30_FDrawableCopyPixelsAlpha
        },
    },
    { // ST_DrawableCopyPixelsAlphaNoDestAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawableCopyPixelsAlphaNoDestAlpha,
            FragShaderDesc::FSI_D3D9SM20_FDrawableCopyPixelsAlphaNoDestAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawableCopyPixelsAlphaNoDestAlpha,
            FragShaderDesc::FSI_D3D9SM30_FDrawableCopyPixelsAlphaNoDestAlpha
        },
    },
    { // ST_DrawableCopyPixelsAlphaMergeAlpha
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawableCopyPixelsAlphaMergeAlpha,
            FragShaderDesc::FSI_D3D9SM20_FDrawableCopyPixelsAlphaMergeAlpha
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawableCopyPixelsAlphaMergeAlpha,
            FragShaderDesc::FSI_D3D9SM30_FDrawableCopyPixelsAlphaMergeAlpha
        },
    },
    { // ST_DrawableMerge
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawableMerge,
            FragShaderDesc::FSI_D3D9SM20_FDrawableMerge
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawableMerge,
            FragShaderDesc::FSI_D3D9SM30_FDrawableMerge
        },
    },
    { // ST_DrawableCompare
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawableCompare,
            FragShaderDesc::FSI_D3D9SM20_FDrawableCompare
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawableCompare,
            FragShaderDesc::FSI_D3D9SM30_FDrawableCompare
        },
    },
    { // ST_DrawablePaletteMap
        { // ShaderVersion_D3D9SM20
            VertexShaderDesc::VSI_D3D9SM20_VDrawablePaletteMap,
            FragShaderDesc::FSI_D3D9SM20_FDrawablePaletteMap
        },
        { // ShaderVersion_D3D9SM30
            VertexShaderDesc::VSI_D3D9SM30_VDrawablePaletteMap,
            FragShaderDesc::FSI_D3D9SM30_FDrawablePaletteMap
        },
    },
};
extern const BYTE pBinary_D3D9SM20_VTexTG[];
extern const BYTE pBinary_D3D9SM20_VTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_VTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VVertex[];
extern const BYTE pBinary_D3D9SM20_VVertexEAlpha[];
extern const BYTE pBinary_D3D9SM20_VVertexCxform[];
extern const BYTE pBinary_D3D9SM20_VVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VTexTGTexTG[];
extern const BYTE pBinary_D3D9SM20_VTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VTexTGVertex[];
extern const BYTE pBinary_D3D9SM20_VTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VPosition3dVertex[];
extern const BYTE pBinary_D3D9SM20_VPosition3dVertexEAlpha[];
extern const BYTE pBinary_D3D9SM20_VPosition3dVertexCxform[];
extern const BYTE pBinary_D3D9SM20_VPosition3dVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGTexTG[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGVertex[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTG[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchVertex[];
extern const BYTE pBinary_D3D9SM20_VBatchVertexEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchVertexCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGTexTG[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGVertex[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dVertex[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dVertexEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dVertexCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGTexTG[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGVertex[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM20_VSolid[];
extern const BYTE pBinary_D3D9SM20_VText[];
extern const BYTE pBinary_D3D9SM20_VTextColor[];
extern const BYTE pBinary_D3D9SM20_VTextColorCxform[];
extern const BYTE pBinary_D3D9SM20_VPosition3dSolid[];
extern const BYTE pBinary_D3D9SM20_VPosition3dText[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTextColor[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTextColorCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchSolid[];
extern const BYTE pBinary_D3D9SM20_VBatchText[];
extern const BYTE pBinary_D3D9SM20_VBatchTextColor[];
extern const BYTE pBinary_D3D9SM20_VBatchTextColorCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dSolid[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dText[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTextColor[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTextColorCxform[];
extern const BYTE pBinary_D3D9SM20_VTexTG[];
extern const BYTE pBinary_D3D9SM20_VTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_VTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTG[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_VBatchPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_VDrawableCopyPixels[];
extern const BYTE pBinary_D3D9SM20_VDrawableCopyPixelsAlpha[];

UniformVar Uniforms_2402505279[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {2, 8, 4, 8, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

BatchVar BatchUniforms_6184305142[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_Count, -1, 0},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_2315053743[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {4, 16, 4, 8, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3962131312[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4660718830[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2666373766[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {2, 8, 4, 16, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3200611686[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {4, 16, 4, 16, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_6395323925[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {4, 16, 4, 8, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4496992820[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {6, 24, 4, 8, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4480697281[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3978807109[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2590809056[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {4, 16, 4, 16, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_6124559549[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 0, 4, 4, 4, 0 },
    /* cxmul*/      {1, 4, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {2, 8, 16, 16, 16, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {6, 24, 4, 16, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4025051752[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ {0, 0, 4, 384, 4, 4 },
};

BatchVar BatchUniforms_3478612506[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 0, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 2, 2},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_4705861522[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ {0, 0, 4, 576, 4, 6 },
};

BatchVar BatchUniforms_6232566636[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 2, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 4, 2},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_2647200773[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ {0, 0, 4, 192, 4, 2 },
};

BatchVar BatchUniforms_2375807902[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 0, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

BatchVar BatchUniforms_5188561028[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 2, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

BatchVar BatchUniforms_3422124174[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 0, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 2, 4},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_2393121201[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ {0, 0, 4, 768, 4, 8 },
};

BatchVar BatchUniforms_4076936984[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfuniforms, 2, 2},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 4, 4},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_3965965598[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{0, 0, 16, 384, 16, 1 },
    /* vfuniforms*/ {96, 384, 4, 192, 4, 2 },
};

BatchVar BatchUniforms_2889592211[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 0, 2},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_3892239162[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{0, 0, 16, 384, 16, 1 },
    /* vfuniforms*/ {96, 384, 4, 384, 4, 4 },
};

BatchVar BatchUniforms_4272630808[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 2, 2},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_5257564337[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{0, 0, 16, 384, 16, 1 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

BatchVar BatchUniforms_4952385266[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

BatchVar BatchUniforms_5592595562[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_Count, -1, 0},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

BatchVar BatchUniforms_4749803573[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_Count, -1, 0},
    /* cxmul*/       {Uniform::SU_Count, -1, 0},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 0, 4},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_6117254102[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{0, 0, 16, 384, 16, 1 },
    /* vfuniforms*/ {96, 384, 4, 576, 4, 6 },
};

BatchVar BatchUniforms_3953522647[Uniform::SU_Count] = 
{
    /* cxadd*/       {Uniform::SU_vfuniforms, 0, 1},
    /* cxmul*/       {Uniform::SU_vfuniforms, 1, 1},
    /* cxmul1*/      {Uniform::SU_Count, -1, 0},
    /* fsize*/       {Uniform::SU_Count, -1, 0},
    /* mvp*/         {Uniform::SU_vfmuniforms, 0, 1},
    /* offset*/      {Uniform::SU_Count, -1, 0},
    /* scolor*/      {Uniform::SU_Count, -1, 0},
    /* scolor2*/     {Uniform::SU_Count, -1, 0},
    /* srctex*/      {Uniform::SU_Count, -1, 0},
    /* srctexscale*/ {Uniform::SU_Count, -1, 0},
    /* tex*/         {Uniform::SU_Count, -1, 0},
    /* texgen*/      {Uniform::SU_vfuniforms, 2, 4},
    /* texscale*/    {Uniform::SU_Count, -1, 0},
    /* vfmuniforms*/ {Uniform::SU_Count, -1, 0},
    /* vfuniforms*/  {Uniform::SU_Count, -1, 0},
};

UniformVar Uniforms_2267270599[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        {0, 0, 4, 8, 4, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     {2, 8, 4, 24, 4, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTexTG,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTexTGEAlpha,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTexTGCxform,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VVertex = 
{
    /* Type */          ShaderDesc::ST_Vertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VVertex,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VVertexEAlpha,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VVertexCxform = 
{
    /* Type */          ShaderDesc::ST_VertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VVertexCxform,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VVertexCxformEAlpha,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTexTGTexTG,
    /* Uniforms */      Uniforms_2666373766,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTexTGTexTGCxform,
    /* Uniforms */      Uniforms_3200611686,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_TexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTexTGVertex,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTexTGVertexCxform,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTexTG = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTG,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dVertex = 
{
    /* Type */          ShaderDesc::ST_Position3dVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dVertex,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dVertexEAlpha,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dVertexCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dVertexCxform,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dVertexCxformEAlpha,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGTexTG,
    /* Uniforms */      Uniforms_2590809056,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGTexTGCxform,
    /* Uniforms */      Uniforms_6124559549,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGVertex,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGVertexCxform,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTG,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchVertex = 
{
    /* Type */          ShaderDesc::ST_BatchVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchVertex,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchVertexEAlpha,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchVertexCxform,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchVertexCxformEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGTexTG,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_3422124174,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGTexTGCxform,
    /* Uniforms */      Uniforms_2393121201,
    /* BatchUniforms */ BatchUniforms_4076936984,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGVertex,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGVertexCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTG,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dVertex = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dVertex,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dVertexEAlpha,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dVertexCxform,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dVertexCxformEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGTexTG,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4749803573,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGTexTGCxform,
    /* Uniforms */      Uniforms_6117254102,
    /* BatchUniforms */ BatchUniforms_3953522647,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGVertex,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGVertexCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VSolid = 
{
    /* Type */          ShaderDesc::ST_Solid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VSolid,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VText = 
{
    /* Type */          ShaderDesc::ST_Text,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VText,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTextColor = 
{
    /* Type */          ShaderDesc::ST_TextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTextColor,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_TextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VTextColorCxform,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dSolid = 
{
    /* Type */          ShaderDesc::ST_Position3dSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dSolid,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dText = 
{
    /* Type */          ShaderDesc::ST_Position3dText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dText,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTextColor = 
{
    /* Type */          ShaderDesc::ST_Position3dTextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTextColor,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTextColorCxform,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchSolid = 
{
    /* Type */          ShaderDesc::ST_BatchSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchSolid,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchText = 
{
    /* Type */          ShaderDesc::ST_BatchText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchText,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTextColor = 
{
    /* Type */          ShaderDesc::ST_BatchTextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTextColor,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchTextColorCxform,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dSolid = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dSolid,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dText = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dText,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTextColor = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTextColor,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTextColorCxform,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VYUV = 
{
    /* Type */          ShaderDesc::ST_YUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VTexTG,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VTexTGEAlpha,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VYUVCxform = 
{
    /* Type */          ShaderDesc::ST_YUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VTexTGCxform,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dYUV = 
{
    /* Type */          ShaderDesc::ST_Position3dYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTG,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dYUVCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VPosition3dYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchYUV = 
{
    /* Type */          ShaderDesc::ST_BatchYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTG,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchYUVCxform = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VBatchTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dYUV = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTG,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dYUVCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VBatchPosition3dYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_VBatchPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VDrawableCopyPixels = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixels,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VDrawableCopyPixels,
    /* Uniforms */      Uniforms_2666373766,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM20_VDrawableCopyPixelsAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_VDrawableCopyPixelsAlpha,
    /* Uniforms */      Uniforms_2267270599,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

extern const BYTE pBinary_D3D9SM30_VTexTG[];
extern const BYTE pBinary_D3D9SM30_VTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VVertex[];
extern const BYTE pBinary_D3D9SM30_VVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_VVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_VTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_VTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VPosition3dVertex[];
extern const BYTE pBinary_D3D9SM30_VPosition3dVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_VPosition3dVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VPosition3dVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTG[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchVertex[];
extern const BYTE pBinary_D3D9SM30_VBatchVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dVertex[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTG[];
extern const BYTE pBinary_D3D9SM30_VInstancedTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VInstancedTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchVertex[];
extern const BYTE pBinary_D3D9SM30_VInstancedVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VInstancedVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VInstancedTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_VInstancedTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VInstancedTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_VInstancedTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dVertex[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_VSolid[];
extern const BYTE pBinary_D3D9SM30_VText[];
extern const BYTE pBinary_D3D9SM30_VTextColor[];
extern const BYTE pBinary_D3D9SM30_VTextColorCxform[];
extern const BYTE pBinary_D3D9SM30_VPosition3dSolid[];
extern const BYTE pBinary_D3D9SM30_VPosition3dText[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTextColor[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTextColorCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchSolid[];
extern const BYTE pBinary_D3D9SM30_VBatchText[];
extern const BYTE pBinary_D3D9SM30_VBatchTextColor[];
extern const BYTE pBinary_D3D9SM30_VBatchTextColorCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dSolid[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dText[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTextColor[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTextColorCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchSolid[];
extern const BYTE pBinary_D3D9SM30_VBatchText[];
extern const BYTE pBinary_D3D9SM30_VBatchTextColor[];
extern const BYTE pBinary_D3D9SM30_VBatchTextColorCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dSolid[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dText[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTextColor[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTextColorCxform[];
extern const BYTE pBinary_D3D9SM30_VTexTG[];
extern const BYTE pBinary_D3D9SM30_VTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTG[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTG[];
extern const BYTE pBinary_D3D9SM30_VInstancedTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VInstancedTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTG[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_VBatchPosition3dTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VInstancedPosition3dTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_VTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_VDrawableCopyPixels[];
extern const BYTE pBinary_D3D9SM30_VDrawableCopyPixelsAlpha[];

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTexTG,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTexTGEAlpha,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTexTGCxform,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VVertex = 
{
    /* Type */          ShaderDesc::ST_Vertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VVertex,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VVertexEAlpha,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VVertexCxform = 
{
    /* Type */          ShaderDesc::ST_VertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VVertexCxform,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VVertexCxformEAlpha,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTexTGTexTG,
    /* Uniforms */      Uniforms_2666373766,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTexTGTexTGCxform,
    /* Uniforms */      Uniforms_3200611686,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_TexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTexTGVertex,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTexTGVertexCxform,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTexTG = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTG,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dVertex = 
{
    /* Type */          ShaderDesc::ST_Position3dVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dVertex,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dVertexEAlpha,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dVertexCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dVertexCxform,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dVertexCxformEAlpha,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGTexTG,
    /* Uniforms */      Uniforms_2590809056,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGTexTGCxform,
    /* Uniforms */      Uniforms_6124559549,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGVertex,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGVertexCxform,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTG,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchVertex = 
{
    /* Type */          ShaderDesc::ST_BatchVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchVertex,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchVertexEAlpha,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchVertexCxform,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchVertexCxformEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGTexTG,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_3422124174,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGTexTGCxform,
    /* Uniforms */      Uniforms_2393121201,
    /* BatchUniforms */ BatchUniforms_4076936984,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGVertex,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGVertexCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTG,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dVertex = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dVertex,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dVertexEAlpha,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dVertexCxform,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dVertexCxformEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGTexTG,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4749803573,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGTexTGCxform,
    /* Uniforms */      Uniforms_6117254102,
    /* BatchUniforms */ BatchUniforms_3953522647,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGVertex,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGVertexCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTG,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedTexTGEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchVertex,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedVertexEAlpha,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchVertexCxform,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedVertexCxformEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedTexTGTexTG,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_3422124174,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedTexTGTexTGCxform,
    /* Uniforms */      Uniforms_2393121201,
    /* BatchUniforms */ BatchUniforms_4076936984,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedTexTGVertex,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedTexTGVertexCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTG,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dVertex,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dVertexEAlpha,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dVertexCxform,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dVertexCxformEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dTexTGTexTG,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4749803573,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dTexTGTexTGCxform,
    /* Uniforms */      Uniforms_6117254102,
    /* BatchUniforms */ BatchUniforms_3953522647,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dTexTGVertex,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dTexTGVertexCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 1},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VSolid = 
{
    /* Type */          ShaderDesc::ST_Solid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VSolid,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VText = 
{
    /* Type */          ShaderDesc::ST_Text,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VText,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTextColor = 
{
    /* Type */          ShaderDesc::ST_TextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTextColor,
    /* Uniforms */      Uniforms_3962131312,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_TextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VTextColorCxform,
    /* Uniforms */      Uniforms_4660718830,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dSolid = 
{
    /* Type */          ShaderDesc::ST_Position3dSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dSolid,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dText = 
{
    /* Type */          ShaderDesc::ST_Position3dText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dText,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTextColor = 
{
    /* Type */          ShaderDesc::ST_Position3dTextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTextColor,
    /* Uniforms */      Uniforms_4480697281,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTextColorCxform,
    /* Uniforms */      Uniforms_3978807109,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchSolid = 
{
    /* Type */          ShaderDesc::ST_BatchSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchSolid,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchText = 
{
    /* Type */          ShaderDesc::ST_BatchText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchText,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTextColor = 
{
    /* Type */          ShaderDesc::ST_BatchTextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTextColor,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchTextColorCxform,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dSolid = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dSolid,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dText = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dText,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTextColor = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTextColor,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTextColorCxform,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedSolid = 
{
    /* Type */          ShaderDesc::ST_InstancedSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchSolid,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedText = 
{
    /* Type */          ShaderDesc::ST_InstancedText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchText,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTextColor = 
{
    /* Type */          ShaderDesc::ST_InstancedTextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchTextColor,
    /* Uniforms */      Uniforms_2647200773,
    /* BatchUniforms */ BatchUniforms_2375807902,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchTextColorCxform,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_5188561028,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dSolid = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dSolid,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dText = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dText,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    4,
    /* Attributes */    {
                              { "acolor",     4 | VET_Color, D3DDECLUSAGE_COLOR, 0},
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 2},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTextColor = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTextColor,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTextColor,
    /* Uniforms */      Uniforms_5257564337,
    /* BatchUniforms */ BatchUniforms_4952385266,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTextColorCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dTextColorCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTextColorCxform,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_5592595562,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "atc",        2 | VET_TexCoord, D3DDECLUSAGE_TEXCOORD, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VYUV = 
{
    /* Type */          ShaderDesc::ST_YUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VTexTG,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VTexTGEAlpha,
    /* Uniforms */      Uniforms_2402505279,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VYUVCxform = 
{
    /* Type */          ShaderDesc::ST_YUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VTexTGCxform,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dYUV = 
{
    /* Type */          ShaderDesc::ST_Position3dYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTG,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_6395323925,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dYUVCxform = 
{
    /* Type */          ShaderDesc::ST_Position3dYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VPosition3dYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_Position3dYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4496992820,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchYUV = 
{
    /* Type */          ShaderDesc::ST_BatchYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTG,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchYUVCxform = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dYUV = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTG,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dYUVCxform = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBatchPosition3dYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchPosition3dYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedYUV = 
{
    /* Type */          ShaderDesc::ST_InstancedYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTG,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VInstancedTexTGEAlpha,
    /* Uniforms */      Uniforms_4025051752,
    /* BatchUniforms */ BatchUniforms_3478612506,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedYUVCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchTexTGCxform,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VInstancedTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4705861522,
    /* BatchUniforms */ BatchUniforms_6232566636,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dYUV = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTG,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dTexTGEAlpha,
    /* Uniforms */      Uniforms_3965965598,
    /* BatchUniforms */ BatchUniforms_2889592211,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dYUVCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VBatchPosition3dTexTGCxform,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    2,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VInstancedPosition3dYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedPosition3dYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_VInstancedPosition3dTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_3892239162,
    /* BatchUniforms */ BatchUniforms_4272630808,
    /* NumAttribs */    3,
    /* Attributes */    {
                              { "afactor",    4 | VET_Color | (1 << VET_Index_Shift), D3DDECLUSAGE_COLOR, 0},
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              { "vbatch",     1 | VET_Instance8, D3DDECLUSAGE_COLOR, 1},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VBox1Blur = 
{
    /* Type */          ShaderDesc::ST_Box1Blur,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_VTexTGCxform,
    /* Uniforms */      Uniforms_2315053743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VDrawableCopyPixels = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixels,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VDrawableCopyPixels,
    /* Uniforms */      Uniforms_2666373766,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

static VertexShaderDesc ShaderDesc_VS_D3D9SM30_VDrawableCopyPixelsAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_VDrawableCopyPixelsAlpha,
    /* Uniforms */      Uniforms_2267270599,
    /* BatchUniforms */ BatchUniforms_6184305142,
    /* NumAttribs */    1,
    /* Attributes */    {
                              { "pos",        4 | VET_Pos, D3DDECLUSAGE_POSITION, 0},
                              },
    
};

const VertexShaderDesc* VertexShaderDesc::Descs[VSI_Count] = {
    0,
    &ShaderDesc_VS_D3D9SM20_VTexTG,                              // 1
    &ShaderDesc_VS_D3D9SM20_VTexTGEAlpha,                        // 2
    &ShaderDesc_VS_D3D9SM20_VTexTGCxform,                        // 3
    &ShaderDesc_VS_D3D9SM20_VTexTGCxformEAlpha,                  // 4
    &ShaderDesc_VS_D3D9SM20_VVertex,                             // 5
    &ShaderDesc_VS_D3D9SM20_VVertexEAlpha,                       // 6
    &ShaderDesc_VS_D3D9SM20_VVertexCxform,                       // 7
    &ShaderDesc_VS_D3D9SM20_VVertexCxformEAlpha,                 // 8
    &ShaderDesc_VS_D3D9SM20_VTexTGTexTG,                         // 9
    &ShaderDesc_VS_D3D9SM20_VTexTGTexTGCxform,                   // 10
    &ShaderDesc_VS_D3D9SM20_VTexTGVertex,                        // 11
    &ShaderDesc_VS_D3D9SM20_VTexTGVertexCxform,                  // 12
    &ShaderDesc_VS_D3D9SM20_VPosition3dTexTG,                    // 13
    &ShaderDesc_VS_D3D9SM20_VPosition3dTexTGEAlpha,              // 14
    &ShaderDesc_VS_D3D9SM20_VPosition3dTexTGCxform,              // 15
    &ShaderDesc_VS_D3D9SM20_VPosition3dTexTGCxformEAlpha,        // 16
    &ShaderDesc_VS_D3D9SM20_VPosition3dVertex,                   // 17
    &ShaderDesc_VS_D3D9SM20_VPosition3dVertexEAlpha,             // 18
    &ShaderDesc_VS_D3D9SM20_VPosition3dVertexCxform,             // 19
    &ShaderDesc_VS_D3D9SM20_VPosition3dVertexCxformEAlpha,       // 20
    &ShaderDesc_VS_D3D9SM20_VPosition3dTexTGTexTG,               // 21
    &ShaderDesc_VS_D3D9SM20_VPosition3dTexTGTexTGCxform,         // 22
    &ShaderDesc_VS_D3D9SM20_VPosition3dTexTGVertex,              // 23
    &ShaderDesc_VS_D3D9SM20_VPosition3dTexTGVertexCxform,        // 24
    &ShaderDesc_VS_D3D9SM20_VBatchTexTG,                         // 25
    &ShaderDesc_VS_D3D9SM20_VBatchTexTGEAlpha,                   // 26
    &ShaderDesc_VS_D3D9SM20_VBatchTexTGCxform,                   // 27
    &ShaderDesc_VS_D3D9SM20_VBatchTexTGCxformEAlpha,             // 28
    &ShaderDesc_VS_D3D9SM20_VBatchVertex,                        // 29
    &ShaderDesc_VS_D3D9SM20_VBatchVertexEAlpha,                  // 30
    &ShaderDesc_VS_D3D9SM20_VBatchVertexCxform,                  // 31
    &ShaderDesc_VS_D3D9SM20_VBatchVertexCxformEAlpha,            // 32
    &ShaderDesc_VS_D3D9SM20_VBatchTexTGTexTG,                    // 33
    &ShaderDesc_VS_D3D9SM20_VBatchTexTGTexTGCxform,              // 34
    &ShaderDesc_VS_D3D9SM20_VBatchTexTGVertex,                   // 35
    &ShaderDesc_VS_D3D9SM20_VBatchTexTGVertexCxform,             // 36
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTG,               // 37
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGEAlpha,         // 38
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGCxform,         // 39
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGCxformEAlpha,   // 40
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dVertex,              // 41
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dVertexEAlpha,        // 42
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dVertexCxform,        // 43
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dVertexCxformEAlpha,  // 44
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGTexTG,          // 45
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGTexTGCxform,    // 46
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGVertex,         // 47
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTexTGVertexCxform,   // 48
    &ShaderDesc_VS_D3D9SM20_VSolid,                              // 49
    &ShaderDesc_VS_D3D9SM20_VText,                               // 50
    &ShaderDesc_VS_D3D9SM20_VTextColor,                          // 51
    &ShaderDesc_VS_D3D9SM20_VTextColorCxform,                    // 52
    &ShaderDesc_VS_D3D9SM20_VPosition3dSolid,                    // 53
    &ShaderDesc_VS_D3D9SM20_VPosition3dText,                     // 54
    &ShaderDesc_VS_D3D9SM20_VPosition3dTextColor,                // 55
    &ShaderDesc_VS_D3D9SM20_VPosition3dTextColorCxform,          // 56
    &ShaderDesc_VS_D3D9SM20_VBatchSolid,                         // 57
    &ShaderDesc_VS_D3D9SM20_VBatchText,                          // 58
    &ShaderDesc_VS_D3D9SM20_VBatchTextColor,                     // 59
    &ShaderDesc_VS_D3D9SM20_VBatchTextColorCxform,               // 60
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dSolid,               // 61
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dText,                // 62
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTextColor,           // 63
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dTextColorCxform,     // 64
    &ShaderDesc_VS_D3D9SM20_VYUV,                                // 65
    &ShaderDesc_VS_D3D9SM20_VYUVEAlpha,                          // 66
    &ShaderDesc_VS_D3D9SM20_VYUVCxform,                          // 67
    &ShaderDesc_VS_D3D9SM20_VYUVCxformEAlpha,                    // 68
    &ShaderDesc_VS_D3D9SM20_VPosition3dYUV,                      // 69
    &ShaderDesc_VS_D3D9SM20_VPosition3dYUVEAlpha,                // 70
    &ShaderDesc_VS_D3D9SM20_VPosition3dYUVCxform,                // 71
    &ShaderDesc_VS_D3D9SM20_VPosition3dYUVCxformEAlpha,          // 72
    &ShaderDesc_VS_D3D9SM20_VBatchYUV,                           // 73
    &ShaderDesc_VS_D3D9SM20_VBatchYUVEAlpha,                     // 74
    &ShaderDesc_VS_D3D9SM20_VBatchYUVCxform,                     // 75
    &ShaderDesc_VS_D3D9SM20_VBatchYUVCxformEAlpha,               // 76
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dYUV,                 // 77
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dYUVEAlpha,           // 78
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dYUVCxform,           // 79
    &ShaderDesc_VS_D3D9SM20_VBatchPosition3dYUVCxformEAlpha,     // 80
    &ShaderDesc_VS_D3D9SM20_VDrawableCopyPixels,                 // 81
    &ShaderDesc_VS_D3D9SM20_VDrawableCopyPixelsAlpha,            // 82
    &ShaderDesc_VS_D3D9SM30_VTexTG,                              // 83
    &ShaderDesc_VS_D3D9SM30_VTexTGEAlpha,                        // 84
    &ShaderDesc_VS_D3D9SM30_VTexTGCxform,                        // 85
    &ShaderDesc_VS_D3D9SM30_VTexTGCxformEAlpha,                  // 86
    &ShaderDesc_VS_D3D9SM30_VVertex,                             // 87
    &ShaderDesc_VS_D3D9SM30_VVertexEAlpha,                       // 88
    &ShaderDesc_VS_D3D9SM30_VVertexCxform,                       // 89
    &ShaderDesc_VS_D3D9SM30_VVertexCxformEAlpha,                 // 90
    &ShaderDesc_VS_D3D9SM30_VTexTGTexTG,                         // 91
    &ShaderDesc_VS_D3D9SM30_VTexTGTexTGCxform,                   // 92
    &ShaderDesc_VS_D3D9SM30_VTexTGVertex,                        // 93
    &ShaderDesc_VS_D3D9SM30_VTexTGVertexCxform,                  // 94
    &ShaderDesc_VS_D3D9SM30_VPosition3dTexTG,                    // 95
    &ShaderDesc_VS_D3D9SM30_VPosition3dTexTGEAlpha,              // 96
    &ShaderDesc_VS_D3D9SM30_VPosition3dTexTGCxform,              // 97
    &ShaderDesc_VS_D3D9SM30_VPosition3dTexTGCxformEAlpha,        // 98
    &ShaderDesc_VS_D3D9SM30_VPosition3dVertex,                   // 99
    &ShaderDesc_VS_D3D9SM30_VPosition3dVertexEAlpha,             // 100
    &ShaderDesc_VS_D3D9SM30_VPosition3dVertexCxform,             // 101
    &ShaderDesc_VS_D3D9SM30_VPosition3dVertexCxformEAlpha,       // 102
    &ShaderDesc_VS_D3D9SM30_VPosition3dTexTGTexTG,               // 103
    &ShaderDesc_VS_D3D9SM30_VPosition3dTexTGTexTGCxform,         // 104
    &ShaderDesc_VS_D3D9SM30_VPosition3dTexTGVertex,              // 105
    &ShaderDesc_VS_D3D9SM30_VPosition3dTexTGVertexCxform,        // 106
    &ShaderDesc_VS_D3D9SM30_VBatchTexTG,                         // 107
    &ShaderDesc_VS_D3D9SM30_VBatchTexTGEAlpha,                   // 108
    &ShaderDesc_VS_D3D9SM30_VBatchTexTGCxform,                   // 109
    &ShaderDesc_VS_D3D9SM30_VBatchTexTGCxformEAlpha,             // 110
    &ShaderDesc_VS_D3D9SM30_VBatchVertex,                        // 111
    &ShaderDesc_VS_D3D9SM30_VBatchVertexEAlpha,                  // 112
    &ShaderDesc_VS_D3D9SM30_VBatchVertexCxform,                  // 113
    &ShaderDesc_VS_D3D9SM30_VBatchVertexCxformEAlpha,            // 114
    &ShaderDesc_VS_D3D9SM30_VBatchTexTGTexTG,                    // 115
    &ShaderDesc_VS_D3D9SM30_VBatchTexTGTexTGCxform,              // 116
    &ShaderDesc_VS_D3D9SM30_VBatchTexTGVertex,                   // 117
    &ShaderDesc_VS_D3D9SM30_VBatchTexTGVertexCxform,             // 118
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTG,               // 119
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGEAlpha,         // 120
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGCxform,         // 121
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGCxformEAlpha,   // 122
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dVertex,              // 123
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dVertexEAlpha,        // 124
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dVertexCxform,        // 125
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dVertexCxformEAlpha,  // 126
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGTexTG,          // 127
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGTexTGCxform,    // 128
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGVertex,         // 129
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTexTGVertexCxform,   // 130
    &ShaderDesc_VS_D3D9SM30_VInstancedTexTG,                     // 131
    &ShaderDesc_VS_D3D9SM30_VInstancedTexTGEAlpha,               // 132
    &ShaderDesc_VS_D3D9SM30_VInstancedTexTGCxform,               // 133
    &ShaderDesc_VS_D3D9SM30_VInstancedTexTGCxformEAlpha,         // 134
    &ShaderDesc_VS_D3D9SM30_VInstancedVertex,                    // 135
    &ShaderDesc_VS_D3D9SM30_VInstancedVertexEAlpha,              // 136
    &ShaderDesc_VS_D3D9SM30_VInstancedVertexCxform,              // 137
    &ShaderDesc_VS_D3D9SM30_VInstancedVertexCxformEAlpha,        // 138
    &ShaderDesc_VS_D3D9SM30_VInstancedTexTGTexTG,                // 139
    &ShaderDesc_VS_D3D9SM30_VInstancedTexTGTexTGCxform,          // 140
    &ShaderDesc_VS_D3D9SM30_VInstancedTexTGVertex,               // 141
    &ShaderDesc_VS_D3D9SM30_VInstancedTexTGVertexCxform,         // 142
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTG,           // 143
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGEAlpha,     // 144
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGCxform,     // 145
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGCxformEAlpha, // 146
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dVertex,          // 147
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dVertexEAlpha,    // 148
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dVertexCxform,    // 149
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dVertexCxformEAlpha, // 150
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGTexTG,      // 151
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGTexTGCxform, // 152
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGVertex,     // 153
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTexTGVertexCxform, // 154
    &ShaderDesc_VS_D3D9SM30_VSolid,                              // 155
    &ShaderDesc_VS_D3D9SM30_VText,                               // 156
    &ShaderDesc_VS_D3D9SM30_VTextColor,                          // 157
    &ShaderDesc_VS_D3D9SM30_VTextColorCxform,                    // 158
    &ShaderDesc_VS_D3D9SM30_VPosition3dSolid,                    // 159
    &ShaderDesc_VS_D3D9SM30_VPosition3dText,                     // 160
    &ShaderDesc_VS_D3D9SM30_VPosition3dTextColor,                // 161
    &ShaderDesc_VS_D3D9SM30_VPosition3dTextColorCxform,          // 162
    &ShaderDesc_VS_D3D9SM30_VBatchSolid,                         // 163
    &ShaderDesc_VS_D3D9SM30_VBatchText,                          // 164
    &ShaderDesc_VS_D3D9SM30_VBatchTextColor,                     // 165
    &ShaderDesc_VS_D3D9SM30_VBatchTextColorCxform,               // 166
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dSolid,               // 167
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dText,                // 168
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTextColor,           // 169
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dTextColorCxform,     // 170
    &ShaderDesc_VS_D3D9SM30_VInstancedSolid,                     // 171
    &ShaderDesc_VS_D3D9SM30_VInstancedText,                      // 172
    &ShaderDesc_VS_D3D9SM30_VInstancedTextColor,                 // 173
    &ShaderDesc_VS_D3D9SM30_VInstancedTextColorCxform,           // 174
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dSolid,           // 175
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dText,            // 176
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTextColor,       // 177
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dTextColorCxform, // 178
    &ShaderDesc_VS_D3D9SM30_VYUV,                                // 179
    &ShaderDesc_VS_D3D9SM30_VYUVEAlpha,                          // 180
    &ShaderDesc_VS_D3D9SM30_VYUVCxform,                          // 181
    &ShaderDesc_VS_D3D9SM30_VYUVCxformEAlpha,                    // 182
    &ShaderDesc_VS_D3D9SM30_VPosition3dYUV,                      // 183
    &ShaderDesc_VS_D3D9SM30_VPosition3dYUVEAlpha,                // 184
    &ShaderDesc_VS_D3D9SM30_VPosition3dYUVCxform,                // 185
    &ShaderDesc_VS_D3D9SM30_VPosition3dYUVCxformEAlpha,          // 186
    &ShaderDesc_VS_D3D9SM30_VBatchYUV,                           // 187
    &ShaderDesc_VS_D3D9SM30_VBatchYUVEAlpha,                     // 188
    &ShaderDesc_VS_D3D9SM30_VBatchYUVCxform,                     // 189
    &ShaderDesc_VS_D3D9SM30_VBatchYUVCxformEAlpha,               // 190
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dYUV,                 // 191
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dYUVEAlpha,           // 192
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dYUVCxform,           // 193
    &ShaderDesc_VS_D3D9SM30_VBatchPosition3dYUVCxformEAlpha,     // 194
    &ShaderDesc_VS_D3D9SM30_VInstancedYUV,                       // 195
    &ShaderDesc_VS_D3D9SM30_VInstancedYUVEAlpha,                 // 196
    &ShaderDesc_VS_D3D9SM30_VInstancedYUVCxform,                 // 197
    &ShaderDesc_VS_D3D9SM30_VInstancedYUVCxformEAlpha,           // 198
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dYUV,             // 199
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dYUVEAlpha,       // 200
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dYUVCxform,       // 201
    &ShaderDesc_VS_D3D9SM30_VInstancedPosition3dYUVCxformEAlpha, // 202
    &ShaderDesc_VS_D3D9SM30_VBox1Blur,                           // 203
    &ShaderDesc_VS_D3D9SM30_VDrawableCopyPixels,                 // 204
    &ShaderDesc_VS_D3D9SM30_VDrawableCopyPixelsAlpha,            // 205
};

const VertexShaderDesc* VertexShaderDesc::GetDesc(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    return Descs[GetShaderIndex(shader, ver)];
};

VertexShaderDesc::ShaderIndex VertexShaderDesc::GetShaderIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    ShaderDesc::ShaderIndex index = ShaderDesc::GetShaderIndex(shader);
    return ShaderIndexingData[index][ver]. VertexIndex;
};

VertexShaderDesc::ShaderIndex VertexShaderDesc::GetShaderIndexForComboIndex(unsigned index, ShaderDesc::ShaderVersion ver) 
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_D3D9SM20:
        {
            SF_UNUSED(index);
            SF_DEBUG_ASSERT(1, "D3D9SM20 indicated that ShaderCombo information was not required.");
            return VSI_None;
        }
        case ShaderDesc::ShaderVersion_D3D9SM30:
        {
            SF_UNUSED(index);
            SF_DEBUG_ASSERT(1, "D3D9SM30 indicated that ShaderCombo information was not required.");
            return VSI_None;
        }
        default: SF_ASSERT(0); return VSI_None;
    }
}

unsigned VertexShaderDesc::GetShaderComboIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_D3D9SM20:
        {
            SF_UNUSED(shader);
            SF_DEBUG_ASSERT(1, "D3D9SM20 indicated that ShaderCombo information was not required.");
            return 0;
        }
        case ShaderDesc::ShaderVersion_D3D9SM30:
        {
            SF_UNUSED(shader);
            SF_DEBUG_ASSERT(1, "D3D9SM30 indicated that ShaderCombo information was not required.");
            return 0;
        }
        default: SF_ASSERT(0); return 0;
    }
}

extern const BYTE pBinary_D3D9SM20_FTexTG[];
extern const BYTE pBinary_D3D9SM20_FTexTGMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FVertex[];
extern const BYTE pBinary_D3D9SM20_FVertexMul[];
extern const BYTE pBinary_D3D9SM20_FVertexInv[];
extern const BYTE pBinary_D3D9SM20_FVertexEAlpha[];
extern const BYTE pBinary_D3D9SM20_FVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FVertexCxform[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformMul[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformInv[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAc[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTG[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertex[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAc[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTG[];
extern const BYTE pBinary_D3D9SM20_FTexTGMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FVertex[];
extern const BYTE pBinary_D3D9SM20_FVertexMul[];
extern const BYTE pBinary_D3D9SM20_FVertexInv[];
extern const BYTE pBinary_D3D9SM20_FVertexEAlpha[];
extern const BYTE pBinary_D3D9SM20_FVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FVertexCxform[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformMul[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformInv[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAc[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTG[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertex[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAc[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FSolid[];
extern const BYTE pBinary_D3D9SM20_FSolidMul[];
extern const BYTE pBinary_D3D9SM20_FSolidInv[];
extern const BYTE pBinary_D3D9SM20_FText[];
extern const BYTE pBinary_D3D9SM20_FTextMul[];
extern const BYTE pBinary_D3D9SM20_FTextInv[];
extern const BYTE pBinary_D3D9SM20_FSolid[];
extern const BYTE pBinary_D3D9SM20_FSolidMul[];
extern const BYTE pBinary_D3D9SM20_FSolidInv[];
extern const BYTE pBinary_D3D9SM20_FText[];
extern const BYTE pBinary_D3D9SM20_FTextMul[];
extern const BYTE pBinary_D3D9SM20_FTextInv[];
extern const BYTE pBinary_D3D9SM20_FYUV[];
extern const BYTE pBinary_D3D9SM20_FYUVMul[];
extern const BYTE pBinary_D3D9SM20_FYUVEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVCxform[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformMul[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformAc[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVA[];
extern const BYTE pBinary_D3D9SM20_FYUVAMul[];
extern const BYTE pBinary_D3D9SM20_FYUVAInv[];
extern const BYTE pBinary_D3D9SM20_FYUVAEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVAEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVAEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FYUVACxform[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformMul[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformInv[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAc[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FYUV[];
extern const BYTE pBinary_D3D9SM20_FYUVMul[];
extern const BYTE pBinary_D3D9SM20_FYUVEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVCxform[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformMul[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformAc[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVA[];
extern const BYTE pBinary_D3D9SM20_FYUVAMul[];
extern const BYTE pBinary_D3D9SM20_FYUVAInv[];
extern const BYTE pBinary_D3D9SM20_FYUVAEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVAEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVAEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FYUVACxform[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformMul[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformInv[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAc[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcMul[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcInv[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FYUVACxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM20_FTexTGCMatrixAc[];
extern const BYTE pBinary_D3D9SM20_FTexTGCMatrixAcMul[];
extern const BYTE pBinary_D3D9SM20_FTexTGCMatrixAcEAlpha[];
extern const BYTE pBinary_D3D9SM20_FTexTGCMatrixAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM20_FDrawableCopyPixels[];
extern const BYTE pBinary_D3D9SM20_FDrawableCopyPixelsMergeAlpha[];
extern const BYTE pBinary_D3D9SM20_FDrawableCopyPixelsNoDestAlpha[];
extern const BYTE pBinary_D3D9SM20_FDrawableCopyPixelsAlpha[];
extern const BYTE pBinary_D3D9SM20_FDrawableCopyPixelsAlphaMergeAlpha[];
extern const BYTE pBinary_D3D9SM20_FDrawableCopyPixelsAlphaNoDestAlpha[];
extern const BYTE pBinary_D3D9SM20_FDrawableMerge[];
extern const BYTE pBinary_D3D9SM20_FDrawableCompare[];
extern const BYTE pBinary_D3D9SM20_FDrawablePaletteMap[];

UniformVar Uniforms_5292135573[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2601942360[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_4895524107[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 2, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2364729977[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      {0, 1036, 4, 4, 4, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        { -1, 0, 0, 0, 0, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_5642581497[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 3, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_5475920020[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 4, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2817570809[Uniform::SU_Count] = 
{
    /* cxadd*/      {0, 1036, 4, 4, 4, 0 },
    /* cxmul*/      {1, 1040, 16, 16, 16, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3033064606[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      {0, 1036, 16, 16, 16, 0 },
    /* cxmul1*/     {4, 1052, 16, 16, 16, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 2, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3820893743[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      { -1, 0, 0, 0, 0, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     {0, 1036, 1, 1, 1, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {1, 1040, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   { -1, 0, 0, 0, 0, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTG,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGMul = 
{
    /* Type */          ShaderDesc::ST_TexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGInv = 
{
    /* Type */          ShaderDesc::ST_TexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertex = 
{
    /* Type */          ShaderDesc::ST_Vertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertex,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexMul = 
{
    /* Type */          ShaderDesc::ST_VertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexInv = 
{
    /* Type */          ShaderDesc::ST_VertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxform = 
{
    /* Type */          ShaderDesc::ST_VertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxform,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAc,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTG,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxform,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAc,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_TexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertex,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTG,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertex = 
{
    /* Type */          ShaderDesc::ST_BatchVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertex,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxform,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAc,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTG,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxform,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAc,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertex,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTexTGVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FSolid = 
{
    /* Type */          ShaderDesc::ST_Solid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FSolid,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FSolidMul = 
{
    /* Type */          ShaderDesc::ST_SolidMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FSolidMul,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FSolidInv = 
{
    /* Type */          ShaderDesc::ST_SolidInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FSolidInv,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FText = 
{
    /* Type */          ShaderDesc::ST_Text,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FText,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTextMul = 
{
    /* Type */          ShaderDesc::ST_TextMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTextMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTextInv = 
{
    /* Type */          ShaderDesc::ST_TextInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTextInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchSolid = 
{
    /* Type */          ShaderDesc::ST_BatchSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FSolid,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchSolidMul = 
{
    /* Type */          ShaderDesc::ST_BatchSolidMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FSolidMul,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchSolidInv = 
{
    /* Type */          ShaderDesc::ST_BatchSolidInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FSolidInv,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchText = 
{
    /* Type */          ShaderDesc::ST_BatchText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FText,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTextMul = 
{
    /* Type */          ShaderDesc::ST_BatchTextMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTextMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchTextInv = 
{
    /* Type */          ShaderDesc::ST_BatchTextInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM20_FTextInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUV = 
{
    /* Type */          ShaderDesc::ST_YUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUV,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVMul = 
{
    /* Type */          ShaderDesc::ST_YUVMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVCxform = 
{
    /* Type */          ShaderDesc::ST_YUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxform,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVCxformMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVCxformAc = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformAc,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformAcMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformAcEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVA = 
{
    /* Type */          ShaderDesc::ST_YUVA,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVA,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVAMul = 
{
    /* Type */          ShaderDesc::ST_YUVAMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVAInv = 
{
    /* Type */          ShaderDesc::ST_YUVAInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVAEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVAEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVAEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxform = 
{
    /* Type */          ShaderDesc::ST_YUVACxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxform,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformAc = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAc,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformAcMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformAcInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FYUVACxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUV = 
{
    /* Type */          ShaderDesc::ST_BatchYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUV,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVCxform = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxform,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformAc,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformAcMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformAcEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVA = 
{
    /* Type */          ShaderDesc::ST_BatchYUVA,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVA,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVAMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVAInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVAEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVAEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVAEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVAEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxform = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxform,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAc,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM20_FYUVACxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCMatrixAc = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCMatrixAc,
    /* Uniforms */      Uniforms_2817570809,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCMatrixAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCMatrixAcMul,
    /* Uniforms */      Uniforms_2817570809,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCMatrixAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCMatrixAcEAlpha,
    /* Uniforms */      Uniforms_2817570809,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FTexTGCMatrixAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FTexTGCMatrixAcEAlphaMul,
    /* Uniforms */      Uniforms_2817570809,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawableCopyPixels = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixels,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawableCopyPixels,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsMergeAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsMergeAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawableCopyPixelsMergeAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsNoDestAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsNoDestAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawableCopyPixelsNoDestAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawableCopyPixelsAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsAlphaMergeAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlphaMergeAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawableCopyPixelsAlphaMergeAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsAlphaNoDestAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlphaNoDestAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawableCopyPixelsAlphaNoDestAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawableMerge = 
{
    /* Type */          ShaderDesc::ST_DrawableMerge,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawableMerge,
    /* Uniforms */      Uniforms_3033064606,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawableCompare = 
{
    /* Type */          ShaderDesc::ST_DrawableCompare,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawableCompare,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM20_FDrawablePaletteMap = 
{
    /* Type */          ShaderDesc::ST_DrawablePaletteMap,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM20,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM20_FDrawablePaletteMap,
    /* Uniforms */      Uniforms_3820893743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

extern const BYTE pBinary_D3D9SM30_FTexTG[];
extern const BYTE pBinary_D3D9SM30_FTexTGMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertex[];
extern const BYTE pBinary_D3D9SM30_FVertexMul[];
extern const BYTE pBinary_D3D9SM30_FVertexInv[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxform[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAc[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTG[];
extern const BYTE pBinary_D3D9SM30_FTexTGMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertex[];
extern const BYTE pBinary_D3D9SM30_FVertexMul[];
extern const BYTE pBinary_D3D9SM30_FVertexInv[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxform[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAc[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTG[];
extern const BYTE pBinary_D3D9SM30_FTexTGMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertex[];
extern const BYTE pBinary_D3D9SM30_FVertexMul[];
extern const BYTE pBinary_D3D9SM30_FVertexInv[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxform[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAc[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTG[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertex[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxform[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FSolid[];
extern const BYTE pBinary_D3D9SM30_FSolidMul[];
extern const BYTE pBinary_D3D9SM30_FSolidInv[];
extern const BYTE pBinary_D3D9SM30_FText[];
extern const BYTE pBinary_D3D9SM30_FTextMul[];
extern const BYTE pBinary_D3D9SM30_FTextInv[];
extern const BYTE pBinary_D3D9SM30_FSolid[];
extern const BYTE pBinary_D3D9SM30_FSolidMul[];
extern const BYTE pBinary_D3D9SM30_FSolidInv[];
extern const BYTE pBinary_D3D9SM30_FText[];
extern const BYTE pBinary_D3D9SM30_FTextMul[];
extern const BYTE pBinary_D3D9SM30_FTextInv[];
extern const BYTE pBinary_D3D9SM30_FSolid[];
extern const BYTE pBinary_D3D9SM30_FSolidMul[];
extern const BYTE pBinary_D3D9SM30_FSolidInv[];
extern const BYTE pBinary_D3D9SM30_FText[];
extern const BYTE pBinary_D3D9SM30_FTextMul[];
extern const BYTE pBinary_D3D9SM30_FTextInv[];
extern const BYTE pBinary_D3D9SM30_FYUV[];
extern const BYTE pBinary_D3D9SM30_FYUVMul[];
extern const BYTE pBinary_D3D9SM30_FYUVEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxform[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAc[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVA[];
extern const BYTE pBinary_D3D9SM30_FYUVAMul[];
extern const BYTE pBinary_D3D9SM30_FYUVAInv[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxform[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAc[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FYUV[];
extern const BYTE pBinary_D3D9SM30_FYUVMul[];
extern const BYTE pBinary_D3D9SM30_FYUVEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxform[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAc[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVA[];
extern const BYTE pBinary_D3D9SM30_FYUVAMul[];
extern const BYTE pBinary_D3D9SM30_FYUVAInv[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxform[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAc[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FYUV[];
extern const BYTE pBinary_D3D9SM30_FYUVMul[];
extern const BYTE pBinary_D3D9SM30_FYUVEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxform[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAc[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVCxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVA[];
extern const BYTE pBinary_D3D9SM30_FYUVAMul[];
extern const BYTE pBinary_D3D9SM30_FYUVAInv[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVAEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxform[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAc[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcInv[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FYUVACxformAcEAlphaInv[];
extern const BYTE pBinary_D3D9SM30_FTexTGCMatrixAc[];
extern const BYTE pBinary_D3D9SM30_FTexTGCMatrixAcMul[];
extern const BYTE pBinary_D3D9SM30_FTexTGCMatrixAcEAlpha[];
extern const BYTE pBinary_D3D9SM30_FTexTGCMatrixAcEAlphaMul[];
extern const BYTE pBinary_D3D9SM30_FBox1Blur[];
extern const BYTE pBinary_D3D9SM30_FBox1BlurMul[];
extern const BYTE pBinary_D3D9SM30_FBox2Blur[];
extern const BYTE pBinary_D3D9SM30_FBox2BlurMul[];
extern const BYTE pBinary_D3D9SM30_FBox2Shadow[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowMul[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowKnockout[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowKnockoutMul[];
extern const BYTE pBinary_D3D9SM30_FBox2InnerShadow[];
extern const BYTE pBinary_D3D9SM30_FBox2InnerShadowMul[];
extern const BYTE pBinary_D3D9SM30_FBox2InnerShadowKnockout[];
extern const BYTE pBinary_D3D9SM30_FBox2InnerShadowKnockoutMul[];
extern const BYTE pBinary_D3D9SM30_FBox2Shadowonly[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowonlyMul[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowHighlight[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowHighlightMul[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowHighlightKnockout[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowHighlightKnockoutMul[];
extern const BYTE pBinary_D3D9SM30_FBox2InnerShadowHighlight[];
extern const BYTE pBinary_D3D9SM30_FBox2InnerShadowHighlightMul[];
extern const BYTE pBinary_D3D9SM30_FBox2InnerShadowHighlightKnockout[];
extern const BYTE pBinary_D3D9SM30_FBox2InnerShadowHighlightKnockoutMul[];
extern const BYTE pBinary_D3D9SM30_FBox2FullShadowHighlight[];
extern const BYTE pBinary_D3D9SM30_FBox2FullShadowHighlightMul[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowonlyHighlight[];
extern const BYTE pBinary_D3D9SM30_FBox2ShadowonlyHighlightMul[];
extern const BYTE pBinary_D3D9SM30_FDrawableCopyPixels[];
extern const BYTE pBinary_D3D9SM30_FDrawableCopyPixelsMergeAlpha[];
extern const BYTE pBinary_D3D9SM30_FDrawableCopyPixelsNoDestAlpha[];
extern const BYTE pBinary_D3D9SM30_FDrawableCopyPixelsAlpha[];
extern const BYTE pBinary_D3D9SM30_FDrawableCopyPixelsAlphaMergeAlpha[];
extern const BYTE pBinary_D3D9SM30_FDrawableCopyPixelsAlphaNoDestAlpha[];
extern const BYTE pBinary_D3D9SM30_FDrawableMerge[];
extern const BYTE pBinary_D3D9SM30_FDrawableCompare[];
extern const BYTE pBinary_D3D9SM30_FDrawablePaletteMap[];

UniformVar Uniforms_5656300913[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1036, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor*/     { -1, 0, 0, 0, 0, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {1, 1040, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2478189079[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1036, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     {1, 1040, 4, 4, 4, 0 },
    /* scolor*/     {2, 1044, 4, 4, 4, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     {0, 1036, 1, 1, 1, 0 },
    /* srctexscale*/{3, 1048, 4, 4, 4, 0 },
    /* tex*/        {1, 1040, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {4, 1052, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_2286958232[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1036, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     {1, 1040, 4, 4, 4, 0 },
    /* scolor*/     {2, 1044, 4, 4, 4, 0 },
    /* scolor2*/    { -1, 0, 0, 0, 0, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {3, 1048, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_3893930098[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1036, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     {1, 1040, 4, 4, 4, 0 },
    /* scolor*/     {2, 1044, 4, 4, 4, 0 },
    /* scolor2*/    {3, 1048, 4, 4, 4, 0 },
    /* srctex*/     {0, 1036, 1, 1, 1, 0 },
    /* srctexscale*/{4, 1052, 4, 4, 4, 0 },
    /* tex*/        {1, 1040, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {5, 1056, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

UniformVar Uniforms_6259487864[Uniform::SU_Count] = 
{
    /* cxadd*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul*/      { -1, 0, 0, 0, 0, 0 },
    /* cxmul1*/     { -1, 0, 0, 0, 0, 0 },
    /* fsize*/      {0, 1036, 4, 4, 4, 0 },
    /* mvp*/        { -1, 0, 0, 0, 0, 0 },
    /* offset*/     {1, 1040, 4, 4, 4, 0 },
    /* scolor*/     {2, 1044, 4, 4, 4, 0 },
    /* scolor2*/    {3, 1048, 4, 4, 4, 0 },
    /* srctex*/     { -1, 0, 0, 0, 0, 0 },
    /* srctexscale*/{ -1, 0, 0, 0, 0, 0 },
    /* tex*/        {0, 1036, 1, 1, 1, 0 },
    /* texgen*/     { -1, 0, 0, 0, 0, 0 },
    /* texscale*/   {4, 1052, 4, 4, 4, 0 },
    /* vfmuniforms*/{ -1, 0, 0, 0, 0, 0 },
    /* vfuniforms*/ { -1, 0, 0, 0, 0, 0 },
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTG,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGMul = 
{
    /* Type */          ShaderDesc::ST_TexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGInv = 
{
    /* Type */          ShaderDesc::ST_TexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertex = 
{
    /* Type */          ShaderDesc::ST_Vertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertex,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexMul = 
{
    /* Type */          ShaderDesc::ST_VertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexInv = 
{
    /* Type */          ShaderDesc::ST_VertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxform = 
{
    /* Type */          ShaderDesc::ST_VertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxform,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAc,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_VertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTG,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxform,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAc,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_TexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertex,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_TexTGVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTG,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertex = 
{
    /* Type */          ShaderDesc::ST_BatchVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertex,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxform,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAc,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTG,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxform,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAc,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertex,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchTexTGVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTG,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertex,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxform,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAc,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_2601942360,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTG = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTG,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTG,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxform,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAc,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGTexTGCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertex = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertex,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertex,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxform,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAc,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlpha,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTexTGVertexCxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTexTGVertexCxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FSolid = 
{
    /* Type */          ShaderDesc::ST_Solid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FSolid,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FSolidMul = 
{
    /* Type */          ShaderDesc::ST_SolidMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FSolidMul,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FSolidInv = 
{
    /* Type */          ShaderDesc::ST_SolidInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FSolidInv,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FText = 
{
    /* Type */          ShaderDesc::ST_Text,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FText,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTextMul = 
{
    /* Type */          ShaderDesc::ST_TextMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTextMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTextInv = 
{
    /* Type */          ShaderDesc::ST_TextInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTextInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchSolid = 
{
    /* Type */          ShaderDesc::ST_BatchSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FSolid,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchSolidMul = 
{
    /* Type */          ShaderDesc::ST_BatchSolidMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FSolidMul,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchSolidInv = 
{
    /* Type */          ShaderDesc::ST_BatchSolidInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FSolidInv,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchText = 
{
    /* Type */          ShaderDesc::ST_BatchText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FText,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTextMul = 
{
    /* Type */          ShaderDesc::ST_BatchTextMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTextMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchTextInv = 
{
    /* Type */          ShaderDesc::ST_BatchTextInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch,
    /* pBinary */       pBinary_D3D9SM30_FTextInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedSolid = 
{
    /* Type */          ShaderDesc::ST_InstancedSolid,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FSolid,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedSolidMul = 
{
    /* Type */          ShaderDesc::ST_InstancedSolidMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FSolidMul,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedSolidInv = 
{
    /* Type */          ShaderDesc::ST_InstancedSolidInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FSolidInv,
    /* Uniforms */      Uniforms_2364729977,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedText = 
{
    /* Type */          ShaderDesc::ST_InstancedText,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FText,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTextMul = 
{
    /* Type */          ShaderDesc::ST_InstancedTextMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTextMul,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedTextInv = 
{
    /* Type */          ShaderDesc::ST_InstancedTextInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced,
    /* pBinary */       pBinary_D3D9SM30_FTextInv,
    /* Uniforms */      Uniforms_5292135573,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUV = 
{
    /* Type */          ShaderDesc::ST_YUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUV,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVMul = 
{
    /* Type */          ShaderDesc::ST_YUVMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVCxform = 
{
    /* Type */          ShaderDesc::ST_YUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxform,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVCxformMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVCxformAc = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAc,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVA = 
{
    /* Type */          ShaderDesc::ST_YUVA,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVA,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVAMul = 
{
    /* Type */          ShaderDesc::ST_YUVAMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVAInv = 
{
    /* Type */          ShaderDesc::ST_YUVAInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVAEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVAEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVAEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVAEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxform = 
{
    /* Type */          ShaderDesc::ST_YUVACxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxform,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformAc = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAc,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformAcMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformAcInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FYUVACxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_YUVACxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUV = 
{
    /* Type */          ShaderDesc::ST_BatchYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUV,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVCxform = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxform,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVCxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVCxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAc,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVA = 
{
    /* Type */          ShaderDesc::ST_BatchYUVA,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVA,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVAMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVAInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVAEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVAEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVAEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVAEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxform = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxform,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAc = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAc,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_BatchYUVACxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Batch | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUV = 
{
    /* Type */          ShaderDesc::ST_InstancedYUV,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUV,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVCxform = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxform,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAc,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcEAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVCxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVCxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVA = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVA,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVA,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVAMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVAInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVAEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVAEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVAEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVAEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVAEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxform = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxform,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxform,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAc = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAc,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlpha,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlphaMul,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcEAlphaInv = 
{
    /* Type */          ShaderDesc::ST_InstancedYUVACxformAcEAlphaInv,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_Instanced | Shader_Video,
    /* pBinary */       pBinary_D3D9SM30_FYUVACxformAcEAlphaInv,
    /* Uniforms */      Uniforms_5475920020,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCMatrixAc = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAc,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCMatrixAc,
    /* Uniforms */      Uniforms_2817570809,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCMatrixAcMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCMatrixAcMul,
    /* Uniforms */      Uniforms_2817570809,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCMatrixAcEAlpha = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcEAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCMatrixAcEAlpha,
    /* Uniforms */      Uniforms_2817570809,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FTexTGCMatrixAcEAlphaMul = 
{
    /* Type */          ShaderDesc::ST_TexTGCMatrixAcEAlphaMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FTexTGCMatrixAcEAlphaMul,
    /* Uniforms */      Uniforms_2817570809,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox1Blur = 
{
    /* Type */          ShaderDesc::ST_Box1Blur,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox1Blur,
    /* Uniforms */      Uniforms_5656300913,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox1BlurMul = 
{
    /* Type */          ShaderDesc::ST_Box1BlurMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox1BlurMul,
    /* Uniforms */      Uniforms_5656300913,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2Blur = 
{
    /* Type */          ShaderDesc::ST_Box2Blur,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2Blur,
    /* Uniforms */      Uniforms_5656300913,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2BlurMul = 
{
    /* Type */          ShaderDesc::ST_Box2BlurMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2BlurMul,
    /* Uniforms */      Uniforms_5656300913,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2Shadow = 
{
    /* Type */          ShaderDesc::ST_Box2Shadow,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2Shadow,
    /* Uniforms */      Uniforms_2478189079,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowMul,
    /* Uniforms */      Uniforms_2478189079,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowKnockout = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowKnockout,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowKnockout,
    /* Uniforms */      Uniforms_2478189079,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowKnockoutMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowKnockoutMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowKnockoutMul,
    /* Uniforms */      Uniforms_2478189079,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2InnerShadow = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadow,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2InnerShadow,
    /* Uniforms */      Uniforms_2478189079,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2InnerShadowMul = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2InnerShadowMul,
    /* Uniforms */      Uniforms_2478189079,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2InnerShadowKnockout = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowKnockout,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2InnerShadowKnockout,
    /* Uniforms */      Uniforms_2478189079,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2InnerShadowKnockoutMul = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowKnockoutMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2InnerShadowKnockoutMul,
    /* Uniforms */      Uniforms_2478189079,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2Shadowonly = 
{
    /* Type */          ShaderDesc::ST_Box2Shadowonly,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2Shadowonly,
    /* Uniforms */      Uniforms_2286958232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowonlyMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowonlyMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowonlyMul,
    /* Uniforms */      Uniforms_2286958232,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowHighlight = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowHighlight,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowHighlight,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowHighlightMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowHighlightMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowHighlightMul,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowHighlightKnockout = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowHighlightKnockout,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowHighlightKnockout,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowHighlightKnockoutMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowHighlightKnockoutMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowHighlightKnockoutMul,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2InnerShadowHighlight = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowHighlight,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2InnerShadowHighlight,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2InnerShadowHighlightMul = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowHighlightMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2InnerShadowHighlightMul,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2InnerShadowHighlightKnockout = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowHighlightKnockout,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2InnerShadowHighlightKnockout,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2InnerShadowHighlightKnockoutMul = 
{
    /* Type */          ShaderDesc::ST_Box2InnerShadowHighlightKnockoutMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2InnerShadowHighlightKnockoutMul,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2FullShadowHighlight = 
{
    /* Type */          ShaderDesc::ST_Box2FullShadowHighlight,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2FullShadowHighlight,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2FullShadowHighlightMul = 
{
    /* Type */          ShaderDesc::ST_Box2FullShadowHighlightMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2FullShadowHighlightMul,
    /* Uniforms */      Uniforms_3893930098,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowonlyHighlight = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowonlyHighlight,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowonlyHighlight,
    /* Uniforms */      Uniforms_6259487864,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FBox2ShadowonlyHighlightMul = 
{
    /* Type */          ShaderDesc::ST_Box2ShadowonlyHighlightMul,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         Shader_DynamicLoop,
    /* pBinary */       pBinary_D3D9SM30_FBox2ShadowonlyHighlightMul,
    /* Uniforms */      Uniforms_6259487864,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawableCopyPixels = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixels,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawableCopyPixels,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsMergeAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsMergeAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawableCopyPixelsMergeAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsNoDestAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsNoDestAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawableCopyPixelsNoDestAlpha,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawableCopyPixelsAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsAlphaMergeAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlphaMergeAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawableCopyPixelsAlphaMergeAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsAlphaNoDestAlpha = 
{
    /* Type */          ShaderDesc::ST_DrawableCopyPixelsAlphaNoDestAlpha,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawableCopyPixelsAlphaNoDestAlpha,
    /* Uniforms */      Uniforms_5642581497,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawableMerge = 
{
    /* Type */          ShaderDesc::ST_DrawableMerge,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawableMerge,
    /* Uniforms */      Uniforms_3033064606,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawableCompare = 
{
    /* Type */          ShaderDesc::ST_DrawableCompare,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawableCompare,
    /* Uniforms */      Uniforms_4895524107,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

static FragShaderDesc ShaderDesc_FS_D3D9SM30_FDrawablePaletteMap = 
{
    /* Type */          ShaderDesc::ST_DrawablePaletteMap,
    /* Version */       ShaderDesc::ShaderVersion_D3D9SM30,
    /* Flags */         0,
    /* pBinary */       pBinary_D3D9SM30_FDrawablePaletteMap,
    /* Uniforms */      Uniforms_3820893743,
    /* BatchUniforms */ BatchUniforms_6184305142,
    
};

const FragShaderDesc* FragShaderDesc::Descs[FSI_Count] = {
    0,
    &ShaderDesc_FS_D3D9SM20_FTexTG,                              // 1
    &ShaderDesc_FS_D3D9SM20_FTexTGMul,                           // 2
    &ShaderDesc_FS_D3D9SM20_FTexTGInv,                           // 3
    &ShaderDesc_FS_D3D9SM20_FTexTGEAlpha,                        // 4
    &ShaderDesc_FS_D3D9SM20_FTexTGEAlphaMul,                     // 5
    &ShaderDesc_FS_D3D9SM20_FTexTGEAlphaInv,                     // 6
    &ShaderDesc_FS_D3D9SM20_FTexTGCxform,                        // 7
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformMul,                     // 8
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformInv,                     // 9
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformEAlpha,                  // 10
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformEAlphaMul,               // 11
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformEAlphaInv,               // 12
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformAc,                      // 13
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformAcMul,                   // 14
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformAcInv,                   // 15
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformAcEAlpha,                // 16
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformAcEAlphaMul,             // 17
    &ShaderDesc_FS_D3D9SM20_FTexTGCxformAcEAlphaInv,             // 18
    &ShaderDesc_FS_D3D9SM20_FVertex,                             // 19
    &ShaderDesc_FS_D3D9SM20_FVertexMul,                          // 20
    &ShaderDesc_FS_D3D9SM20_FVertexInv,                          // 21
    &ShaderDesc_FS_D3D9SM20_FVertexEAlpha,                       // 22
    &ShaderDesc_FS_D3D9SM20_FVertexEAlphaMul,                    // 23
    &ShaderDesc_FS_D3D9SM20_FVertexEAlphaInv,                    // 24
    &ShaderDesc_FS_D3D9SM20_FVertexCxform,                       // 25
    &ShaderDesc_FS_D3D9SM20_FVertexCxformMul,                    // 26
    &ShaderDesc_FS_D3D9SM20_FVertexCxformInv,                    // 27
    &ShaderDesc_FS_D3D9SM20_FVertexCxformEAlpha,                 // 28
    &ShaderDesc_FS_D3D9SM20_FVertexCxformEAlphaMul,              // 29
    &ShaderDesc_FS_D3D9SM20_FVertexCxformEAlphaInv,              // 30
    &ShaderDesc_FS_D3D9SM20_FVertexCxformAc,                     // 31
    &ShaderDesc_FS_D3D9SM20_FVertexCxformAcMul,                  // 32
    &ShaderDesc_FS_D3D9SM20_FVertexCxformAcInv,                  // 33
    &ShaderDesc_FS_D3D9SM20_FVertexCxformAcEAlpha,               // 34
    &ShaderDesc_FS_D3D9SM20_FVertexCxformAcEAlphaMul,            // 35
    &ShaderDesc_FS_D3D9SM20_FVertexCxformAcEAlphaInv,            // 36
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTG,                         // 37
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGMul,                      // 38
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGInv,                      // 39
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGEAlpha,                   // 40
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGEAlphaMul,                // 41
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGEAlphaInv,                // 42
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxform,                   // 43
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformMul,                // 44
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformInv,                // 45
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformEAlpha,             // 46
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformEAlphaMul,          // 47
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformEAlphaInv,          // 48
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAc,                 // 49
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcMul,              // 50
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcInv,              // 51
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcEAlpha,           // 52
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcEAlphaMul,        // 53
    &ShaderDesc_FS_D3D9SM20_FTexTGTexTGCxformAcEAlphaInv,        // 54
    &ShaderDesc_FS_D3D9SM20_FTexTGVertex,                        // 55
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexMul,                     // 56
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexInv,                     // 57
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexEAlpha,                  // 58
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexEAlphaMul,               // 59
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexEAlphaInv,               // 60
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxform,                  // 61
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformMul,               // 62
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformInv,               // 63
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformEAlpha,            // 64
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformEAlphaMul,         // 65
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformEAlphaInv,         // 66
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAc,                // 67
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcMul,             // 68
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcInv,             // 69
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcEAlpha,          // 70
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcEAlphaMul,       // 71
    &ShaderDesc_FS_D3D9SM20_FTexTGVertexCxformAcEAlphaInv,       // 72
    &ShaderDesc_FS_D3D9SM20_FBatchTexTG,                         // 73
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGMul,                      // 74
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGInv,                      // 75
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGEAlpha,                   // 76
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGEAlphaMul,                // 77
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGEAlphaInv,                // 78
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxform,                   // 79
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformMul,                // 80
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformInv,                // 81
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformEAlpha,             // 82
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformEAlphaMul,          // 83
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformEAlphaInv,          // 84
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAc,                 // 85
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcMul,              // 86
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcInv,              // 87
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcEAlpha,           // 88
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcEAlphaMul,        // 89
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGCxformAcEAlphaInv,        // 90
    &ShaderDesc_FS_D3D9SM20_FBatchVertex,                        // 91
    &ShaderDesc_FS_D3D9SM20_FBatchVertexMul,                     // 92
    &ShaderDesc_FS_D3D9SM20_FBatchVertexInv,                     // 93
    &ShaderDesc_FS_D3D9SM20_FBatchVertexEAlpha,                  // 94
    &ShaderDesc_FS_D3D9SM20_FBatchVertexEAlphaMul,               // 95
    &ShaderDesc_FS_D3D9SM20_FBatchVertexEAlphaInv,               // 96
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxform,                  // 97
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformMul,               // 98
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformInv,               // 99
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformEAlpha,            // 100
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformEAlphaMul,         // 101
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformEAlphaInv,         // 102
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAc,                // 103
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcMul,             // 104
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcInv,             // 105
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcEAlpha,          // 106
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcEAlphaMul,       // 107
    &ShaderDesc_FS_D3D9SM20_FBatchVertexCxformAcEAlphaInv,       // 108
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTG,                    // 109
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGMul,                 // 110
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGInv,                 // 111
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGEAlpha,              // 112
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGEAlphaMul,           // 113
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGEAlphaInv,           // 114
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxform,              // 115
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformMul,           // 116
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformInv,           // 117
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformEAlpha,        // 118
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformEAlphaMul,     // 119
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformEAlphaInv,     // 120
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAc,            // 121
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcMul,         // 122
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcInv,         // 123
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcEAlpha,      // 124
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcEAlphaMul,   // 125
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGTexTGCxformAcEAlphaInv,   // 126
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertex,                   // 127
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexMul,                // 128
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexInv,                // 129
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexEAlpha,             // 130
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexEAlphaMul,          // 131
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexEAlphaInv,          // 132
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxform,             // 133
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformMul,          // 134
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformInv,          // 135
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformEAlpha,       // 136
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformEAlphaMul,    // 137
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformEAlphaInv,    // 138
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAc,           // 139
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcMul,        // 140
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcInv,        // 141
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcEAlpha,     // 142
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcEAlphaMul,  // 143
    &ShaderDesc_FS_D3D9SM20_FBatchTexTGVertexCxformAcEAlphaInv,  // 144
    &ShaderDesc_FS_D3D9SM20_FSolid,                              // 145
    &ShaderDesc_FS_D3D9SM20_FSolidMul,                           // 146
    &ShaderDesc_FS_D3D9SM20_FSolidInv,                           // 147
    &ShaderDesc_FS_D3D9SM20_FText,                               // 148
    &ShaderDesc_FS_D3D9SM20_FTextMul,                            // 149
    &ShaderDesc_FS_D3D9SM20_FTextInv,                            // 150
    &ShaderDesc_FS_D3D9SM20_FBatchSolid,                         // 151
    &ShaderDesc_FS_D3D9SM20_FBatchSolidMul,                      // 152
    &ShaderDesc_FS_D3D9SM20_FBatchSolidInv,                      // 153
    &ShaderDesc_FS_D3D9SM20_FBatchText,                          // 154
    &ShaderDesc_FS_D3D9SM20_FBatchTextMul,                       // 155
    &ShaderDesc_FS_D3D9SM20_FBatchTextInv,                       // 156
    &ShaderDesc_FS_D3D9SM20_FYUV,                                // 157
    &ShaderDesc_FS_D3D9SM20_FYUVMul,                             // 158
    &ShaderDesc_FS_D3D9SM20_FYUVEAlpha,                          // 159
    &ShaderDesc_FS_D3D9SM20_FYUVEAlphaMul,                       // 160
    &ShaderDesc_FS_D3D9SM20_FYUVCxform,                          // 161
    &ShaderDesc_FS_D3D9SM20_FYUVCxformMul,                       // 162
    &ShaderDesc_FS_D3D9SM20_FYUVCxformEAlpha,                    // 163
    &ShaderDesc_FS_D3D9SM20_FYUVCxformEAlphaMul,                 // 164
    &ShaderDesc_FS_D3D9SM20_FYUVCxformAc,                        // 165
    &ShaderDesc_FS_D3D9SM20_FYUVCxformAcMul,                     // 166
    &ShaderDesc_FS_D3D9SM20_FYUVCxformAcEAlpha,                  // 167
    &ShaderDesc_FS_D3D9SM20_FYUVCxformAcEAlphaMul,               // 168
    &ShaderDesc_FS_D3D9SM20_FYUVA,                               // 169
    &ShaderDesc_FS_D3D9SM20_FYUVAMul,                            // 170
    &ShaderDesc_FS_D3D9SM20_FYUVAInv,                            // 171
    &ShaderDesc_FS_D3D9SM20_FYUVAEAlpha,                         // 172
    &ShaderDesc_FS_D3D9SM20_FYUVAEAlphaMul,                      // 173
    &ShaderDesc_FS_D3D9SM20_FYUVAEAlphaInv,                      // 174
    &ShaderDesc_FS_D3D9SM20_FYUVACxform,                         // 175
    &ShaderDesc_FS_D3D9SM20_FYUVACxformMul,                      // 176
    &ShaderDesc_FS_D3D9SM20_FYUVACxformInv,                      // 177
    &ShaderDesc_FS_D3D9SM20_FYUVACxformEAlpha,                   // 178
    &ShaderDesc_FS_D3D9SM20_FYUVACxformEAlphaMul,                // 179
    &ShaderDesc_FS_D3D9SM20_FYUVACxformEAlphaInv,                // 180
    &ShaderDesc_FS_D3D9SM20_FYUVACxformAc,                       // 181
    &ShaderDesc_FS_D3D9SM20_FYUVACxformAcMul,                    // 182
    &ShaderDesc_FS_D3D9SM20_FYUVACxformAcInv,                    // 183
    &ShaderDesc_FS_D3D9SM20_FYUVACxformAcEAlpha,                 // 184
    &ShaderDesc_FS_D3D9SM20_FYUVACxformAcEAlphaMul,              // 185
    &ShaderDesc_FS_D3D9SM20_FYUVACxformAcEAlphaInv,              // 186
    &ShaderDesc_FS_D3D9SM20_FBatchYUV,                           // 187
    &ShaderDesc_FS_D3D9SM20_FBatchYUVMul,                        // 188
    &ShaderDesc_FS_D3D9SM20_FBatchYUVEAlpha,                     // 189
    &ShaderDesc_FS_D3D9SM20_FBatchYUVEAlphaMul,                  // 190
    &ShaderDesc_FS_D3D9SM20_FBatchYUVCxform,                     // 191
    &ShaderDesc_FS_D3D9SM20_FBatchYUVCxformMul,                  // 192
    &ShaderDesc_FS_D3D9SM20_FBatchYUVCxformEAlpha,               // 193
    &ShaderDesc_FS_D3D9SM20_FBatchYUVCxformEAlphaMul,            // 194
    &ShaderDesc_FS_D3D9SM20_FBatchYUVCxformAc,                   // 195
    &ShaderDesc_FS_D3D9SM20_FBatchYUVCxformAcMul,                // 196
    &ShaderDesc_FS_D3D9SM20_FBatchYUVCxformAcEAlpha,             // 197
    &ShaderDesc_FS_D3D9SM20_FBatchYUVCxformAcEAlphaMul,          // 198
    &ShaderDesc_FS_D3D9SM20_FBatchYUVA,                          // 199
    &ShaderDesc_FS_D3D9SM20_FBatchYUVAMul,                       // 200
    &ShaderDesc_FS_D3D9SM20_FBatchYUVAInv,                       // 201
    &ShaderDesc_FS_D3D9SM20_FBatchYUVAEAlpha,                    // 202
    &ShaderDesc_FS_D3D9SM20_FBatchYUVAEAlphaMul,                 // 203
    &ShaderDesc_FS_D3D9SM20_FBatchYUVAEAlphaInv,                 // 204
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxform,                    // 205
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformMul,                 // 206
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformInv,                 // 207
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformEAlpha,              // 208
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformEAlphaMul,           // 209
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformEAlphaInv,           // 210
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAc,                  // 211
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcMul,               // 212
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcInv,               // 213
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcEAlpha,            // 214
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcEAlphaMul,         // 215
    &ShaderDesc_FS_D3D9SM20_FBatchYUVACxformAcEAlphaInv,         // 216
    &ShaderDesc_FS_D3D9SM20_FTexTGCMatrixAc,                     // 217
    &ShaderDesc_FS_D3D9SM20_FTexTGCMatrixAcMul,                  // 218
    &ShaderDesc_FS_D3D9SM20_FTexTGCMatrixAcEAlpha,               // 219
    &ShaderDesc_FS_D3D9SM20_FTexTGCMatrixAcEAlphaMul,            // 220
    &ShaderDesc_FS_D3D9SM20_FDrawableCopyPixels,                 // 221
    &ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsMergeAlpha,       // 222
    &ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsNoDestAlpha,      // 223
    &ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsAlpha,            // 224
    &ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsAlphaMergeAlpha,  // 225
    &ShaderDesc_FS_D3D9SM20_FDrawableCopyPixelsAlphaNoDestAlpha, // 226
    &ShaderDesc_FS_D3D9SM20_FDrawableMerge,                      // 227
    &ShaderDesc_FS_D3D9SM20_FDrawableCompare,                    // 228
    &ShaderDesc_FS_D3D9SM20_FDrawablePaletteMap,                 // 229
    &ShaderDesc_FS_D3D9SM30_FTexTG,                              // 230
    &ShaderDesc_FS_D3D9SM30_FTexTGMul,                           // 231
    &ShaderDesc_FS_D3D9SM30_FTexTGInv,                           // 232
    &ShaderDesc_FS_D3D9SM30_FTexTGEAlpha,                        // 233
    &ShaderDesc_FS_D3D9SM30_FTexTGEAlphaMul,                     // 234
    &ShaderDesc_FS_D3D9SM30_FTexTGEAlphaInv,                     // 235
    &ShaderDesc_FS_D3D9SM30_FTexTGCxform,                        // 236
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformMul,                     // 237
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformInv,                     // 238
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformEAlpha,                  // 239
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformEAlphaMul,               // 240
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformEAlphaInv,               // 241
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformAc,                      // 242
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformAcMul,                   // 243
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformAcInv,                   // 244
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformAcEAlpha,                // 245
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformAcEAlphaMul,             // 246
    &ShaderDesc_FS_D3D9SM30_FTexTGCxformAcEAlphaInv,             // 247
    &ShaderDesc_FS_D3D9SM30_FVertex,                             // 248
    &ShaderDesc_FS_D3D9SM30_FVertexMul,                          // 249
    &ShaderDesc_FS_D3D9SM30_FVertexInv,                          // 250
    &ShaderDesc_FS_D3D9SM30_FVertexEAlpha,                       // 251
    &ShaderDesc_FS_D3D9SM30_FVertexEAlphaMul,                    // 252
    &ShaderDesc_FS_D3D9SM30_FVertexEAlphaInv,                    // 253
    &ShaderDesc_FS_D3D9SM30_FVertexCxform,                       // 254
    &ShaderDesc_FS_D3D9SM30_FVertexCxformMul,                    // 255
    &ShaderDesc_FS_D3D9SM30_FVertexCxformInv,                    // 256
    &ShaderDesc_FS_D3D9SM30_FVertexCxformEAlpha,                 // 257
    &ShaderDesc_FS_D3D9SM30_FVertexCxformEAlphaMul,              // 258
    &ShaderDesc_FS_D3D9SM30_FVertexCxformEAlphaInv,              // 259
    &ShaderDesc_FS_D3D9SM30_FVertexCxformAc,                     // 260
    &ShaderDesc_FS_D3D9SM30_FVertexCxformAcMul,                  // 261
    &ShaderDesc_FS_D3D9SM30_FVertexCxformAcInv,                  // 262
    &ShaderDesc_FS_D3D9SM30_FVertexCxformAcEAlpha,               // 263
    &ShaderDesc_FS_D3D9SM30_FVertexCxformAcEAlphaMul,            // 264
    &ShaderDesc_FS_D3D9SM30_FVertexCxformAcEAlphaInv,            // 265
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTG,                         // 266
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGMul,                      // 267
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGInv,                      // 268
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGEAlpha,                   // 269
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGEAlphaMul,                // 270
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGEAlphaInv,                // 271
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxform,                   // 272
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformMul,                // 273
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformInv,                // 274
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformEAlpha,             // 275
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformEAlphaMul,          // 276
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformEAlphaInv,          // 277
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAc,                 // 278
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcMul,              // 279
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcInv,              // 280
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcEAlpha,           // 281
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcEAlphaMul,        // 282
    &ShaderDesc_FS_D3D9SM30_FTexTGTexTGCxformAcEAlphaInv,        // 283
    &ShaderDesc_FS_D3D9SM30_FTexTGVertex,                        // 284
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexMul,                     // 285
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexInv,                     // 286
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexEAlpha,                  // 287
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexEAlphaMul,               // 288
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexEAlphaInv,               // 289
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxform,                  // 290
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformMul,               // 291
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformInv,               // 292
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformEAlpha,            // 293
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformEAlphaMul,         // 294
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformEAlphaInv,         // 295
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAc,                // 296
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcMul,             // 297
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcInv,             // 298
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcEAlpha,          // 299
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcEAlphaMul,       // 300
    &ShaderDesc_FS_D3D9SM30_FTexTGVertexCxformAcEAlphaInv,       // 301
    &ShaderDesc_FS_D3D9SM30_FBatchTexTG,                         // 302
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGMul,                      // 303
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGInv,                      // 304
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGEAlpha,                   // 305
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGEAlphaMul,                // 306
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGEAlphaInv,                // 307
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxform,                   // 308
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformMul,                // 309
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformInv,                // 310
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformEAlpha,             // 311
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformEAlphaMul,          // 312
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformEAlphaInv,          // 313
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAc,                 // 314
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcMul,              // 315
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcInv,              // 316
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcEAlpha,           // 317
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcEAlphaMul,        // 318
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGCxformAcEAlphaInv,        // 319
    &ShaderDesc_FS_D3D9SM30_FBatchVertex,                        // 320
    &ShaderDesc_FS_D3D9SM30_FBatchVertexMul,                     // 321
    &ShaderDesc_FS_D3D9SM30_FBatchVertexInv,                     // 322
    &ShaderDesc_FS_D3D9SM30_FBatchVertexEAlpha,                  // 323
    &ShaderDesc_FS_D3D9SM30_FBatchVertexEAlphaMul,               // 324
    &ShaderDesc_FS_D3D9SM30_FBatchVertexEAlphaInv,               // 325
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxform,                  // 326
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformMul,               // 327
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformInv,               // 328
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformEAlpha,            // 329
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformEAlphaMul,         // 330
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformEAlphaInv,         // 331
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAc,                // 332
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcMul,             // 333
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcInv,             // 334
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcEAlpha,          // 335
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcEAlphaMul,       // 336
    &ShaderDesc_FS_D3D9SM30_FBatchVertexCxformAcEAlphaInv,       // 337
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTG,                    // 338
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGMul,                 // 339
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGInv,                 // 340
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGEAlpha,              // 341
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGEAlphaMul,           // 342
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGEAlphaInv,           // 343
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxform,              // 344
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformMul,           // 345
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformInv,           // 346
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformEAlpha,        // 347
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformEAlphaMul,     // 348
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformEAlphaInv,     // 349
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAc,            // 350
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcMul,         // 351
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcInv,         // 352
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcEAlpha,      // 353
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcEAlphaMul,   // 354
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGTexTGCxformAcEAlphaInv,   // 355
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertex,                   // 356
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexMul,                // 357
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexInv,                // 358
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexEAlpha,             // 359
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexEAlphaMul,          // 360
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexEAlphaInv,          // 361
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxform,             // 362
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformMul,          // 363
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformInv,          // 364
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformEAlpha,       // 365
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformEAlphaMul,    // 366
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformEAlphaInv,    // 367
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAc,           // 368
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcMul,        // 369
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcInv,        // 370
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcEAlpha,     // 371
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcEAlphaMul,  // 372
    &ShaderDesc_FS_D3D9SM30_FBatchTexTGVertexCxformAcEAlphaInv,  // 373
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTG,                     // 374
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGMul,                  // 375
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGInv,                  // 376
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGEAlpha,               // 377
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGEAlphaMul,            // 378
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGEAlphaInv,            // 379
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxform,               // 380
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformMul,            // 381
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformInv,            // 382
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformEAlpha,         // 383
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformEAlphaMul,      // 384
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformEAlphaInv,      // 385
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAc,             // 386
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcMul,          // 387
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcInv,          // 388
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcEAlpha,       // 389
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcEAlphaMul,    // 390
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGCxformAcEAlphaInv,    // 391
    &ShaderDesc_FS_D3D9SM30_FInstancedVertex,                    // 392
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexMul,                 // 393
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexInv,                 // 394
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexEAlpha,              // 395
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexEAlphaMul,           // 396
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexEAlphaInv,           // 397
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxform,              // 398
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformMul,           // 399
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformInv,           // 400
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformEAlpha,        // 401
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformEAlphaMul,     // 402
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformEAlphaInv,     // 403
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAc,            // 404
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcMul,         // 405
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcInv,         // 406
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcEAlpha,      // 407
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcEAlphaMul,   // 408
    &ShaderDesc_FS_D3D9SM30_FInstancedVertexCxformAcEAlphaInv,   // 409
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTG,                // 410
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGMul,             // 411
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGInv,             // 412
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGEAlpha,          // 413
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGEAlphaMul,       // 414
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGEAlphaInv,       // 415
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxform,          // 416
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformMul,       // 417
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformInv,       // 418
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformEAlpha,    // 419
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformEAlphaMul, // 420
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformEAlphaInv, // 421
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAc,        // 422
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcMul,     // 423
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcInv,     // 424
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlpha,  // 425
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlphaMul, // 426
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGTexTGCxformAcEAlphaInv, // 427
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertex,               // 428
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexMul,            // 429
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexInv,            // 430
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexEAlpha,         // 431
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexEAlphaMul,      // 432
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexEAlphaInv,      // 433
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxform,         // 434
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformMul,      // 435
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformInv,      // 436
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformEAlpha,   // 437
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformEAlphaMul, // 438
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformEAlphaInv, // 439
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAc,       // 440
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcMul,    // 441
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcInv,    // 442
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcEAlpha, // 443
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcEAlphaMul, // 444
    &ShaderDesc_FS_D3D9SM30_FInstancedTexTGVertexCxformAcEAlphaInv, // 445
    &ShaderDesc_FS_D3D9SM30_FSolid,                              // 446
    &ShaderDesc_FS_D3D9SM30_FSolidMul,                           // 447
    &ShaderDesc_FS_D3D9SM30_FSolidInv,                           // 448
    &ShaderDesc_FS_D3D9SM30_FText,                               // 449
    &ShaderDesc_FS_D3D9SM30_FTextMul,                            // 450
    &ShaderDesc_FS_D3D9SM30_FTextInv,                            // 451
    &ShaderDesc_FS_D3D9SM30_FBatchSolid,                         // 452
    &ShaderDesc_FS_D3D9SM30_FBatchSolidMul,                      // 453
    &ShaderDesc_FS_D3D9SM30_FBatchSolidInv,                      // 454
    &ShaderDesc_FS_D3D9SM30_FBatchText,                          // 455
    &ShaderDesc_FS_D3D9SM30_FBatchTextMul,                       // 456
    &ShaderDesc_FS_D3D9SM30_FBatchTextInv,                       // 457
    &ShaderDesc_FS_D3D9SM30_FInstancedSolid,                     // 458
    &ShaderDesc_FS_D3D9SM30_FInstancedSolidMul,                  // 459
    &ShaderDesc_FS_D3D9SM30_FInstancedSolidInv,                  // 460
    &ShaderDesc_FS_D3D9SM30_FInstancedText,                      // 461
    &ShaderDesc_FS_D3D9SM30_FInstancedTextMul,                   // 462
    &ShaderDesc_FS_D3D9SM30_FInstancedTextInv,                   // 463
    &ShaderDesc_FS_D3D9SM30_FYUV,                                // 464
    &ShaderDesc_FS_D3D9SM30_FYUVMul,                             // 465
    &ShaderDesc_FS_D3D9SM30_FYUVEAlpha,                          // 466
    &ShaderDesc_FS_D3D9SM30_FYUVEAlphaMul,                       // 467
    &ShaderDesc_FS_D3D9SM30_FYUVCxform,                          // 468
    &ShaderDesc_FS_D3D9SM30_FYUVCxformMul,                       // 469
    &ShaderDesc_FS_D3D9SM30_FYUVCxformEAlpha,                    // 470
    &ShaderDesc_FS_D3D9SM30_FYUVCxformEAlphaMul,                 // 471
    &ShaderDesc_FS_D3D9SM30_FYUVCxformAc,                        // 472
    &ShaderDesc_FS_D3D9SM30_FYUVCxformAcMul,                     // 473
    &ShaderDesc_FS_D3D9SM30_FYUVCxformAcEAlpha,                  // 474
    &ShaderDesc_FS_D3D9SM30_FYUVCxformAcEAlphaMul,               // 475
    &ShaderDesc_FS_D3D9SM30_FYUVA,                               // 476
    &ShaderDesc_FS_D3D9SM30_FYUVAMul,                            // 477
    &ShaderDesc_FS_D3D9SM30_FYUVAInv,                            // 478
    &ShaderDesc_FS_D3D9SM30_FYUVAEAlpha,                         // 479
    &ShaderDesc_FS_D3D9SM30_FYUVAEAlphaMul,                      // 480
    &ShaderDesc_FS_D3D9SM30_FYUVAEAlphaInv,                      // 481
    &ShaderDesc_FS_D3D9SM30_FYUVACxform,                         // 482
    &ShaderDesc_FS_D3D9SM30_FYUVACxformMul,                      // 483
    &ShaderDesc_FS_D3D9SM30_FYUVACxformInv,                      // 484
    &ShaderDesc_FS_D3D9SM30_FYUVACxformEAlpha,                   // 485
    &ShaderDesc_FS_D3D9SM30_FYUVACxformEAlphaMul,                // 486
    &ShaderDesc_FS_D3D9SM30_FYUVACxformEAlphaInv,                // 487
    &ShaderDesc_FS_D3D9SM30_FYUVACxformAc,                       // 488
    &ShaderDesc_FS_D3D9SM30_FYUVACxformAcMul,                    // 489
    &ShaderDesc_FS_D3D9SM30_FYUVACxformAcInv,                    // 490
    &ShaderDesc_FS_D3D9SM30_FYUVACxformAcEAlpha,                 // 491
    &ShaderDesc_FS_D3D9SM30_FYUVACxformAcEAlphaMul,              // 492
    &ShaderDesc_FS_D3D9SM30_FYUVACxformAcEAlphaInv,              // 493
    &ShaderDesc_FS_D3D9SM30_FBatchYUV,                           // 494
    &ShaderDesc_FS_D3D9SM30_FBatchYUVMul,                        // 495
    &ShaderDesc_FS_D3D9SM30_FBatchYUVEAlpha,                     // 496
    &ShaderDesc_FS_D3D9SM30_FBatchYUVEAlphaMul,                  // 497
    &ShaderDesc_FS_D3D9SM30_FBatchYUVCxform,                     // 498
    &ShaderDesc_FS_D3D9SM30_FBatchYUVCxformMul,                  // 499
    &ShaderDesc_FS_D3D9SM30_FBatchYUVCxformEAlpha,               // 500
    &ShaderDesc_FS_D3D9SM30_FBatchYUVCxformEAlphaMul,            // 501
    &ShaderDesc_FS_D3D9SM30_FBatchYUVCxformAc,                   // 502
    &ShaderDesc_FS_D3D9SM30_FBatchYUVCxformAcMul,                // 503
    &ShaderDesc_FS_D3D9SM30_FBatchYUVCxformAcEAlpha,             // 504
    &ShaderDesc_FS_D3D9SM30_FBatchYUVCxformAcEAlphaMul,          // 505
    &ShaderDesc_FS_D3D9SM30_FBatchYUVA,                          // 506
    &ShaderDesc_FS_D3D9SM30_FBatchYUVAMul,                       // 507
    &ShaderDesc_FS_D3D9SM30_FBatchYUVAInv,                       // 508
    &ShaderDesc_FS_D3D9SM30_FBatchYUVAEAlpha,                    // 509
    &ShaderDesc_FS_D3D9SM30_FBatchYUVAEAlphaMul,                 // 510
    &ShaderDesc_FS_D3D9SM30_FBatchYUVAEAlphaInv,                 // 511
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxform,                    // 512
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformMul,                 // 513
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformInv,                 // 514
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformEAlpha,              // 515
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformEAlphaMul,           // 516
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformEAlphaInv,           // 517
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAc,                  // 518
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcMul,               // 519
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcInv,               // 520
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcEAlpha,            // 521
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcEAlphaMul,         // 522
    &ShaderDesc_FS_D3D9SM30_FBatchYUVACxformAcEAlphaInv,         // 523
    &ShaderDesc_FS_D3D9SM30_FInstancedYUV,                       // 524
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVMul,                    // 525
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVEAlpha,                 // 526
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVEAlphaMul,              // 527
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVCxform,                 // 528
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformMul,              // 529
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformEAlpha,           // 530
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformEAlphaMul,        // 531
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformAc,               // 532
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformAcMul,            // 533
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformAcEAlpha,         // 534
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVCxformAcEAlphaMul,      // 535
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVA,                      // 536
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVAMul,                   // 537
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVAInv,                   // 538
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVAEAlpha,                // 539
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVAEAlphaMul,             // 540
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVAEAlphaInv,             // 541
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxform,                // 542
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformMul,             // 543
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformInv,             // 544
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformEAlpha,          // 545
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformEAlphaMul,       // 546
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformEAlphaInv,       // 547
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAc,              // 548
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcMul,           // 549
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcInv,           // 550
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcEAlpha,        // 551
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcEAlphaMul,     // 552
    &ShaderDesc_FS_D3D9SM30_FInstancedYUVACxformAcEAlphaInv,     // 553
    &ShaderDesc_FS_D3D9SM30_FTexTGCMatrixAc,                     // 554
    &ShaderDesc_FS_D3D9SM30_FTexTGCMatrixAcMul,                  // 555
    &ShaderDesc_FS_D3D9SM30_FTexTGCMatrixAcEAlpha,               // 556
    &ShaderDesc_FS_D3D9SM30_FTexTGCMatrixAcEAlphaMul,            // 557
    &ShaderDesc_FS_D3D9SM30_FBox1Blur,                           // 558
    &ShaderDesc_FS_D3D9SM30_FBox1BlurMul,                        // 559
    &ShaderDesc_FS_D3D9SM30_FBox2Blur,                           // 560
    &ShaderDesc_FS_D3D9SM30_FBox2BlurMul,                        // 561
    &ShaderDesc_FS_D3D9SM30_FBox2Shadow,                         // 562
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowMul,                      // 563
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowKnockout,                 // 564
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowKnockoutMul,              // 565
    &ShaderDesc_FS_D3D9SM30_FBox2InnerShadow,                    // 566
    &ShaderDesc_FS_D3D9SM30_FBox2InnerShadowMul,                 // 567
    &ShaderDesc_FS_D3D9SM30_FBox2InnerShadowKnockout,            // 568
    &ShaderDesc_FS_D3D9SM30_FBox2InnerShadowKnockoutMul,         // 569
    &ShaderDesc_FS_D3D9SM30_FBox2Shadowonly,                     // 570
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowonlyMul,                  // 571
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowHighlight,                // 572
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowHighlightMul,             // 573
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowHighlightKnockout,        // 574
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowHighlightKnockoutMul,     // 575
    &ShaderDesc_FS_D3D9SM30_FBox2InnerShadowHighlight,           // 576
    &ShaderDesc_FS_D3D9SM30_FBox2InnerShadowHighlightMul,        // 577
    &ShaderDesc_FS_D3D9SM30_FBox2InnerShadowHighlightKnockout,   // 578
    &ShaderDesc_FS_D3D9SM30_FBox2InnerShadowHighlightKnockoutMul, // 579
    &ShaderDesc_FS_D3D9SM30_FBox2FullShadowHighlight,            // 580
    &ShaderDesc_FS_D3D9SM30_FBox2FullShadowHighlightMul,         // 581
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowonlyHighlight,            // 582
    &ShaderDesc_FS_D3D9SM30_FBox2ShadowonlyHighlightMul,         // 583
    &ShaderDesc_FS_D3D9SM30_FDrawableCopyPixels,                 // 584
    &ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsMergeAlpha,       // 585
    &ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsNoDestAlpha,      // 586
    &ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsAlpha,            // 587
    &ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsAlphaMergeAlpha,  // 588
    &ShaderDesc_FS_D3D9SM30_FDrawableCopyPixelsAlphaNoDestAlpha, // 589
    &ShaderDesc_FS_D3D9SM30_FDrawableMerge,                      // 590
    &ShaderDesc_FS_D3D9SM30_FDrawableCompare,                    // 591
    &ShaderDesc_FS_D3D9SM30_FDrawablePaletteMap,                 // 592
};

const FragShaderDesc* FragShaderDesc::GetDesc(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    return Descs[GetShaderIndex(shader, ver)];
};

FragShaderDesc::ShaderIndex FragShaderDesc::GetShaderIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    ShaderDesc::ShaderIndex index = ShaderDesc::GetShaderIndex(shader);
    return ShaderIndexingData[index][ver]. FragIndex;
};

FragShaderDesc::ShaderIndex FragShaderDesc::GetShaderIndexForComboIndex(unsigned index, ShaderDesc::ShaderVersion ver) 
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_D3D9SM20:
        {
            SF_UNUSED(index);
            SF_DEBUG_ASSERT(1, "D3D9SM20 indicated that ShaderCombo information was not required.");
            return FSI_None;
        }
        case ShaderDesc::ShaderVersion_D3D9SM30:
        {
            SF_UNUSED(index);
            SF_DEBUG_ASSERT(1, "D3D9SM30 indicated that ShaderCombo information was not required.");
            return FSI_None;
        }
        default: SF_ASSERT(0); return FSI_None;
    }
}

unsigned FragShaderDesc::GetShaderComboIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)
{
    switch(ver)
    {
        case ShaderDesc::ShaderVersion_D3D9SM20:
        {
            SF_UNUSED(shader);
            SF_DEBUG_ASSERT(1, "D3D9SM20 indicated that ShaderCombo information was not required.");
            return 0;
        }
        case ShaderDesc::ShaderVersion_D3D9SM30:
        {
            SF_UNUSED(shader);
            SF_DEBUG_ASSERT(1, "D3D9SM30 indicated that ShaderCombo information was not required.");
            return 0;
        }
        default: SF_ASSERT(0); return 0;
    }
}

}}} // Scaleform::Render::D3D9

