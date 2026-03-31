#include "usb.h"
#include "pci.h"
#include "console.h"

typedef struct {
    int count;
} usb_scan_ctx_t;

static bool usb_scan_cb(const pci_device_t *dev, void *ctx_ptr) {
    usb_scan_ctx_t *ctx = ctx_ptr;
    if (dev->class_code == 0x0C && dev->subclass == 0x03) {
        console_printf("USB controller: bus=%u slot=%u if=%u vendor=%x\n",
                       dev->bus, dev->slot, dev->prog_if, dev->vendor_id);
        ctx->count++;
    }
    return false;
}

void usb_init(void) {
    usb_scan_ctx_t ctx = {0};
    pci_scan(usb_scan_cb, &ctx);
    if (ctx.count == 0) {
        console_writeln("USB: no UHCI/EHCI controller found");
    }
}
