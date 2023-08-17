#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/inotify.h>
#include <ctime>
#include <fstream>
#include <string>
#include <dirent.h>

void logFileTransferEvent(const std::string& event, const std::string& source, const std::string& destination) {
    std::ofstream logFile("file_transfer_log.txt", std::ios::app);
    if (logFile.is_open()) {
        std::time_t now = std::time(nullptr);
        logFile << std::ctime(&now) << ": " << event << " - Source: " << source << " - Destination: " << destination << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file." << std::endl;
    }
}

void addWatchesRecursively(int fd, const std::string& dirPath) {
    int wd = inotify_add_watch(fd, dirPath.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE);
    if (wd == -1) {
        std::cerr << "Failed to add watch to directory: " << dirPath << std::endl;
    }

    DIR *dir = opendir(dirPath.c_str());
    if (!dir) {
        std::cerr << "Failed to open directory: " << dirPath << std::endl;
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_DIR && std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
            std::string subDirPath = dirPath + "/" + entry->d_name;
            addWatchesRecursively(fd, subDirPath);
        }
    }

    closedir(dir);
}

int main() {
    int fd = inotify_init();
    if (fd == -1) {
        std::cerr << "Failed to initialize inotify." << std::endl;
        return 1;
    }

    std::string dirPath = "/media/"; // Set the directory path you want to monitor

    addWatchesRecursively(fd, dirPath);

    char buffer[4096];
    while (true) {
        int length = read(fd, buffer, sizeof(buffer));
        if (length == -1) {
            std::cerr << "Error while reading events." << std::endl;
            break;
        }

        for (char* ptr = buffer; ptr < buffer + length;) {
            struct inotify_event* event = reinterpret_cast<struct inotify_event*>(ptr);
            if (event->len > 0) {
                std::string source = dirPath + "/" + event->name;
                std::string destination = "unknown"; // Set this based on your file transfer scenario

                if (event->mask & IN_CREATE) {
                    logFileTransferEvent("File created", source, destination);
                } else if (event->mask & IN_MODIFY) {
                    logFileTransferEvent("File modified", source, destination);
                } else if (event->mask & IN_DELETE) {
                    logFileTransferEvent("File deleted", source, destination);
                }
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    inotify_rm_watch(fd, IN_ALL_EVENTS);
    close(fd);

    return 0;
}
