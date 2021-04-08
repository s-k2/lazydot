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

#include "filter.h"

#include <algorithm>
#include <string>
#include <stdio.h>
#include <vector>
#include <map>

extern FILE *yyout;

typedef std::map<std::string, std::string> AttributesList;

struct FlowItem
{
        std::string title;
        AttributesList edgeAttributes;
};

static struct {
        std::map<std::string, AttributesList> templates;

        std::string currentFlowStart;
        std::string currentFlowEnd;
        std::string lastFlowStart;
        std::string lastFlowEnd;

        std::vector<std::string> alreadyDeclaredNodes;

        AttributesList edgeAttributesForNextNode;
        std::vector<std::vector<FlowItem>> flows;

        AttributesList attributes;

        size_t intendLevel;
        size_t subgraphCounter;
} state;

void out(const std::string &str)
{
        fwrite(str.c_str(), 1, str.size(), yyout);
}

void intendedOut(const std::string &str)
{
        for(size_t i = 0; i <= state.intendLevel; i++)
                fprintf(yyout, "  ");
        out(str);
}

std::string quoteString(const std::string &str)
{
        return('"' + str + '"');
}

std::string formatAttribute(std::map<std::string, std::string> &attributes, char separator = ',', bool inBrackets = true)
{
        if(attributes.empty())
                return("");

        std::string out = inBrackets ? " [ " : "";

        for(auto it = attributes.begin(); it != attributes.end(); ++it) {
                if(it != attributes.begin())
                        out += separator, out += " ";
                out += it->first + "=\"" + it->second + "\"";
        }

        out += inBrackets ? " ] " : "";
        return(out);
}

void onStart()
{
        state.flows.emplace_back(std::vector<FlowItem>());
        state.intendLevel = 0;

        out("digraph G {\n");
}

void onGraph()
{
        intendedOut(formatAttribute(state.attributes, ';', false) + "\n\n");
        state.attributes.clear();
}

void onTemplateDefinition(std::string name)
{
        if(state.templates.find(name) != state.templates.end())
                fprintf(stderr, "Warning: Template '%s' redefined, discarding old attributes.\n", name.c_str());
        state.templates.insert(std::make_pair(name, state.attributes));
        state.attributes.clear();
}

void onNode(std::string title)
{
        if(title == ".")
                title = state.lastFlowStart;
        else if(title == "$")
                title = state.lastFlowEnd;

        if(state.currentFlowStart.empty())
                state.currentFlowStart = title;
        state.currentFlowEnd = title;

        if(std::find(state.alreadyDeclaredNodes.begin(), state.alreadyDeclaredNodes.end(), title) == state.alreadyDeclaredNodes.end()) {
                intendedOut(quoteString(title) + formatAttribute(state.attributes).c_str() + "\n");
                state.alreadyDeclaredNodes.emplace_back(title);
        }

        state.attributes.clear();

        state.flows.back().emplace_back(FlowItem { title, state.edgeAttributesForNextNode });
        state.edgeAttributesForNextNode.clear();
}

void onEdge()
{
        state.edgeAttributesForNextNode = state.attributes;
        state.attributes.clear();
}

void onNodeFlowEnd()
{
        state.lastFlowStart = state.currentFlowStart;
        state.lastFlowEnd = state.currentFlowEnd;

        state.currentFlowStart = state.currentFlowEnd = "";

        state.flows.emplace_back(std::vector<FlowItem>());
}


void onAttribute(std::string key, std::string value)
{
        state.attributes.insert(std::pair<std::string, std::string>(key, value));
}

void onTemplateUse(std::string name)
{
        auto source = state.templates.find(name);
        if(source != state.templates.end())
                state.attributes.insert(source->second.begin(), source->second.end());
        else
                fprintf(stderr, "Warning: Template '%s' does not exist, ignoring is-attribute.\n", name.c_str());
}

void onClusterStart(std::string title)
{
        out("\n");
        intendedOut("subgraph cluster_" + std::to_string(state.subgraphCounter++) + " {\n");
        state.intendLevel++;

        state.attributes.insert(std::pair<std::string, std::string>("label", title));
        intendedOut(formatAttribute(state.attributes, ';', false) + "\n\n");
        state.attributes.clear();
}

void onClusterEnd()
{
        state.intendLevel--;
        intendedOut("}\n");
}

void onEnd()
{
        out("\n");
        for(auto &flow : state.flows) {
                if(flow.size() > 1) {
                        intendedOut("");
                        for(auto it = flow.begin(); !flow.empty() && it != flow.end(); ++it) {
                                // if we have attributes and already printed more than one node, repeat the last one
                                if(!it->edgeAttributes.empty() && std::distance(flow.begin(), it) > 1 && (it - 1)->edgeAttributes.empty())
                                        out(" ; " + quoteString((it - 1)->title));

                                // print the arrow, if needed
                                if(it != flow.begin())
                                        out(" -> ");

                                out(quoteString(it->title));

                                // if this item has attributes, output them... 
                                if(!it->edgeAttributes.empty()) {
                                        out(formatAttribute(it->edgeAttributes));

                                        // ...and print this item's a second time, if it not the last item
                                        if(it + 1 != flow.end())
                                                out("; " + quoteString(it->title));
                                }
                        }
                        out("\n");
                }
        }
        out("}\n");
}
