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
    echo "emcc not found, check emsdk already exists"
    if [ -d "emsdk" ]; then
        echo "emsdk already exists, set environment variables"
        cd emsdk
        git pull
        ./emsdk install 4.0.5
        ./emsdk activate 4.0.5
        source ./emsdk_env.sh
        cd ..
    else
        echo "emsdk not found, clone and install emscripten"
        git clone https://github.com/emscripten-core/emsdk.git
        cd emsdk
        ./emsdk install 4.0.5
        ./emsdk activate 4.0.5
        source ./emsdk_env.sh
        cd ..
    fi
fi

./build_hevc_decoder.sh
