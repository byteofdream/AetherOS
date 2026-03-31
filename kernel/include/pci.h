#ifndef DRIVERS_PCI_H
#define DRIVERS_PCI_H

#include "common.h"

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t header_type;
    uint32_t bar[6];
} pci_device_t;

typedef bool (*pci_scan_cb_t)(const pci_device_t *dev, void *ctx);

void pci_init(void);
void pci_scan(pci_scan_cb_t cb, void *ctx);
uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset);
void pci_write_config(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset, uint32_t value);

#endif
