# gyeongho-h265-decoder

A H.265 decoder implemented in WebAssembly using FFMPEG. This project converts H.265 bitstreams into YUV420 Uint8Array for playing H.265 bitstream in browser.

# Features

- Decode H.265 bitstream into YUV420 Uint8Array
- Convert YUV420 Uint8Array to VideoFrame
- Render VideoFrame to Canvas(2d, WebGL, WebGPU)

# Getting Started

## Installation

This command copy WASM binary and glue Javascript file to current directory.

```bash
npx gyeongho-h265-decoder
```

There is path option to specify path of WASM binary and glue Javascript file.  
I recommend you to copy WASM and JS files to your **public** directory.

```bash
npx gyeongho-h265-decoder ./public
```

then your directory structure should be like this.

```bash
public/
├── gyeongho-h265-decoder.js
└── gyeongho-h265-decoder.wasm
```

Basically, Glue Javascript file **fetch from network(same path that js file is located)** WASM binary file.  
If you want to use custom path to WASM binary file, you should write this code **before** importing glue Javascript file.  

```javascript
let Module = {
  locateFile: (path, prefix) => {
    // prefix means path of glue Javascript file
    // if your glue Javascript file is located in public directory and base path is "/"
    // prefix is ""
  }
}
```

## Usage(Browser)

You can use decoder by

- option 1: script tag in your HTML

```html
<script src="/gyeongho-h265-decoder.js"></script>
```

- option 2: import in your Javascript

```javascript
// Web Worker(recommended)
let Module = {
  onRuntimeInitialized: function() {
    // do something when initialized
  },
  locateFile: (path, prefix) => {
    // do something
  }
}
importScripts("/gyeongho-h265-decoder.js");

// In main event loop
function loadEmscriptenModule(path) {
  return new Promise((resolve, reject) => {

    window.Module = {
      onRuntimeInitialized: function() {
        resolve(window.Module);
      },

      locateFile: function(path, prefix) {
        return '/path/to/wasm/' + path;
      }
    };

    const script = document.createElement('script');
    script.src = path;
    script.onerror = reject;
    document.body.appendChild(script);
  });
}
```

and then, you must define callback function that is called when frame is decoded.

```javascript
const callback = (offset, size, width, height) => {
  const yuv420ArrayBuffer = Module.HEAPU8.subarray(offset, offset + size);
  // do something
  // make VideoFrame using WebCodecs API, or render WebGL Canvas, or something you want
}
// it means add function that returns void and takes int32_t, int32_t, int32_t, int32_t arguments
const pointer = Module.addFunction(callback, "viiii");
Module._init_decoder(pointer);
```

finally,

1. get single nalu Uing8Array or flatten nalus(in AnnexB format) Uint8Array from HTTP, WebSocket, File or Whatever
2. call `_malloc(nalus.length)` to allocate memory for nalus, you can get pointer of allocated memory
3. set nalu buffer to allocated memory by `HEAPU8.set(nalu, pointer)`
4. call `_decode_AnnexB_buffer(pointer, nalus.length)` to decode nalus
5. get Uint8Array you allocated previously by `HEAPU8.subarray(offset, offset + size)`, you can get offset and size from paramter of callback function you defined.

# Interface

```typescript
declare interface Module {
  // Memory related
  _malloc(size: number): number;
  _free(pointer: number): number;
  HEAPU8: Uint8Array;

  // Decoder related
  _init_decoder(callback: number): number;
  _decode_AnnexB_buffer(buffer: number, size: number): number;
  _flush_decoder(): number;
  _close_decoder(): number;

  // Function management
  addFunction(
    func: (...args: any[]) => void,
    signature: string
  ): number;
}
```

# Warning

Make sure you `_malloc()` within 1GB, and call `_free()` after you don't need it.  
You should not expect javascript's behavior(Garbage Collection, etc).

# License

This project builds FFMPEG under GPLv2 license.  
See [LICENSE](LICENSE) for more details.