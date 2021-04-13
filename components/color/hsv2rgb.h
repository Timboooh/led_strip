#ifndef HSV2RGB_H
#define HSV2RGB_H

rgb_t hsv2rgb_raw(hsv_t hsv);
rgb_t hsv2rgb_spectrum(hsv_t hsv);
rgb_t hsv2rgb_rainbow(hsv_t hsv);
hsv_t rgb2hsv_approximate(rgb_t rgb);

#endif