#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <libudev.h>

void logDeviceInfo(const char *devnode, const char *vendorId, const char *productId) {
    std::time_t currentTime = std::time(nullptr);
    std::ofstream logFile("device_log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << "Timestamp: " << std::ctime(&currentTime);
        logFile << "USB Device Connected:" << std::endl;
        logFile << "Device Node: " << devnode << std::endl;
        logFile << "Vendor ID: " << vendorId << std::endl;
        logFile << "Product ID: " << productId << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file." << std::endl;
    }
}

int monitorUSBDevice() {
    struct udev *udev;
    struct udev_monitor *monitor;
    struct udev_device *dev;
    int fd;

    udev = udev_new();
    if (!udev) {
        std::cerr << "Failed to initialize udev." << std::endl;
        return 1;
    }

    monitor = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(monitor, "usb", NULL); // Filter for USB subsystem
    udev_monitor_enable_receiving(monitor);
    fd = udev_monitor_get_fd(monitor);

    std::cout << "Monitoring for USB device connections..." << std::endl;

    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int ret = select(fd + 1, &fds, NULL, NULL, NULL);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            dev = udev_monitor_receive_device(monitor);
            if (dev) {
                const char *devnode = udev_device_get_devnode(dev);
                const char *vendorId = udev_device_get_property_value(dev, "ID_VENDOR_ID");
                const char *productId = udev_device_get_property_value(dev, "ID_MODEL_ID");

                if (devnode && vendorId && productId) {
                    logDeviceInfo(devnode, vendorId, productId);
                    std::cout << "Logged USB device information." << std::endl;
                }

                udev_device_unref(dev);
            }
        }
    }

    udev_unref(udev);
    return 0;
}

int main() {
    monitorUSBDevice();
}