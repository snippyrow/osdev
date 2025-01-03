#include "ATA.h"

// Wait until the drive is not busy
void ATA_Wait_Busy(uint16_t io_base) {
    while (inb(io_base + ATA_REG_STATUS) & ATA_SR_BSY);
}

// Wait until the drive is ready to transfer data
void ATA_Wait_Ready(uint16_t io_base) {
    while (!(inb(io_base + ATA_REG_STATUS) & ATA_SR_DRQ));
}

void ide_400ns_delay(uint16_t io_base){
	for(int i = 0;i < 4; i++) {
		inb(io_base + ATA_REG_ALTSTATUS);
    }
}

// Read a sector from the primary ATA drive to a 512-byte buffer
void ATA_Read_Single(uint32_t lba, uint8_t *buffer) {
    // Select the master drive and specify the LBA
    outb(ATA_IO_BASE + ATA_REG_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));  // 0xE0 selects master drive
    outb(ATA_IO_BASE + ATA_REG_SECTOR_COUNT, 1);
    outb(ATA_IO_BASE + ATA_REG_LBA_LOW, lba & 0xFF);                        // LBA low byte
    outb(ATA_IO_BASE + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);                 // LBA mid byte
    outb(ATA_IO_BASE + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);               // LBA high byte

    // Send the "read sectors" command
    outb(ATA_IO_BASE + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    // Wait for the drive to signal that data is ready
    ATA_Wait_Busy(ATA_IO_BASE);
    ATA_Wait_Ready(ATA_IO_BASE);

    // Read the 512 bytes from the data register into the buffer
    for (uint32_t i = 0; i < (512 / 2); i++) {
        uint16_t data = inw(ATA_IO_BASE + ATA_REG_DATA);
        ((uint16_t *)buffer)[i] = data;  // Read 16 bits at a time
    }
}

// Write a sector to the primary ATA drive from a 512-byte buffer
void ATA_Write_Single(uint32_t lba, uint8_t *buffer) {
    // Select the master drive and specify the LBA
    outb(ATA_IO_BASE + ATA_REG_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));  // 0xE0 selects master drive
    outb(ATA_IO_BASE + ATA_REG_SECTOR_COUNT, 1);
    outb(ATA_IO_BASE + ATA_REG_LBA_LOW, lba & 0xFF);                        // LBA low byte
    outb(ATA_IO_BASE + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);                 // LBA mid byte
    outb(ATA_IO_BASE + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);               // LBA high byte

    // Send the "write sectors" command
    outb(ATA_IO_BASE + ATA_REG_COMMAND, ATA_CMD_WRITE_SECTORS);

    // Wait for the drive to be ready to accept data
    ATA_Wait_Busy(ATA_IO_BASE);
    ATA_Wait_Ready(ATA_IO_BASE);
    // Write the 512 bytes from the buffer into the data register
    for (uint32_t i = 0; i < (512 / 2); i++) {
        outw(ATA_IO_BASE + ATA_REG_DATA, ((uint16_t *)buffer)[i]);  // Write 16 bits at a time
    }

    // Send the "flush cache" command if required (optional but recommended)
    outb(ATA_IO_BASE + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);

    // Wait for the drive to finish flushing its cache
    ATA_Wait_Busy(ATA_IO_BASE);
}

void ATA_Write(uint32_t lba, uint16_t num_sectors, uint8_t *buffer) {
    for (uint16_t s=0;s<num_sectors;s++) {
        ATA_Write_Single(lba + s, buffer + (s * 512));
    }
}

void ATA_Read(uint32_t lba, uint16_t num_sectors, uint8_t *buffer) {
    for (uint16_t s=0;s<num_sectors;s++) {
        ATA_Read_Single(lba + s, buffer + (s * 512));
    }
}


uint32_t pci_config_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    return (1 << 31) | // Enable bit
           (bus << 16) |
           (device << 11) |
           (function << 8) |
           (offset & 0xFC); // Offset must be 4-byte aligned
}

uint32_t read_pci_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = pci_config_address(bus, device, function, offset);
    outl(PCI_CONFIG_ADDR, address);
    return inl(PCI_CONFIG_DATA);
}