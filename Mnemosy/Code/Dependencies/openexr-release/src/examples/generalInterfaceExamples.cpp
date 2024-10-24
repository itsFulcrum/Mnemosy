//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) Contributors to the OpenEXR Project.
//

//-----------------------------------------------------------------------------
//
//	Code examples that show how class InputFile and class OutputFile
//	can be used to read and write OpenEXR image files with an arbitrary
//	set of channels.
//
//-----------------------------------------------------------------------------


#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfInputFile.h>
#include <ImfMatrixAttribute.h>
#include <ImfOutputFile.h>
#include <ImfStringAttribute.h>

#include "drawImage.h"

#include <cfloat>
#include <iostream>
#include <limits>

#include "namespaceAlias.h"
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;

void
writeGZ1 (
    const char   fileName[],
    const half*  gPixels,
    const float* zPixels,
    int          width,
    int          height)
{
    //
    // Write an image with only a G (green) and a Z (depth) channel,
    // using class OutputFile.
    //
    //	- create a file header
    //	- add G and Z channels to the header
    //	- open the file, and store the header in the file
    //	- describe the memory layout of the G and Z pixels
    //	- store the pixels in the file
    //

    Header header (width, height);
    header.channels ().insert ("G", Channel (IMF::HALF));
    header.channels ().insert ("Z", Channel (IMF::FLOAT));

    OutputFile file (fileName, header);

    FrameBuffer frameBuffer;

    frameBuffer.insert (
        "G", // name
        Slice (
            IMF::HALF,                   // type
            (char*) gPixels,             // base
            sizeof (*gPixels) * 1,       // xStride
            sizeof (*gPixels) * width)); // yStride

    frameBuffer.insert (
        "Z", // name
        Slice (
            IMF::FLOAT,                  // type
            (char*) zPixels,             // base
            sizeof (*zPixels) * 1,       // xStride
            sizeof (*zPixels) * width)); // yStride

    file.setFrameBuffer (frameBuffer);
    file.writePixels (height);
}

void
writeGZ2 (
    const char   fileName[],
    const half*  gPixels,
    const float* zPixels,
    int          width,
    int          height,
    const Box2i& dataWindow)
{
    //
    // Write an image with only a G (green) and a Z (depth) channel,
    // using class OutputFile.  Don't store the whole image in the
    // file, but crop it according to the given data window.
    //
    //	- create a file header
    //	- set the header's data window
    //	- add G and Z channels to the header
    //	- open the file, and store the header in the file
    //	- describe the memory layout of the G anx Z pixels
    //	- store the pixels in the file
    //

    Header header (width, height);
    header.dataWindow () = dataWindow;
    header.channels ().insert ("G", Channel (IMF::HALF));
    header.channels ().insert ("Z", Channel (IMF::FLOAT));

    OutputFile file (fileName, header);

    FrameBuffer frameBuffer;

    frameBuffer.insert (
        "G", // name
        Slice (
            IMF::HALF,                   // type
            (char*) gPixels,             // base
            sizeof (*gPixels) * 1,       // xStride
            sizeof (*gPixels) * width)); // yStride

    frameBuffer.insert (
        "Z", // name
        Slice (
            IMF::FLOAT,                  // type
            (char*) zPixels,             // base
            sizeof (*zPixels) * 1,       // xStride
            sizeof (*zPixels) * width)); // yStride

    file.setFrameBuffer (frameBuffer);
    file.writePixels (dataWindow.max.y - dataWindow.min.y + 1);
}

void
readGZ1 (
    const char      fileName[],
    Array2D<half>&  rPixels,
    Array2D<half>&  gPixels,
    Array2D<float>& zPixels,
    int&            width,
    int&            height)
{
    //
    // Read an image using class InputFile.  Try to read two
    // channels, R and G, of type HALF, and one channel, Z,
    // of type FLOAT.  Store the R, G, and Z pixels in three
    // separate memory buffers.
    // If a channel is missing in the file, the buffer for that
    // channel will be filled with an appropriate default value.
    //
    //	- open the file
    //	- allocate memory for the pixels
    //	- describe the layout of the R, G, and Z pixel buffers
    //	- read the pixels from the file
    //

    InputFile file (fileName);

    Box2i dw = file.header ().dataWindow ();
    width    = dw.max.x - dw.min.x + 1;
    height   = dw.max.y - dw.min.y + 1;

    rPixels.resizeErase (height, width);
    gPixels.resizeErase (height, width);
    zPixels.resizeErase (height, width);

    FrameBuffer frameBuffer;

    frameBuffer.insert (
        "R", // name
        Slice (
            IMF::HALF,                // type
            (char*) (&rPixels[0][0] - // base
                     dw.min.x - dw.min.y * width),
            sizeof (rPixels[0][0]) * 1,     // xStride
            sizeof (rPixels[0][0]) * width, // yStride
            1,
            1,     // x/y sampling
            0.0)); // fillValue

    frameBuffer.insert (
        "G", // name
        Slice (
            IMF::HALF,                // type
            (char*) (&gPixels[0][0] - // base
                     dw.min.x - dw.min.y * width),
            sizeof (gPixels[0][0]) * 1,     // xStride
            sizeof (gPixels[0][0]) * width, // yStride
            1,
            1,     // x/y sampling
            0.0)); // fillValue

    frameBuffer.insert (
        "Z", // name
        Slice (
            IMF::FLOAT,               // type
            (char*) (&zPixels[0][0] - // base
                     dw.min.x - dw.min.y * width),
            sizeof (zPixels[0][0]) * 1,     // xStride
            sizeof (zPixels[0][0]) * width, // yStride
            1,
            1,                                    // x/y sampling
            std::numeric_limits<float>::max ())); // fillValue

    file.setFrameBuffer (frameBuffer);
    file.readPixels (dw.min.y, dw.max.y);
}

void
readGZ2 (const char fileName[], Array2D<GZ>& pixels, int& width, int& height)
{
    //
    // Read an image using class InputFile.  Try to read one channel,
    // G, of type HALF, and one channel, Z, of type FLOAT.  In memory,
    // the G and Z channels will be interleaved in a single buffer.
    //
    //	- open the file
    //	- allocate memory for the pixels
    //	- describe the layout of the GZ pixel buffer
    //	- read the pixels from the file
    //

    InputFile file (fileName);

    Box2i dw = file.header ().dataWindow ();
    width    = dw.max.x - dw.min.x + 1;
    height   = dw.max.y - dw.min.y + 1;
    int dx   = dw.min.x;
    int dy   = dw.min.y;

    pixels.resizeErase (height, width);

    FrameBuffer frameBuffer;

    frameBuffer.insert (
        "G", // name
        Slice (
            IMF::HALF,                       // type
            (char*) &pixels[-dy][-dx].g,     // base
            sizeof (pixels[0][0]) * 1,       // xStride
            sizeof (pixels[0][0]) * width)); // yStride

    frameBuffer.insert (
        "Z", // name
        Slice (
            IMF::FLOAT,                      // type
            (char*) &pixels[-dy][-dx].z,     // base
            sizeof (pixels[0][0]) * 1,       // xStride
            sizeof (pixels[0][0]) * width)); // yStride

    file.setFrameBuffer (frameBuffer);
    file.readPixels (dw.min.y, dw.max.y);
}

void
generalInterfaceExamples ()
{
    cout << "\nGZ (green, depth) images\n" << endl;
    cout << "drawing image" << endl;

    int w = 800;
    int h = 600;

    Array2D<half>  gp (h, w);
    Array2D<float> zp (h, w);
    drawImage2 (gp, zp, w, h);

    cout << "writing entire image" << endl;

    writeGZ1 ("gz1.exr", &gp[0][0], &zp[0][0], w, h);

    cout << "writing cropped image" << endl;

    writeGZ2 (
        "gz2.exr",
        &gp[0][0],
        &zp[0][0],
        w,
        h,
        Box2i (V2i (w / 6, h / 6), V2i (w / 2, h / 2)));

    cout << "reading file into separate per-channel buffers" << endl;

    Array2D<half> rp (1, 1);
    readGZ1 ("gz2.exr", rp, gp, zp, w, h);

    cout << "reading file into interleaved multi-channel buffer" << endl;

    Array2D<GZ> gzp (1, 1);
    readGZ2 ("gz2.exr", gzp, w, h);
}
