#!/bin/bash

export TOTAL_MEMORY=1073741824
export EXPORTED_FUNCTIONS="[ \
	'_init_decoder', \
	'_flush_decoder', \
	'_close_decoder', \
    '_decode_AnnexB_buffer', \
    '_main', \
    '_malloc', \
    '_free'
]"

echo "Running Emscripten..."
emcc src/decode_h265.c ffmpeg_build/lib/libavcodec.a ffmpeg_build/lib/libavutil.a \
    -O3 \
    -I "ffmpeg_build/include" \
    -s WASM=1 \
    -s TOTAL_MEMORY=${TOTAL_MEMORY} \
   	-s EXPORTED_FUNCTIONS="${EXPORTED_FUNCTIONS}" \
   	-s EXTRA_EXPORTED_RUNTIME_METHODS="['addFunction']" \
	-s RESERVED_FUNCTION_POINTERS=14 \
	-s FORCE_FILESYSTEM=1 \
    -o dist/gyeongho-h265-decoder.js \

echo "Finished Build"
