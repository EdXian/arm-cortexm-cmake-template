#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "dsputil.h"
#include "pconfig.h"

#define SAVE_FILE_NAME "stft.txt"

uint32_t data_len = 0;

static float32_t data_temp[STFT_PRE_SAMPLES];
static float32_t result[STFT_PRE_SAMPLES >> 1];
static int count = 0;
static int shift_count = 0;
static int c = 0;

static int pressure_arr[4] = {0,0,0,0};
static int p_cnt = 0;

int main(int argc, char *argv[]) {
    printf("Start \n");
    memset(data_temp, 0, sizeof(data_temp));
    memset(result, 0, sizeof(result));

    // Read the file
    if(argc < 2) {
        return 0;
    }
    char file_name[50];
    memset(file_name, 0, sizeof(file_name));
    sprintf(file_name, "%s", argv[1]);
    printf("load the file: %s\n", file_name);

    // load the data
    FILE *pFile = fopen((const char *)file_name, "r");
    printf("filename %s\n", file_name);
    char s[100];
    if(pFile!=NULL){
        printf("open file succeed\n");
        memset(s, 0, sizeof(s));
        while (fgets(s, 60, pFile) !=NULL){

            //printf("data count %d\n",count);
            uint32_t time;
            int32_t p;
            int hr;
            int br;
            int32_t acc;
            int32_t accx;
            int32_t accy;
            int32_t accz;
            int32_t pressure;
            //sscanf(s,"%d, %d, %d, %d, %d\n",&time,&sd.pressure,&sd.accx,&sd.accy,&sd.accz,&hr,&br,&acc);

            sscanf(s,"%d, %d, %d, %d, %d\n",&time,&pressure,&accx,&accy,&accz,&hr,&br,&acc);

            //-933353
            //-2592
            pressure -= PRESSURE_DC;


            pressure_arr[p_cnt++] = pressure;
            
            // Simulate the four data
            if(p_cnt == 4) {
                p_cnt = 0;
            
                // printf("%d\n",pressure);
                assert(count<STFT_PRE_SAMPLES);

                for(int i=0;i<4;i++) {
                    data_temp[count++] = (float32_t)pressure_arr[i];
                }

                if((STFT_PRE_SAMPLES == count) && (0 == shift_count)) {
                    dsp_stft(data_temp, result, STFT_PRE_SAMPLES);
                    dsp_reserve_freq_band(result,result,1024,50,1024,0,1);
                    dsp_normalize(result,1024);
                    shift_count++;
                    count = STFT_PRE_SAMPLES - SHIFT;

                    /* Save file start */
                    FILE *f = fopen(SAVE_FILE_NAME, "a");
                    char temp_str[20];
                    memset(temp_str, 0, sizeof(temp_str));
                    sprintf(temp_str, "==p%d==\n", shift_count-1);
                    fwrite(temp_str, 1, strlen(temp_str), f);
                    for(int i=0;i<(STFT_PRE_SAMPLES>>1);i++) {
                        memset(temp_str, 0, sizeof(temp_str));
                        sprintf(temp_str, "%.5f\n", result[i]);
                        fwrite(temp_str, 1, strlen(temp_str), f);
                    }
                    fclose(f);
                    /* Save file end */
                }
                else if((STFT_PRE_SAMPLES == count) && (0 < shift_count)) {
                    for(int i=0;i<STFT_PRE_SAMPLES-SHIFT;i++) {
                        data_temp[i] = data_temp[i+SHIFT];
                    }
                    dsp_stft(data_temp, result, STFT_PRE_SAMPLES);
                    dsp_reserve_freq_band(result,result,1024,20,1024,0,1);
                    dsp_normalize(result,1024);
                    count = STFT_PRE_SAMPLES - SHIFT;
                    shift_count++;

                    /* Save file start */
                    FILE *f = fopen(SAVE_FILE_NAME, "a");
                    char temp_str[20];
                    memset(temp_str, 0, sizeof(temp_str));
                    sprintf(temp_str, "==p%d==\n", shift_count-1);
                    fwrite(temp_str, 1, strlen(temp_str), f);
                    for(int i=0;i<(STFT_PRE_SAMPLES>>1);i++) {
                        memset(temp_str, 0, sizeof(temp_str));
                        sprintf(temp_str, "%.5f\n", result[i]);
                        fwrite(temp_str, 1, strlen(temp_str), f);
                    }
                    fclose(f);
                    /* Save file end */
                }
            }
            data_len++;
        }
        
        fclose(pFile);
        pFile=NULL;
        printf("close file\n");
        
    }
    else {
        printf("open file fail\n");
        return 0;
    }

    printf("data length: %d\n", data_len);
    printf("data length: %d\n", shift_count);

    printf("end\n");
}


