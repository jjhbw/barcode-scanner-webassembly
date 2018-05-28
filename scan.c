#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <zbar.h>
#include "emscripten.h"

// External javascript function to pass the retrieved data to.
extern void js_output_result(const char *symbolName, const char *data, const int *polygon, const unsigned polysize);

zbar_image_scanner_t *scanner = NULL;

EMSCRIPTEN_KEEPALIVE
int scan_image(uint8_t *raw, int width, int height)
{
    // create the scanner
    scanner = zbar_image_scanner_create();

    // set the scanner density (function will have nonzero return code on error, check your browser console)
    printf("%d \n", zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_X_DENSITY, 1));
    printf("%d \n", zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_Y_DENSITY, 1));

    // hydrate a zbar image struct with the image data.
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, zbar_fourcc('Y', '8', '0', '0'));
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);

    // scan the image for barcodes
    int n = zbar_scan_image(scanner, image);

    // Iterate over each detected barcode and extract its data and location
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for (; symbol; symbol = zbar_symbol_next(symbol))
    {
        // Get the data encoded in the detected barcode.
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);

        // get the polygon describing the bounding box of the barcode in the image
        unsigned poly_size = zbar_symbol_get_loc_size(symbol);

        // return the polygon as a flat array.
        // (Parsing two-dimensional arrays from the wesm heap introduces unnecessary upstream code complexity)
        int poly[poly_size * 2];
        unsigned u = 0;
        for (unsigned p = 0; p < poly_size; p++)
        {
            poly[u] = zbar_symbol_get_loc_x(symbol, p);
            poly[u + 1] = zbar_symbol_get_loc_y(symbol, p);
            u += 2;
        }

        // Output the result to the javascript environment
        js_output_result(zbar_get_symbol_name(typ), data, poly, poly_size);
    }

    // clean up
    zbar_image_destroy(image);
    zbar_image_scanner_destroy(scanner);

    return (0);
}

// this function can be used from the javascript environment to
// allocate a buffer on the WebAssembly heap to accomodate the image that is to be scanned.
EMSCRIPTEN_KEEPALIVE
uint8_t *create_buffer(int width, int height)
{
    return malloc(width * height * 4 * sizeof(uint8_t));
}

// this function can be used from the javascript environment to free an image buffer.
EMSCRIPTEN_KEEPALIVE
void destroy_buffer(uint8_t *p)
{
    free(p);
}