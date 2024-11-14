// Adds compile-time JS functions to augment the SkiaWebBinding interface.
// Specifically, anything that should only be on the WebGL version of canvaskit.
// Functions in this file are supplemented by cpu.js.
(function(SkiaWebBinding) {
    SkiaWebBinding._extraInitializations = SkiaWebBinding._extraInitializations || [];
    SkiaWebBinding._extraInitializations.push(function() {
        SkiaWebBinding.MakeGPUDeviceContext = function(device) {
            if (!device) {
                return null;
            }

            // This allows native code to access this device by calling
            // `emscripten_webgpu_get_device().`
            SkiaWebBinding.preinitializedWebGPUDevice = device;
            var context = this._MakeWebGPUDirectContext();
            context._device = device;

            return context;
        };

        SkiaWebBinding.MakeGPUCanvasContext = function(devCtx, canvas, opts) {
            var canvasCtx = canvas.getContext('webgpu');
            if (!canvasCtx) {
                return null;
            }

            let format = (opts && opts.format) ? opts.format : navigator.gpu.getPreferredCanvasFormat();
            // GPUCanvasConfiguration
            canvasCtx.configure({
                device: devCtx._device,
                format: format,
                alphaMode: (opts && opts.alphaMode) ? opts.alphaMode : undefined,
            });

            var context = {
                '_inner': canvasCtx,
                '_deviceContext': devCtx,
                '_textureFormat': format,
            };
            context['requestAnimationFrame'] = function(callback) {
                requestAnimationFrame(function() {
                    const surface = SkiaWebBinding.MakeGPUCanvasSurface(context);
                    if (!surface) {
                        console.error('Failed to initialize Surface for current canvas swapchain texture');
                        return;
                    }
                    callback(surface.getCanvas());
                    surface.flush();
                    surface.dispose();
                });
            };
            return context;
        };

        SkiaWebBinding.MakeGPUCanvasSurface = function(canvasCtx, colorSpace, width, height) {
            let context = canvasCtx._inner;
            if (!width) {
                width = context.canvas.width;
            }
            if (!height) {
                height = context.canvas.height;
            }
            let surface = this.MakeGPUTextureSurface(canvasCtx._deviceContext,
                context.getCurrentTexture(),
                canvasCtx._textureFormat,
                width, height, colorSpace);
            surface._canvasContext = canvasCtx;
            return surface;
        };

        SkiaWebBinding.MakeGPUTextureSurface = function(devCtx, texture, textureFormat, width, height, colorSpace) {
            colorSpace = colorSpace || null;

            // JsValStore and WebGPU are objects in Emscripten's library_html5_webgpu.js utility
            // library. JsValStore allows a WebGPU object to be imported by native code by calling the
            // various `emscripten_webgpu_import_*` functions.
            //
            // The SkiaWebBinding WASM module is responsible for removing entries from the value store by
            // calling `emscripten_webgpu_release_js_handle` after importing the object.
            //
            // (see
            // https://github.com/emscripten-core/emscripten/blob/0e63f74f36b06849ef1c777b130783a43316ade0/src/library_html5_webgpu.js
            // for reference)
            return this._MakeGPUTextureSurface(
                devCtx,
                this.JsValStore.add(texture),
                this.WebGPU.TextureFormat.indexOf(textureFormat),
                width, height,
                colorSpace);
        };
    });
}(Module)); // When this file is loaded in, the high level object is "Module".