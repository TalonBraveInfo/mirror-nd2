//-----------------------------------------------------------------------------
//  nmaxanimator.h
//
//  (C)2004 Kim, Hyoiun Woo
//-----------------------------------------------------------------------------
#ifndef N_MAXANIMATOR_H
//-----------------------------------------------------------------------------
/**
    @class nMaxAnimator
    @ingroup

    @brief
*/

//-----------------------------------------------------------------------------
class nMaxAnimator
{
public:
    enum Type {
        PRS,
        IK,
        Path,
        Point3,
        Float,
    };

    nMaxAnimator();
    virtual ~nMaxAnimator();

    void Export(INode* inode);

protected:
    nMaxNode* CreateAnimator(INode* inode);

};
//-----------------------------------------------------------------------------
#endif 