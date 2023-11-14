#ifndef _INSTANCING_SHADER_H_
#define _INSTANCING_SHADER_H_

mat4x4 GPUTransformToMatrix4x4(const in GPUTransform m)
{
    return mat4x4(m.row0.x, m.row1.x, m.row2.x, 0.0,
                  m.row0.y, m.row1.y, m.row2.y, 0.0,
                  m.row0.z, m.row1.z, m.row2.z, 0.0,
                  m.row0.w, m.row1.w, m.row2.w, 1.0);
}

mat4x4 GPUTransformToInverseMatrix4x4(const in GPUTransform m)
{
    return mat4x4(m.invRow0.x, m.invRow1.x, m.invRow2.x, 0.0,
                  m.invRow0.y, m.invRow1.y, m.invRow2.y, 0.0,
                  m.invRow0.z, m.invRow1.z, m.invRow2.z, 0.0,
                  m.invRow0.w, m.invRow1.w, m.invRow2.w, 1.0);
}

mat4x4 GPUTransformToInverseTransposeMatrix4x4(const in GPUTransform m)
{
    return mat4x4(m.invRow0.x, m.invRow0.y, m.invRow0.z, m.invRow0.w,
                  m.invRow1.x, m.invRow1.y, m.invRow1.z, m.invRow1.w,
                  m.invRow2.x, m.invRow2.y, m.invRow2.z, m.invRow2.w,
                  0.0, 0.0, 0.0, 1.0);
}


mat4x4 GPUTransformToPreviousMatrix4x4(const in GPUTransform m)
{
    return mat4x4(m.prevRow0.x, m.prevRow1.x, m.prevRow2.x, 0.0,
                  m.prevRow0.y, m.prevRow1.y, m.prevRow2.y, 0.0,
                  m.prevRow0.z, m.prevRow1.z, m.prevRow2.z, 0.0,
                  m.prevRow0.w, m.prevRow1.w, m.prevRow2.w, 1.0);
}

#endif//INCLUDE_GUARD
