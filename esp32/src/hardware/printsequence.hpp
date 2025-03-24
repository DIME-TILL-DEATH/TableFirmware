#ifndef PRINTSEQEUNCE_HPP
#define PRINTSEQUENCE_HPP

#include "hardware/abstractprinter.hpp"
#include "hardware/polarprinter.hpp"
#include "hardware/decartprinter.hpp"

extern AbstractPrinter* printer;
void Printer_Init();

#endif