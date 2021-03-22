#pragma once

#include "maths.h"

class Camera
{
public:
    vec_t mPosition{ 0.f, 0.f, 0.f };
    vec_t mAngles{ 0.f, 0.f, 0.f };


    matrix_t ComputeView() const
    {
        matrix_t view;
        view.rotationYawPitchRoll(mAngles.x, mAngles.y, mAngles.z);
        view.position = vec_t(mPosition.x, mPosition.y, mPosition.z, 1.f);
        return view;
    }

};