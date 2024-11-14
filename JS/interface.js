// Adds JS functions to augment the SkiaWebBinding interface.
// For example, if there is a wrapper around the C++ call or logic to allow
// chaining, it should go here.

// SkiaWebBinding.onRuntimeInitialized is called after the WASM library has loaded.
// Anything that modifies an exposed class (e.g. Path) should be set
// after onRuntimeInitialized, otherwise, it can happen outside of that scope.
SkiaWebBinding.onRuntimeInitialized = function() {
    // All calls to 'this' need to go in externs.js so closure doesn't minify them away.

    // Run through the JS files that are added at compile time.
    if (SkiaWebBinding._extraInitializations) {
        SkiaWebBinding._extraInitializations.forEach(function(init) {
            init();
        });
    }
}; // end SkiaWebBinding.onRuntimeInitialized, that is, anything changing prototypes or dynamic.