#include <stdint.h>
#include <stdbool.h>

#include "driverlib.h"
#include "device.h"
#include "pin_map.h"   // <-- for GPIO_28_SCIRXDA etc

#define SCI_BASE     SCIA_BASE
#define SCI_BAUD     115200U

static void sci_putc(uint32_t base, uint16_t c)
{
    // Blocking write handles FIFO space internally
    SCI_writeCharBlockingFIFO(base, c);
}

static void sci_puts(uint32_t base, const char *s)
{
    while(*s)
    {
        if(*s == '\n') sci_putc(base, '\r');
        sci_putc(base, (uint16_t)*s++);
    }
}

static void initSCIA(void)
{
    // Pin mux: LaunchPad default SCIA is often GPIO28/29
    GPIO_setPinConfig(GPIO_28_SCIRXDA);
    GPIO_setPinConfig(GPIO_29_SCITXDA);

    GPIO_setDirectionMode(28, GPIO_DIR_MODE_IN);
    GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
    GPIO_setQualificationMode(28, GPIO_QUAL_ASYNC);

    SCI_performSoftwareReset(SCI_BASE);

    SCI_setConfig(SCI_BASE,
                  DEVICE_LSPCLK_FREQ,
                  SCI_BAUD,
                  (SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));

    SCI_enableFIFO(SCI_BASE);
    SCI_resetTxFIFO(SCI_BASE);
    SCI_resetRxFIFO(SCI_BASE);

    SCI_enableModule(SCI_BASE);
}

int main(void)
{
    Device_init();
    Device_initGPIO();

    Interrupt_initModule();
    Interrupt_initVectorTable();

    initSCIA();

    sci_puts(SCI_BASE, "\nSCIA echo test (F280039C)\n");
    sci_puts(SCI_BASE, "115200 8N1. Type and it will echo.\n");

    for(;;)
    {
        // Poll RX FIFO status; if not empty, read and echo
        if(SCI_getRxFIFOStatus(SCI_BASE) != SCI_FIFO_RX0)
        {
            uint16_t ch = SCI_readCharBlockingFIFO(SCI_BASE);
            sci_putc(SCI_BASE, ch);
        }
    }
}
