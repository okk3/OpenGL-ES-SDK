#ifndef VECMATH_H
#define VECMATH_H

#include <cfloat>
#include <cmath>

struct vec4
{
    float x, y, z, w;

    inline static vec4 cross(const vec4& u, const vec4& v)
    {
        return vec4(u.y*v.z-v.y*u.z, u.z*v.x-v.z*u.x, u.x*v.y-v.x*u.y, 0.0f);
    }

    inline static float length(const vec4& u)
    {
        return sqrt(u.x*u.x + u.y*u.y + u.z*u.z + u.w*u.w);
    }

    inline static vec4 normalize(const vec4& u)
    {
        float invmag = 1.0f/(length(u) + FLT_EPSILON);
        return vec4(u.x * invmag, u.y * invmag, u.z * invmag, u.w * invmag);
    }

    inline vec4()
    {
    }

    inline vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
    {
    }

    friend vec4 operator +(const vec4& u, const vec4& v);
    friend vec4 operator -(const vec4& u, const vec4& v);
    friend vec4 operator *(const vec4& u, const vec4& v);
    friend vec4 operator /(const vec4& u, const vec4& v);
};

inline vec4 operator +(const vec4& u, const vec4& v)
{
    return vec4(u.x+v.x, u.y+v.y, u.z+v.z, u.w+v.w);
}

inline vec4 operator -(const vec4& u, const vec4& v)
{
    return vec4(u.x-v.x, u.y-v.y, u.z-v.z, u.w-v.w);
}

inline vec4 operator *(const vec4& u, const vec4& v)
{
    return vec4(u.x*v.x, u.y*v.y, u.z*v.z, u.w*v.w);
}

inline vec4 operator /(const vec4& u, const vec4& v)
{
    return vec4(u.x/v.x, u.y/v.y, u.z/v.z, u.w/v.w);
}

struct mat4
{
    vec4 x;
    vec4 y;
    vec4 z;
    vec4 w;

    inline static mat4 identity()
    {
        return mat4(1.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f, 0.0f,0.0f,0.0f,1.0f);
    }

    inline static mat4 lookAt(const vec4& eye, const vec4& target, const vec4& up)
    {
        vec4 f = vec4::normalize(target - eye);
        vec4 s = vec4::normalize(vec4::cross(f, up));
        vec4 u = vec4::normalize(vec4::cross(s, f));

        mat4 m(
            s.x,
            u.x,
            -f.x,
            0.0f,

            s.y,
            u.y,
            -f.y,
            0.0f,

            s.z,
            u.z,
            -f.z,
            0.0f,

            0.0f,
            0.0f,
            0.0f,
            1.0f);

        vec4 t = m * vec4(-eye.x, -eye.y, -eye.z, 1.0f);
        m.w.x = t.x;
        m.w.y = t.y;
        m.w.z = t.z;

        return m;
    }

    inline static mat4 perspective(float fovY, float aspect, float zNear, float zFar)
    {
        float rad = fovY/180.0f * 3.141593f * 0.5f;
        float cotan = cos(rad) / sin(rad);
        float deltaZ = zNear - zFar;

        return mat4(
            cotan/aspect,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            cotan,
            0.0f,
            0.0f,

            0.0f,
            0.0f,
            (zFar + zNear) / deltaZ,
            -1.0f,

            0.0f,
            0.0f,
            2.0f * zNear * zFar / deltaZ,
            0.0f);
    }

    inline static mat4 rotate(float deg, const vec4& axis)
    {
        float rad = deg/180.0f * 3.141593f;
        float c = cos(rad);
        float s = sin(rad);
        return mat4(
            axis.x*axis.x*(1.0f-c) + c,
            axis.y*axis.x*(1.0f-c) + axis.z*s,
            axis.x*axis.z*(1.0f-c) - axis.y*s,
            0.0f,
            axis.x*axis.y*(1.0f-c) - axis.z*s,
            axis.y*axis.y*(1.0f-c) + c,
            axis.y*axis.z*(1.0f-c) + axis.x*s,
            0.0f,
            axis.x*axis.z*(1.0f-c) + axis.y*s,
            axis.y*axis.z*(1.0f-c) - axis.x*s,
            axis.z*axis.z*(1.0f-c) + c,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f);
    }

    inline mat4(float cxrx,
                float cxry,
                float cxrz,
                float cxrw,
                float cyrx,
                float cyry,
                float cyrz,
                float cyrw,
                float czrx,
                float czry,
                float czrz,
                float czrw,
                float cwrx,
                float cwry,
                float cwrz,
                float cwrw) :
        x(cxrx,cxry,cxrz,cxrw),
        y(cyrx,cyry,cyrz,cyrw),
        z(czrx,czry,czrz,czrw),
        w(cwrx,cwry,cwrz,cwrw)
    {
    }

    inline mat4()
    {
    }

    friend vec4 operator *(const mat4& m, const vec4& u);
    friend mat4 operator *(const mat4& m, const mat4& n);
};

inline vec4 operator *(const mat4& m, const vec4& u)
{
    return vec4(
        m.x.x*u.x + m.y.x*u.y + m.z.x*u.z + m.w.x*u.w,
        m.x.y*u.x + m.y.y*u.y + m.z.y*u.z + m.w.y*u.w,
        m.x.z*u.x + m.y.z*u.y + m.z.z*u.z + m.w.z*u.w,
        m.x.w*u.x + m.y.w*u.y + m.z.w*u.z + m.w.w*u.w);
}

inline mat4 operator *(const mat4& m, const mat4& n)
{
    return mat4(
        m.x.x*n.x.x + m.y.x*n.x.y + m.z.x*n.x.z + m.w.x*n.x.w,
        m.x.y*n.x.x + m.y.y*n.x.y + m.z.y*n.x.z + m.w.y*n.x.w,
        m.x.z*n.x.x + m.y.z*n.x.y + m.z.z*n.x.z + m.w.z*n.x.w,
        m.x.w*n.x.x + m.y.w*n.x.y + m.z.w*n.x.z + m.w.w*n.x.w,
        m.x.x*n.y.x + m.y.x*n.y.y + m.z.x*n.y.z + m.w.x*n.y.w,
        m.x.y*n.y.x + m.y.y*n.y.y + m.z.y*n.y.z + m.w.y*n.y.w,
        m.x.z*n.y.x + m.y.z*n.y.y + m.z.z*n.y.z + m.w.z*n.y.w,
        m.x.w*n.y.x + m.y.w*n.y.y + m.z.w*n.y.z + m.w.w*n.y.w,
        m.x.x*n.z.x + m.y.x*n.z.y + m.z.x*n.z.z + m.w.x*n.z.w,
        m.x.y*n.z.x + m.y.y*n.z.y + m.z.y*n.z.z + m.w.y*n.z.w,
        m.x.z*n.z.x + m.y.z*n.z.y + m.z.z*n.z.z + m.w.z*n.z.w,
        m.x.w*n.z.x + m.y.w*n.z.y + m.z.w*n.z.z + m.w.w*n.z.w,
        m.x.x*n.w.x + m.y.x*n.w.y + m.z.x*n.w.z + m.w.x*n.w.w,
        m.x.y*n.w.x + m.y.y*n.w.y + m.z.y*n.w.z + m.w.y*n.w.w,
        m.x.z*n.w.x + m.y.z*n.w.y + m.z.z*n.w.z + m.w.z*n.w.w,
        m.x.w*n.w.x + m.y.w*n.w.y + m.z.w*n.w.z + m.w.w*n.w.w);
}

#endif // VECMATH_H
