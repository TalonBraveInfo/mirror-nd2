//-----------------------------------------------------------------------------
// nmaxprscontroller_pos.cc
//
// (c)2004 Kim, Hyoun Woo.
//-----------------------------------------------------------------------------
#include "export2/nmax.h"
#include "export2/nmaxcontroller.h"
#include "export2/nmaxprscontroller.h"
#include "export2/nmaxinterface.h"
#include "export2/nmaxtransform.h"
#include "pluginlibs/nmaxdlg.h"
#include "pluginlibs/nmaxlogdlg.h"

#include "kernel/ntypes.h"
#include "util/narray.h"
#include "scene/ntransformanimator.h"

//-----------------------------------------------------------------------------
/**
*/
void nMaxTransformAnimator::ExportPosition(Control *control, nTransformAnimator* animator)
{
    IKeyControl* iKeyControl = GetKeyControlInterface(control);

    Control* xControl = control->GetXController();
    Control* yControl = control->GetYController();
    Control* zControl = control->GetZController();

    int numKeys = iKeyControl->GetNumKeys();

    if (iKeyControl)
    {
        if (numKeys > 0)
        {
            nMaxControl::Type type = nMaxControl::GetType(control);

            switch(type)
            {
            case nMaxControl::TCBPosition:
                ExportTCBPosition(iKeyControl, numKeys);
                break;

            case nMaxControl::HybridPosition:
                ExportHybridPosition(iKeyControl, numKeys);
                break;

            case nMaxControl::LinearPosition:
                ExportLinearPosition(iKeyControl, numKeys);
                break;

            default:
                ExportSampledKeyPosition(numKeys, animator);
                break;
            }
        }
        else
        {
            // we have Controller but it has no keys.
            n_maxlog(Warning, "The node '%s' has Control but no keys.", this->maxNode->GetName());
            return;
        }

    }
    else
    if (xControl || yControl || zControl)
    {
        if (HasSampledKeys(control))
        {
            ExportSampledKeyPosition(numKeys, animator);
        }
        else
        {
            // got one of the xControl, yControl or zControl but 
            // any control of those has no keys.
            n_maxlog(Warning, "The node '%s' has Control but no keys.", this->maxNode->GetName());
            return;
        }
    }
    else
    {
        // export sampled key position animation.
        ExportSampledKeyPosition(numKeys, animator);
    }
}

//-----------------------------------------------------------------------------
/**
*/
void nMaxTransformAnimator::ExportTCBPosition(IKeyControl* ikc, int numKeys)
{
    //for (i=0; i<numKeys; i++)
    //{
    //    ITCBPoint3Key key;
    //    ikc->GetKey(i, &key);
    //}

    n_maxlog(Warning, "The 'TCBPosition' type of control is not supported.");
}

//-----------------------------------------------------------------------------
/**
*/
void nMaxTransformAnimator::ExportHybridPosition(IKeyControl* ikc, int numKeys)
{
    //for (i=0; i<numKeys; i++)
    //{
    //    IBezPoint3Key key;
    //    ikc->GetKey(i, &key);
    //}

    n_maxlog(Warning, "The 'HybridPosition' type of control is not supported.");
}

//-----------------------------------------------------------------------------
/**
*/
void nMaxTransformAnimator::ExportLinearPosition(IKeyControl* ikc, int numKeys)
{
    //for (i=0; i<numKeys; i++)
    //{
    //    ILinPoint3Key key;
    //    ikc->GetKey(i, &key);
    //}

    n_maxlog(Warning, "The 'LinearPosition' type of control is not supported.");
}

//-----------------------------------------------------------------------------
/**
*/
void nMaxTransformAnimator::ExportSampledKeyPosition(int numKeys, nTransformAnimator* animator)
{
    nArray<nMaxSampleKey> sampleKeyArray;
    sampleKeyArray.SetFixedSize(numKeys + 1);

    nMaxControl::GetSampledKey(this->maxNode, sampleKeyArray, 1, nMaxPos);

    // assign sample keys to animator.
    for (int i=0; i<sampleKeyArray.Size(); i++)
    {
        nMaxSampleKey sampleKey = sampleKeyArray[i];

        TimeValue time = sampleKey.time;

        vector3 pos;
        pos.x = -(sampleKey.pos.x);
        pos.y = sampleKey.pos.z;
        pos.z = sampleKey.pos.y;

        animator->AddPosKey(time, pos);
    }
}