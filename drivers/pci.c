#include "pci.h"
#include "port.h"

static uint32_t pci_addr(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    return 0x80000000u
        | ((uint32_t)bus << 16)
        | ((uint32_t)slot << 11)
        | ((uint32_t)function << 8)
        | (offset & 0xfc);
}

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset) {
    outl(0xCF8, pci_addr(bus, slot, function, offset));
    return inl(0xCFC);
}

void pci_write_config(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value) {
    outl(0xCF8, pci_addr(bus, slot, function, offset));
    outl(0xCFC, value);
}

void pci_init(void) {
}

void pci_scan(pci_scan_cb_t cb, void *ctx) {
    for (uint16_t bus = 0; bus < 256; ++bus) {
        for (uint8_t slot = 0; slot < 32; ++slot) {
            for (uint8_t function = 0; function < 8; ++function) {
                uint32_t id = pci_read_config(bus, slot, function, 0x00);
                if ((id & 0xffff) == 0xffff) {
                    if (function == 0) {
                        break;
                    }
                    continue;
                }

                uint32_t class_reg = pci_read_config(bus, slot, function, 0x08);
                uint32_t header = pci_read_config(bus, slot, function, 0x0C);

                pci_device_t dev = {
                    .bus = (uint8_t)bus,
                    .slot = slot,
                    .function = function,
                    .vendor_id = id & 0xffff,
                    .device_id = (id >> 16) & 0xffff,
                    .prog_if = (class_reg >> 8) & 0xff,
                    .subclass = (class_reg >> 16) & 0xff,
                    .class_code = (class_reg >> 24) & 0xff,
                    .header_type = (header >> 16) & 0xff
                };

                for (uint8_t i = 0; i < 6; ++i) {
                    dev.bar[i] = pci_read_config(bus, slot, function, 0x10 + i * 4);
                }

                if (cb(&dev, ctx)) {
                    return;
                }

                if (function == 0 && !(dev.header_type & 0x80)) {
                    break;
                }
            }
        }
    }
}
