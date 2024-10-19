# options from Mnemosy for libtiff

set(tiff_tools            	OFF CACHE BOOL "" FORCE)
set(tiff_contrib           	OFF CACHE BOOL "" FORCE)
set(tiff_tests            	OFF CACHE BOOL "" FORCE)
set(tiff_docs             	OFF CACHE BOOL "" FORCE)
set(tiff_deprecated       	OFF CACHE BOOL "" FORCE)
set(tiff_opengl       		OFF CACHE BOOL "" FORCE)
set(tiff_install       		OFF CACHE BOOL "" FORCE)


# === Features

set(tiff_extra-warnings			OFF CACHE BOOL "" FORCE)
set(tiff_fatal-warnings			OFF CACHE BOOL "" FORCE)


set(tiff_strip-chopping			ON CACHE BOOL "" FORCE)
set(tiff_defer-strile-load		OFF CACHE BOOL "" FORCE)
set(tiff_chunky-strip-read		OFF CACHE BOOL "" FORCE)
set(tiff_extrasample-as-alpha	ON CACHE BOOL "" FORCE)
set(tiff_check-ycbcr-subsampling	ON CACHE BOOL "" FORCE)


set(tiff_ld-version-script			ON CACHE BOOL "" FORCE)
# === Codecs

# == Internal Codecs - tale all we can get

set(tiff_ccitt      ON CACHE BOOL "" FORCE)
set(tiff_packbits   ON CACHE BOOL "" FORCE)
set(tiff_lzw       	ON CACHE BOOL "" FORCE)
set(tiff_thunder    ON CACHE BOOL "" FORCE)
set(tiff_next       ON CACHE BOOL "" FORCE)
set(tiff_logluv     ON CACHE BOOL "" FORCE)
set(tiff_mdi       	ON CACHE BOOL "" FORCE)


# == External Codecs
# zlib is important but not neccesarry
set(tiff_zlib      		ON CACHE BOOL "" FORCE)
set(tiff_pixarlog     		 ON CACHE BOOL "" FORCE)

set(tiff_jbig       OFF CACHE BOOL "" FORCE)
set(tiff_jpeg       OFF CACHE BOOL "" FORCE)
set(tiff_jpeg12     OFF CACHE BOOL "" FORCE)
set(tiff_old-jpeg	OFF CACHE BOOL "" FORCE)



set(tiff_lzma       		OFF CACHE BOOL "" FORCE)
set(tiff_lerc       		OFF CACHE BOOL "" FORCE)
set(tiff_zstd       		OFF CACHE BOOL "" FORCE)
set(tiff_webp       		OFF CACHE BOOL "" FORCE)





