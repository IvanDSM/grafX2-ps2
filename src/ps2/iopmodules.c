#include <debug.h>
#include "iopmodules.h"
#include <stdlib.h>
#include "usbd_irx.h"
#include "usbhdfsd_irx.h"

int Ps2_load_modules()
{
	int sifreturn;
	scr_printf("Starting IOP... ");
	SifInitRpc(0);
	// Reset IOP borrowed from uLaunchelf
    while (!SifIopReset(NULL, 0)){};
    while (!SifIopSync()){};
    SifInitRpc(0);
	scr_printf("ok!\n");
	scr_printf("Loading SIO2MAN... ");
	sifreturn = SifLoadModule("rom0:SIO2MAN", 0, 0);
	scr_printf("%d\nLoading USBD... ", sifreturn);
	sifreturn = SifExecModuleBuffer(usbd_irx, size_usbd_irx, 0, NULL, &sifreturn);
	scr_printf("%d\nLoading USBHDFSD... ", sifreturn);
	sifreturn = SifExecModuleBuffer(usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, &sifreturn);
	scr_printf("%d\nAll modules (hopefully) loaded.", sifreturn);
	while(1){};
	return sifreturn;
}
