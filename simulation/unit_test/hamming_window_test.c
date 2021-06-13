#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dsputil.h"

#define TEST_SINE_LEN  (50)
#define FS             (32.0)

// answer
// 0.0
// 0.016344813419418954
// 0.036373477720883195
// 0.06312632282453316
// 0.09842365038460163
// 0.1424773037717253
// 0.19361183630456633
// 0.2483333530749989
// 0.3015394185771585
// 0.34700670865015587
// 0.377983482847331
// 0.3879646361486827
// 0.37140871156957794
// 0.3245651808693817
// 0.2458310302555375
// 0.13634913217409467
// -0.0
// -0.15669918177932426
// -0.32501119332338274
// -0.49463131225614987
// -0.6544355655257494
// -0.7936215822839963
// -0.9022489981450521
// -0.972480081741345
// -0.9990548806651547
// -0.9798730269563837
// -0.916062752366261
// -0.8120143326741107
// -0.6748885397871482
// -0.5142192054958371
// -0.3407044694032192
// -0.16569031569739215
// 0.0
// 0.146864961395954
// 0.2674567549104358
// 0.3568950102168191
// 0.4130670255448879
// 0.43675059209461775
// 0.4310770022101844
// 0.4012622577948506
// 0.3537996621636989
// 0.29574986174047735
// 0.23392657514884949
// 0.17424855708112041
// 0.12116139686949744
// 0.07733585087496063
// 0.04348172020329168
// 0.018543155221699272
// -0.0
// -0.015608000000000002

int main() {
    printf("Start test Hamming window\n");

    float32_t test_input[TEST_SINE_LEN];
    float32_t test_output[TEST_SINE_LEN];

    for(int i=0;i<TEST_SINE_LEN;i++) {
        test_input[i] = sinf(2*3.141593*1*i/FS);
    }

    dsp_hamming_window(test_input, test_output, TEST_SINE_LEN);

    for(int i=0;i<TEST_SINE_LEN;i++) {
        printf("%.4f, %.4f\n", test_input[i], test_output[i]);
    }


    printf("end\n");
}


