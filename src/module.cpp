/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "./skiaWebBinding.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>

#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"

using namespace emscripten;
using JSObject = emscripten::val;
using WASMPointerU8 = uintptr_t;

#ifdef CK_ENABLE_WEBGPU
#error "WebGPU not supported yet."
#endif  // CK_ENABLE_WEBGPU

#if defined(CK_ENABLE_WEBGL) || defined(CK_ENABLE_WEBGPU)
#define ENABLE_GPU
#endif

#ifdef ENABLE_GPU
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrExternalTextureGenerator.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#endif  // ENABLE_GPU

#ifdef CK_ENABLE_WEBGL
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLMakeWebGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"

#include <GLES2/gl2.h>
#endif  // CK_ENABLE_WEBGL

#if defined(GPU_TEST_UTILS)
#error "This define should not be set, as it brings in test-only things and bloats codesize."
#endif

#ifdef CK_ENABLE_WEBGL

sk_sp<GrDirectContext> MakeWebGLDirectContext() {
    // We assume that any calls we make to GL for the remainder of this function will go to the
    // desired WebGL Context.
    // setup interface.
    auto interface = GrGLInterfaces::MakeWebGL();
    // setup context
    return GrDirectContexts::MakeGL(interface);
}

sk_sp<SkSurface> MakeOnScreenGLSurface(
        sk_sp<GrDirectContext> dContext, int width, int height, int sampleCnt, int stencil) {
    // WebGL should already be clearing the color and stencil buffers, but do it again here to
    // ensure Skia receives them in the expected state.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    dContext->resetContext(kRenderTarget_GrGLBackendState | kMisc_GrGLBackendState);

    // The on-screen canvas is FBO 0. Wrap it in a Skia render target so Skia can render to it.
    GrGLFramebufferInfo info;
    info.fFBOID = 0;

    auto colorSpace = SkColorSpace::MakeSRGB();
    SkColorType colorType = kRGBA_8888_SkColorType;
    GrGLenum pixFormat = GR_GL_RGBA8;

    info.fFormat = pixFormat;
    auto target = GrBackendRenderTargets::MakeGL(width, height, sampleCnt, stencil, info);
    sk_sp<SkSurface> surface(SkSurfaces::WrapBackendRenderTarget(dContext.get(),
                                                                 target,
                                                                 kBottomLeft_GrSurfaceOrigin,
                                                                 kRGBA_8888_SkColorType,
                                                                 colorSpace,
                                                                 nullptr));
    return surface;
}

sk_sp<SkSurface> MakeOnScreenGLSurface(sk_sp<GrDirectContext> dContext, int width, int height) {
    GrGLint sampleCnt;
    glGetIntegerv(GL_SAMPLES, &sampleCnt);

    GrGLint stencil;
    glGetIntegerv(GL_STENCIL_BITS, &stencil);

    return MakeOnScreenGLSurface(dContext, width, height, sampleCnt, stencil);
}

#endif  // CK_ENABLE_WEBGL

//========================================================================================
// Path Effects
//========================================================================================

// These objects have private destructors / delete methods - I don't think
// we need to do anything other than tell emscripten to do nothing.
namespace emscripten {
namespace internal {
template <typename ClassType> void raw_destructor(ClassType*);
}  // namespace internal
}  // namespace emscripten

EMSCRIPTEN_BINDINGS(Skia) {
#ifdef ENABLE_GPU
    constant("gpu", true);
#endif  // ENABLE_GPU

#ifdef CK_ENABLE_WEBGL
    function("_MakeWebGLDirectContext", &MakeWebGLDirectContext);
    constant("webgl", true);
    function("_MakeOnScreenGLSurface",
             select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int)>(
                     &MakeOnScreenGLSurface));
    function("_MakeOnScreenGLSurface",
             select_overload<sk_sp<SkSurface>(sk_sp<GrDirectContext>, int, int, int, int)>(
                     &MakeOnScreenGLSurface));
#endif  // CK_ENABLE_WEBGL

#ifdef ENABLE_GPU
    class_<GrDirectContext>("GrDirectContext")
            .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>");
#endif  // ENABLE_GPU

    class_<SkSurface>("Surface")
            .smart_ptr<sk_sp<SkSurface>>("sk_sp<Surface>")
            .class_function(
                    "_makeRasterDirect",
                    optional_override([](int width, int height, WASMPointerU8 pPtr, size_t rowBytes)
                                              -> sk_sp<SkSurface> {
                        uint8_t* pixels = reinterpret_cast<uint8_t*>(pPtr);
                        SkImageInfo imageInfo =
                                SkImageInfo::Make(width,
                                                  height,
                                                  SkColorType::kRGBA_8888_SkColorType,
                                                  SkAlphaType::kUnpremul_SkAlphaType,
                                                  SkColorSpace::MakeSRGB());
                        return SkSurfaces::WrapPixels(imageInfo, pixels, rowBytes, nullptr);
                    }),
                    allow_raw_pointers())
            .function("_flush", optional_override([](SkSurface& self) {
#ifdef CK_ENABLE_WEBGL
                          skgpu::ganesh::FlushAndSubmit(&self);
#endif
                      }))
            .function("height", &SkSurface::height)
            .function("width", &SkSurface::width);

    function("drawSkiaLogo", optional_override([]() {
                 SWB::WebSurface webSurface("canvasToDrawOn");
                 webSurface.makeCurrent();

                 SkCanvas* canvas = webSurface.surface().getCanvas();
                 const SkColor background = SK_ColorWHITE;  // SK_ColorTRANSPARENT;
                 canvas->clear(background);

                 SWB::drawSkiaLogo(canvas);
                 webSurface.flush();
             }));
}
