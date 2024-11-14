#include "./skiaWebBinding.h"

#include "include/gpu/ganesh/SkSurfaceGanesh.h"

namespace SWB {
using namespace emscripten;

namespace Impl {
emscripten::val& jsModule() {
    static emscripten::val theModule = val::global("SkiaWebBinding");
    return theModule;
}
}  // namespace Impl

WebSurface::WebSurface(const char* canvasID) {
    surfaceJS_ = Impl::jsModule().call<val>("MakeSurface", val(canvasID));
    assert(not(surfaceJS_.isNull() or surfaceJS_.isUndefined()));
    surface_ = &surfaceJS_.as<SkSurface&>();

    contextJS_ = surfaceJS_["_context"];  // can be undefined

    if (surfaceJS_["isSoftware"].isTrue()) {
        backend_ = BackendEnum::eCPU;
    } else if (surfaceJS_["isWebGL"].isTrue()) {
        backend_ = BackendEnum::eWebGL;
    } else {
        assert(false);
    }
}

WebSurface::~WebSurface() { surfaceJS_.call<val>("delete"); }

auto WebSurface::flush() -> void {
    switch (backend_) {
        case BackendEnum::eCPU:
            surfaceJS_.call<val>("flush");
            break;
        case BackendEnum::eWebGL:
            skgpu::ganesh::FlushAndSubmit(surface_);
            break;
    }
}

auto WebSurface::makeCurrent() -> void {
    switch (backend_) {
        case BackendEnum::eCPU:
            break;

        case BackendEnum::eWebGL:
            Impl::jsModule().call<val>("setCurrentContext", contextJS_);
            break;
    }
}
}  // namespace SWB
