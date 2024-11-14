#pragma once

#include <emscripten.h>
#include <emscripten/bind.h>
#include "include/core/SkSurface.h"

namespace SWB {
namespace Impl {
emscripten::val& jsModule();
}

/// Draw the skia logo. Demo code.
auto drawSkiaLogo(SkCanvas* canvas) -> void;

/// A surface bound to a canvas in a web browser
class WebSurface {
public:
    WebSurface(const char* canvasID);
    ~WebSurface();

    auto surface() const -> SkSurface& { return *surface_; }
    auto makeCurrent() -> void;
    auto flush() -> void;

protected:
    emscripten::val surfaceJS_;
    emscripten::val contextJS_;
    SkSurface* surface_;

    /// What are we drawing on
    enum class BackendEnum {
        eCPU,
        eWebGL
        // eWebGPU // oneday!
    };
    BackendEnum backend_;
};
}  // namespace SWB
