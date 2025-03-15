#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

typedef struct {
    int file; 
    int addr; 
} I2CDevice;

// try to open I2C device
int i2c_open(I2CDevice *dev, int bus, int address) {
    char filename[20];
    snprintf(filename, 19, "/dev/i2c-%d", bus);

    dev->file = open(filename, O_RDWR);
    if (dev->file < 0) {
        perror("Failed to open I2C bus");
        return -1;
    }

    dev->addr = address;
    if (ioctl(dev->file, I2C_SLAVE, dev->addr) < 0) {
        perror("Failed to connect to device");
        return -1;
    }

    return 0;
}

// write to I2C reg
int i2c_write_register(I2CDevice *dev, uint16_t reg, uint8_t value) {
    uint8_t buffer[3] = { (uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF), value };
    if (write(dev->file, buffer, 3) != 3) {
        perror("I2C Write failed");
        return -1;
    }
    return 0;
}

// read from I2C reg
uint8_t i2c_read_register(I2CDevice *dev, uint16_t reg) {
    uint8_t buffer[2] = { (uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF) };
    if (write(dev->file, buffer, 2) != 2) {
        perror("I2C Register write failed");
        return 0;
    }

    uint8_t data;
    if (read(dev->file, &data, 1) != 1) {
        perror("I2C Read failed");
        return 0;
    }
    return data;
}

// close I2C device
void i2c_close(I2CDevice *dev) {
    close(dev->file);
}
