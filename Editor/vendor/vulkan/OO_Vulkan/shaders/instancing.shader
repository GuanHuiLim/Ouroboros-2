#ifndef _INSTANCING_SHADER_H_
#define _INSTANCING_SHADER_H_

mat4x4 GPUTransformToMatrix4x4(const in GPUTransform m)
{
    return mat4x4(m.row0.x, m.row1.x, m.row2.x, 0.0,
                  m.row0.y, m.row1.y, m.row2.y, 0.0,
                  m.row0.z, m.row1.z, m.row2.z, 0.0,
                  m.row0.w, m.row1.w, m.row2.w, 1.0);
}

#endif//INCLUDE_GUARD
