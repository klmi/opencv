/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Copyright (C) 2010-2012, Multicoreware, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "test_precomp.hpp"
#include "opencv2/ts/ocl_test.hpp"

#ifdef HAVE_OPENCL

namespace cvtest {
namespace ocl {

/////////////////////////////////////////////////////////////////////////////////////////////////
// Filter2D
PARAM_TEST_CASE(Filter2D, MatDepth, Channels, int, int, BorderType, bool, bool)
{
    static const int kernelMinSize = 2;
    static const int kernelMaxSize = 10;

    int type;
    Size dsize;
    Point anchor;
    int borderType;
    int widthMultiple;
    bool useRoi;
    Mat kernel;
    double delta;

    TEST_DECLARE_INPUT_PARAMETER(src);
    TEST_DECLARE_OUTPUT_PARAMETER(dst);

    virtual void SetUp()
    {
        type = CV_MAKE_TYPE(GET_PARAM(0), GET_PARAM(1));
        Size ksize(GET_PARAM(2), GET_PARAM(2));
        widthMultiple = GET_PARAM(3);
        borderType = GET_PARAM(4) | (GET_PARAM(5) ? BORDER_ISOLATED : 0);
        useRoi = GET_PARAM(6);
        Mat temp = randomMat(ksize, CV_MAKE_TYPE(((CV_64F == CV_MAT_DEPTH(type)) ? CV_64F : CV_32F), 1), -MAX_VALUE, MAX_VALUE);
        cv::normalize(temp, kernel, 1.0, 0.0, NORM_L1);
    }

    void random_roi()
    {
        dsize = randomSize(1, MAX_VALUE);
        // Make sure the width is a multiple of the requested value, and no more.
        dsize.width &= ~((widthMultiple * 2) - 1);
        dsize.width += widthMultiple;

        Size roiSize = randomSize(kernel.size[0], MAX_VALUE, kernel.size[1], MAX_VALUE);
        Border srcBorder = randomBorder(0, useRoi ? MAX_VALUE : 0);
        randomSubMat(src, src_roi, roiSize, srcBorder, type, -MAX_VALUE, MAX_VALUE);

        Border dstBorder = randomBorder(0, useRoi ? MAX_VALUE : 0);
        randomSubMat(dst, dst_roi, dsize, dstBorder, type, -MAX_VALUE, MAX_VALUE);

        anchor.x = randomInt(-1, kernel.size[0]);
        anchor.y = randomInt(-1, kernel.size[1]);

        delta = randomDouble(-100, 100);

        UMAT_UPLOAD_INPUT_PARAMETER(src);
        UMAT_UPLOAD_OUTPUT_PARAMETER(dst);
    }

    void Near(double threshold = 0.0)
    {
        EXPECT_MAT_NEAR(dst, udst, threshold);
        EXPECT_MAT_NEAR(dst_roi, udst_roi, threshold);
    }
};

OCL_TEST_P(Filter2D, Mat)
{
    for (int j = 0; j < test_loop_times; j++)
    {
        random_roi();

        OCL_OFF(cv::filter2D(src_roi, dst_roi, -1, kernel, anchor, delta, borderType));
        OCL_ON(cv::filter2D(usrc_roi, udst_roi, -1, kernel, anchor, delta, borderType));

        Near(1.0);
    }
}

OCL_INSTANTIATE_TEST_CASE_P(ImageProc, Filter2D,
                            Combine(
                                Values(CV_8U, CV_16U, CV_32F),
                                OCL_ALL_CHANNELS,
                                Values(3, 5, 9),  // Kernel size
                                Values(1, 4, 8),   // Width mutiple
                                Values((BorderType)BORDER_CONSTANT,
                                       (BorderType)BORDER_REPLICATE,
                                       (BorderType)BORDER_REFLECT,
                                       (BorderType)BORDER_REFLECT_101),
                                Bool(), // BORDER_ISOLATED
                                Bool()  // ROI
                                )
                           );


} } // namespace cvtest::ocl

#endif // HAVE_OPENCL
