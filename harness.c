#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "apriltag.h"
#include "tag36h11.h"
#include "common/image_u8.h"

// AFL++ Persistent Mode
__AFL_FUZZ_INIT();

int main(int argc, char **argv) {
    apriltag_detector_t *td = apriltag_detector_create();
    apriltag_family_t *tf = tag36h11_create();
    apriltag_detector_add_family(td, tf);

    td->quad_decimate = 1.0;
    td->quad_sigma = 0.0;
    td->nthreads = 1;
    td->debug = 0;
    td->refine_edges = 1;

    unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;
    
    while (__AFL_LOOP(10000)) {
        int len = __AFL_FUZZ_TESTCASE_LEN;
        if (len < 5) continue;
        uint16_t width = (*(uint16_t *)buf) % 512;
        uint16_t height = (*(uint16_t *)(buf + 2)) % 512;
        if (width == 0) width = 1;
        if (height == 0) height = 1;
        int needed_pixels = width * height;
        int available_pixels = len - 4;
        if (available_pixels < needed_pixels) {
            height = available_pixels / width;
            if (height == 0) continue;
        }
        image_u8_t *im = image_u8_create(width, height);
        if (!im) continue;
        memcpy(im->buf, buf + 4, width * height);
        zarray_t *detections = apriltag_detector_detect(td, im);
        apriltag_detections_destroy(detections);
        image_u8_destroy(im);
    }

    apriltag_detector_remove_family(td, tf);
    tag36h11_destroy(tf);
    apriltag_detector_destroy(td);

    return 0;
}
