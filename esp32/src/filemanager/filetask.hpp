#ifndef FILETASK_H
#define FILETASK_H

#include "hardware/printer.hpp"

void file_task(void *arg);

extern FileManager fileManager;
extern Printer printer;

#endif