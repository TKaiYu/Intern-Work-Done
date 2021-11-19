#include "ff.h"
#include "webserver.h"
#include "xil_printf.h"

int platform_init_fs()
{
	static FATFS fatfs;
	static FIL fil;		/* File object */
	FRESULT Res;
	TCHAR *Path = "mfsimage.img";

	/*
	 * Register volume work area, initialize device
	 */
	Res = f_mount(&fatfs, Path, 1);
	if (Res != FR_OK) {
		xil_printf("Failed to mount FAT FS \r\n");
		return -1;
	}

	Res = f_open(&fil, "index.htm", FA_READ);
	if (Res) {
		xil_printf("%s: ERROR: unable to locate index.html in FS\r\n",
			   __func__);
	}

	/* Closing the file */
	f_close(&fil);
	return 0;
}
