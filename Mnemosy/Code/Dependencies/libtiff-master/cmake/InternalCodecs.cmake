# Options to enable and disable internal codecs
#
# Copyright © 2015 Open Microscopy Environment / University of Dundee
# Copyright © 2021 Roger Leigh <rleigh@codelibre.net>
# Written by Roger Leigh <rleigh@codelibre.net>
#
# Permission to use, copy, modify, distribute, and sell this software and
# its documentation for any purpose is hereby granted without fee, provided
# that (i) the above copyright notices and this permission notice appear in
# all copies of the software and related documentation, and (ii) the names of
# Sam Leffler and Silicon Graphics may not be used in any advertising or
# publicity relating to the software without the specific, prior written
# permission of Sam Leffler and Silicon Graphics.
#
# THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
# EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
# WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
# ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
# OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
# LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.


option(tiff_ccitt "support for CCITT Group 3 & 4 algorithms" ON)
set(CCITT_SUPPORT ${tiff_ccitt})

option(tiff_packbits "support for Macintosh PackBits algorithm" ON)
set(PACKBITS_SUPPORT ${tiff_packbits})

option(tiff_lzw "support for LZW algorithm" ON)
set(LZW_SUPPORT ${tiff_lzw})

option(tiff_thunder "support for ThunderScan 4-bit RLE algorithm" ON)
set(THUNDER_SUPPORT ${tiff_thunder})

option(tiff_next "support for NeXT 2-bit RLE algorithm" ON)
set(NEXT_SUPPORT ${tiff_next})

option(tiff_logluv "support for LogLuv high dynamic range algorithm" ON)
set(LOGLUV_SUPPORT ${tiff_logluv})

option(tiff_mdi "support for Microsoft Document Imaging" ON)
set(MDI_SUPPORT ${tiff_mdi})
