/*

WXChip - chip8 emulator using XChip library and a wxWidgets gui.
Copyright (C) 2016  Jared Bruni.


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.html.

*/

#ifndef WXCHIP_SAVELIST_H_
#define WXCHIP_SAVELIST_H_

#include <iostream>
#include <fstream>
#include <string>

void saveDirectory(const std::string &text, const std::string &fps, const std::string &cpufreq);
std::string getDirectory(std::string &fps, std::string &cpufreq);



#endif // WXCHIP_SAVELIST_H_