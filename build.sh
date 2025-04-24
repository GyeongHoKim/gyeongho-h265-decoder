echo "Check ffmpeg repository already exists"
if [ -d "ffmpeg" ]; then
    echo "ffmpeg repository already exists, skip clone"
else
    echo "ffmpeg repository not found, clone and checkout to n4.1"
    git clone https://git.ffmpeg.org/ffmpeg.git
    cd ffmpeg
    git checkout n4.1
    cd ..
fi

echo "Check emcc already installed"
if ! command -v emcc --version > /dev/null 2>&1; then
    echo "emcc not found, install emscripten"
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install 4.0.5
    ./emsdk activate 4.0.5
    source ./emsdk_env.sh
    cd ..
fi

./build_hevc_decoder.sh
