#ifndef FILETASK_H
#define FILETASK_H

#include "hardware/abstractprinter.hpp"
#include "hardware/polarprinter.hpp"

void file_task(void *arg);

extern FileManager fileManager;

extern AbstractPrinter* printer;


#endif