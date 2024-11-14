SkiaWebBinding = null;

const loadSkiaWebBindings = async function() {
    if (SkiaWebBinding === null) {
        const swbLoader = SkiaWebBindingInit({
            locateFile: (file) => '/build/' + file
        });
        SkiaWebBinding = await swbLoader;
    }
    return SkiaWebBinding;
}();