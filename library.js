
mergeInto(LibraryManager.library, {
    js_output_result: function (symbol, data, polygon, polygon_size) {

        // function provided by Emscripten to convert WASM heap string pointers to JS strings.
        const Pointer_stringify = Module['Pointer_stringify'];

        // Note: new TypedArray(someBuffer) will create a new view onto the same memory chunk, 
        // while new TypedArray(someTypedArray) will copy the data so the original can be freed.
        const resultView = new Int32Array(Module.HEAP32.buffer, polygon, polygon_size * 2);
        const coordinates = new Int32Array(resultView);

        // call the downstream processing function that should have been set by the client code
        const downstreamProcessor = Module['processResult'];
        if (downstreamProcessor == null) {
            throw new Error("No downstream processing function set")
        }
        downstreamProcessor(Pointer_stringify(symbol), Pointer_stringify(data), coordinates)

    }
});