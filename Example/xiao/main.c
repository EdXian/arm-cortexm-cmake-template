#include "sam.h"
#include "clock.h"

void SysTick_Handler(){

}
int main(){

    clock_source_init();

    clock_pin_test();

    while (1) {

    }



}
