#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <libudev.h>

void displayHelp() {
    std::cout << "Usage: server_info [OPTIONS]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help     Display this help message" << std::endl;
    std::cout << "  -showlog       Show USB device log" << std::endl;
    std::cout << "  -showinfo      Show Hardware Information" << std::endl;
    std::cout << "  -enableusb        Enable a USB Connection" << std::endl;
    std::cout << "  -disableusb        Enable a USB Connection" << std::endl;
}

void displayLogs() {
    // Open and display the log file
    std::ifstream logFile("device_log.txt");
    if (logFile.is_open()) {
        std::cout << "USB Device Log:" << std::endl;
        std::string line;
        while (std::getline(logFile, line)) {
            std::cout << line << std::endl;
        }
        logFile.close();
    } else {
        std::cerr << "Log file not found." << std::endl;
    }
}

void displayHardwareInfo() {
    // Display CPU information using lscpu command
    std::cout << "CPU Information:" << std::endl;
    std::system("lscpu");

    // Display memory information using free command
    std::cout << "\nMemory Information:" << std::endl;
    std::system("free -h");

    // Display disk space information using df command
    std::cout << "\nDisk Space Information:" << std::endl;
    std::system("df -h");

    // Display block device information using lsblk command
    std::cout << "\nBlock Device Information:" << std::endl;
    std::system("lsblk");

    // Display PCI device information using lspci command
    std::cout << "\nPCI Device Information:" << std::endl;
    std::system("lspci");

    // Display DMI hardware information using dmidecode command
    std::cout << "\nDMI Hardware Information:" << std::endl;
    std::system("sudo dmidecode");

}

void createUdevRule() {
    std::ofstream ruleFile("/etc/udev/rules.d/99-block-all-usb.rules");
    if (ruleFile.is_open()) {
        ruleFile << "SUBSYSTEM==\"usb\", ENV{UDISKS_IGNORE}=\"1\"" << std::endl;
        ruleFile.close();
    } else {
        std::cerr << "Failed to create udev rule." << std::endl;
    }
}

void blockAllUsbDevices() {
    system("sudo udevadm control --reload-rules");
    std::cout << "Udev rules reloaded." << std::endl;

    createUdevRule();
    std::cout << "All USB devices blocked." << std::endl;
}

void removeUdevRule() {
    std::ifstream inputFile("/etc/udev/rules.d/99-block-all-usb.rules");
    std::ofstream outputFile("/etc/udev/rules.d/99-block-all-usb.rules.tmp");

    if (inputFile.is_open() && outputFile.is_open()) {
        std::string line;
        while (std::getline(inputFile, line)) {
            if (line.find("SUBSYSTEM==\"usb\", ENV{UDISKS_IGNORE}=\"1\"") == std::string::npos) {
                outputFile << line << std::endl;
            }
        }

        inputFile.close();
        outputFile.close();

        // Replace the original rule file with the modified temporary file
        std::string cmd = "sudo mv /etc/udev/rules.d/99-block-all-usb.rules.tmp /etc/udev/rules.d/99-block-all-usb.rules";
        system(cmd.c_str());

        std::cout << "Udev rule removed. All USB devices are now enabled." << std::endl;
    } else {
        std::cerr << "Failed to modify udev rule." << std::endl;
    }
}

void enableAllUsbDevices() {
    system("sudo udevadm control --reload-rules");
    std::cout << "Udev rules reloaded." << std::endl;

    removeUdevRule();
}

int main(int argc, char *argv[]) {

    // Get the hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        std::cerr << "Error getting hostname" << std::endl;
        return 1;
    }

    // Get the current time
    std::time_t now = std::time(nullptr);
    char timeString[100];
    std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    // Display server information
    std::cout << "Server Information:" << std::endl;
    std::cout << "Hostname: " << hostname << std::endl;
    std::cout << "Current Time: " << timeString << std::endl;

    // Display help if no arguments provided
    if (argc == 1) {
        displayHelp();
        return 0;
    }

    // Process command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            displayHelp();
        } else if (arg == "-showlog") {
            displayLogs();
        } else if (arg == "-showinfo") {
            displayHardwareInfo();
        } else if (arg == "-enableUSB") {
            std::cout << "Feature enabling USB Connection." << std::endl;
            enableAllUsbDevices();
        } else if (arg == "-disableUSB") {
            std::cout << "Feature disabling USB Connection." << std::endl;
            blockAllUsbDevices();
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            displayHelp();
        }
    }

    return 0;
}