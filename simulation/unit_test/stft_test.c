#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dsputil.h"


#define TEST_SINE_LEN  (2048)
#define FS             (32.0)

int main() {
    printf("Start test sine, len:%d, FS: %.2f\n", TEST_SINE_LEN, FS);

    // 2048 sine 1Hz
    float test_sine[TEST_SINE_LEN];

    for(int i=0;i<TEST_SINE_LEN;i++) {
        test_sine[i] = sinf(2*3.141593*1*i/FS);
    }
    float result[TEST_SINE_LEN>>1];
    memset(result, 0, TEST_SINE_LEN*sizeof(float32_t));

    dsp_hamming_window(test_sine, test_sine, TEST_SINE_LEN);
    dsp_stft(test_sine, result, TEST_SINE_LEN);

    float max_d = 0;
    unsigned int ii = 0;
    arm_max_f32(result, TEST_SINE_LEN, &max_d, &ii);

    printf("%.4f\n", max_d);
    printf("index: %d\n", ii);

    printf("end\n");
}


