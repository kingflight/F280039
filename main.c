//#############################################################################
// SCIA Echo / Heartbeat Test for F280039C (Driverlib)
//#############################################################################

#include "driverlib.h"
#include "device.h"

// -------------------- user config --------------------
#define SCI_BAUDRATE      115200U
#define SCI_BASE          SCIA_BASE

// F28003x commonly uses SCIA on GPIO28/29
#define SCIRX_GPIO        28U
#define SCITX_GPIO        29U
#define SCIRX_PINCONFIG   GPIO_28_SCIRXDA
#define SCITX_PINCONFIG   GPIO_29_SCITXDA
// -----------------------------------------------------

static inline void sci_putc(uint16_t base, uint16_t c)
{
    // Wait for space in TX FIFO (or TX ready)
    while(SCI_getTxFIFOStatus(base) == SCI_FIFO_TXFULL)
    {
    }
    SCI_writeCharBlockingFIFO(base, (uint16_t)c);
}

static void sci_puts(uint16_t base, const char *s)
{
    while(*s)
    {
        if(*s == '\n')
            sci_putc(base, '\r');
        sci_putc(base, (uint16_t)*s++);
    }
}

static void sci_print_u32_hex(uint16_t base, uint32_t x)
{
    const char *hex = "0123456789ABCDEF";
    sci_puts(base, "0x");
    for(int i = 7; i >= 0; --i)
    {
        sci_putc(base, hex[(x >> (i * 4)) & 0xF]);
    }
}

static void initSCIA(uint32_t baudrate)
{
    // GPIO mux for SCIA
    GPIO_setPinConfig(SCIRX_PINCONFIG);
    GPIO_setPinConfig(SCITX_PINCONFIG);

    // Optional pad config (depends on board)
    GPIO_setDirectionMode(SCIRX_GPIO, GPIO_DIR_MODE_IN);
    GPIO_setDirectionMode(SCITX_GPIO, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(SCIRX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setPadConfig(SCITX_GPIO, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(SCIRX_GPIO, GPIO_QUAL_ASYNC);

    // Reset + configure SCI
    SCI_performSoftwareReset(SCI_BASE);

    // 8-N-1, async mode, internal clock
    SCI_setConfig(SCI_BASE,
                  DEVICE_LSPCLK_FREQ,
                  baudrate,
                  (SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));

    // Enable FIFO and clear
    SCI_enableFIFO(SCI_BASE);
    SCI_resetTxFIFO(SCI_BASE);
    SCI_resetRxFIFO(SCI_BASE);

    // Enable SCI module
    SCI_enableModule(SCI_BASE);

    // Enable TX/RX
    SCI_enableTx(SCI_BASE);
    SCI_enableRx(SCI_BASE);
}

int main(void)
{
    Device_init();
    Device_initGPIO();

    // If you use interrupts elsewhere, this is fine; here we stay polling.
    Interrupt_initModule();
    Interrupt_initVectorTable();

    initSCIA(SCI_BAUDRATE);

    sci_puts(SCI_BASE, "\nSCIA test on F280039C\n");
    sci_puts(SCI_BASE, "Baud: 115200 8N1\n");
    sci_puts(SCI_BASE, "Type characters; they will echo back.\n");
    sci_puts(SCI_BASE, "-------------------------------------\n");

    uint32_t lastHeartbeat = 0;
    uint32_t echoCount = 0;

    for(;;)
    {
        // Heartbeat every ~1s (coarse)
        uint32_t now = DEVICE_DELAY_US(0), dummy = now; // not a real timer read
        (void)dummy;

        // Use a simple delay for heartbeat (no timer dependency)
        if(lastHeartbeat == 0)
        {
            lastHeartbeat = 1;
        }

        // Periodic heartbeat line (about every 1 second)
        DEVICE_DELAY_US(1000000);
        sci_puts(SCI_BASE, "[HB] echoCount=");
        sci_print_u32_hex(SCI_BASE, echoCount);
        sci_puts(SCI_BASE, "\n");

        // Echo loop: poll RX FIFO and echo everything available
        while(SCI_getRxFIFOStatus(SCI_BASE) != SCI_FIFO_RX0)
        {
            uint16_t ch = SCI_readCharBlockingFIFO(SCI_BASE);

            // Echo back
            sci_putc(SCI_BASE, ch);

            // Count received bytes
            echoCount++;
        }
    }
}
