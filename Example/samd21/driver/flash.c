#include "flash.h"

uint32_t PAGE_SIZE, PAGES, MAX_FLASH;

void flash_init(void)
{
    //const uint32_t pageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
    //PAGE_SIZE = pageSizes[NVMCTRL->PARAM.bit.PSZ];
    PAGE_SIZE = 8 << NVMCTRL->PARAM.bit.PSZ;

    PAGES = NVMCTRL->PARAM.bit.NVMP;
    MAX_FLASH = PAGE_SIZE * PAGES;
}


void flash_write(uint32_t *dst_addr, uint32_t *src_addr, uint32_t size)
{
    uint32_t i;

    size /= 4;

    // Set automatic page write
    NVMCTRL->CTRLB.bit.MANW = 0;

    // Do writes in pages
    while (size)
    {
        // Execute "PBC" Page Buffer Clear
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
        while (NVMCTRL->INTFLAG.bit.READY == 0)
            ;

        // Fill page buffer
        for (i=0; i<(PAGE_SIZE/4) && i<size; i++)
        {
            dst_addr[i] = src_addr[i];
        }

        // Execute "WP" Write Page
        NVMCTRL->ADDR.reg = ((uint32_t)dst_addr) / 2;
        NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
        while (NVMCTRL->INTFLAG.bit.READY == 0)
            ;

        // Advance to next page
        dst_addr += i;
        src_addr += i;
        size     -= i;
    }
}


void flash_erase(uint32_t dst_addr, int32_t size)
{
//	while (size > 0)
//	{
//		if(dst_addr >= APP_START_ADDRESS && dst_addr < MAX_FLASH)   // protect the bootloader
//		{
//			// Execute "ER" Erase Row
//			NVMCTRL->ADDR.reg = dst_addr / 2;
//			NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
//			while (NVMCTRL->INTFLAG.bit.READY == 0)
//				;
//		}

//		dst_addr += PAGE_SIZE * 4; // Skip a ROW
//		size -= PAGE_SIZE * 4;
//	}
}


