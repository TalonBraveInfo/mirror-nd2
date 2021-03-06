#ifndef N_TRANSFORM33_H
#define N_TRANSFORM33_H
//------------------------------------------------------------------------------
/**
    @class transform33
    @ingroup NebulaMathDataTypes

    A 3x3 matrix which is described by 2D translation, rotation and scale.

    -01-Nov-06  kims Changed seteulerrotation() to have vector3 animation instead of 
                     vector2 and return vector3 type for geteulerrotation().  
             The change were made for uv animation.

    (C) 2004 RadonLabs GmbH
*/
#include "mathlib/matrix.h"
#include "mathlib/vector.h"

//------------------------------------------------------------------------------
class transform33
{
public:
    /// constructor
    transform33();
    /// set translation
    void settranslation(const vector2& v);
    /// get translation
    const vector2& gettranslation() const;
    /// set euler rotation
    void seteulerrotation(const vector3& v);
    /// get euler rotation
    const vector3& geteulerrotation() const;
    /// return true if the transformation matrix is dirty
    bool isdirty() const;
    /// HACK: set the dirty bit
    void setdirty(bool dirty);
    /// set scale
    void setscale(const vector2& v);
    /// get scale
    const vector2& getscale() const;
    /// get resulting 3x3 matrix
    const matrix33& getmatrix33();
    /// get resulting 4x4 matrix, with 3x3 matrix cast to the upper-left corner
    void getmatrix44(matrix44& out);
    /// set resulting 3x3 matrix
    void setmatrix33(const matrix33& mx);

private:
    /// update internal matrix
    void update();

    enum
    {
        Dirty = (1<<0),
    };
    vector2 translation;
    vector3 euler;
    vector2 scale;
    matrix33 matrix;
    uchar flags;
};

//------------------------------------------------------------------------------
/**
*/
inline
transform33::transform33() :
    scale(1.0f, 1.0f),
    flags(0)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
void
transform33::settranslation(const vector2& v)
{
    this->translation = v;
    this->flags |= Dirty;
}

//------------------------------------------------------------------------------
/**
*/
inline
const vector2&
transform33::gettranslation() const
{
    return this->translation;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
transform33::seteulerrotation(const vector3& v)
{
    this->euler = v;
    this->flags |= Dirty;
}

//------------------------------------------------------------------------------
/**
*/
inline
const vector3&
transform33::geteulerrotation() const
{
    return this->euler;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
transform33::setscale(const vector2& v)
{
    this->scale = v;
    this->flags |= Dirty;
}

//------------------------------------------------------------------------------
/**
*/
inline
const vector2&
transform33::getscale() const
{
    return this->scale;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
transform33::update()
{
    if (this->flags & Dirty)
    {
        this->matrix.ident();
        this->matrix.scale(vector3(this->scale.x, this->scale.y, 1.0f));
        this->matrix.rotate_x(this->euler.x);
        this->matrix.rotate_y(this->euler.y);
        this->matrix.rotate_z(this->euler.z);
        this->matrix.translate(this->translation);
        this->flags &= ~Dirty;
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
const matrix33&
transform33::getmatrix33()
{
    this->update();
    return this->matrix;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
transform33::getmatrix44(matrix44& out)
{
    this->update();
    out.set(this->matrix.M11, this->matrix.M12, this->matrix.M13, 0.0f,
            this->matrix.M21, this->matrix.M22, this->matrix.M23, 0.0f,
            this->matrix.M31, this->matrix.M32, this->matrix.M33, 0.0f,
            0.0f,             0.0f,             0.0f,             1.0f);
}

//------------------------------------------------------------------------------
/**
    -01-Nov-06  kims Added for uv animation.
*/
inline
void
transform33::setmatrix33(const matrix33& mx)
{
    this->matrix = mx;
    this->flags |= Dirty;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
transform33::isdirty() const
{
    return (0 != (this->flags & Dirty));
}

//------------------------------------------------------------------------------
/**
    -01-Nov-06  kims Added for uv animation.
*/
inline
void
transform33::setdirty(bool dirty)
{
    if (dirty)
    {
        this->flags |= Dirty;
    }
    else
    {
        this->flags &= ~Dirty;
    }
}
//------------------------------------------------------------------------------
#endif

