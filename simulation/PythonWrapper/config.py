CMSISDSP = 1

ROOT="../../thirdparty/CMSIS_5/CMSIS/DSP"
ALGO_PATH = "../../dsptest_rtos/Bio-Detection_SAME_Platform/Bio-Detection_SAM_Platform/algo"
FREERTOS_PATH = "../../thirdparty/FreeRTOS"

config = CMSISDSP

if config == CMSISDSP:
    extensionName = 'cmsisdsp' 
    setupName = 'CMSISDSP'
    setupDescription = 'CMSIS-DSP Python API'
    cflags="-DCMSISDSP"

