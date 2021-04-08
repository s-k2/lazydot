/*
    lazydot - a simple dot preprocessor
    Copyright (C) 2021 Stefan Klein

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef FILTER_H
#define FILTER_H

#include <string>

extern void onStart();

extern void onGraph();

extern void onTemplateDefinition(std::string name);

extern void onNode(std::string title);
extern void onEdge();
extern void onNodeFlowEnd();

extern void onAttribute(std::string key, std::string value);
extern void onTemplateUse(std::string name);

extern void onClusterStart(std::string title);
extern void onClusterEnd();

extern void onEnd();

#endif /* FILTER_H */
