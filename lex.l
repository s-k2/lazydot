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

%{
#include "parser.tab.h"
#include <stdlib.h>

extern "C" int yylex();
%}

%option nounput

%%

"graph"        { return(KEYWORD_GRAPH); }
"template"     { return(KEYWORD_TEMPLATE); }

"cluster"      { return(KEYWORD_CLUSTER); }
"{"            { return(CLUSTER_START); }
"}"            { return(CLUSTER_END); }

"->"           { return(ARROW); }

"["            { return(ATTRIBUTES_START); }
"="            { return(ATTRIBUTES_ASSIGNMENT); }
"]"            { return(ATTRIBUTES_END); }

\"(\\.|[^"\n])*\"               { yylval.sval = strdup(yytext + 1); yylval.sval[strlen(yylval.sval) - 1] = 0x00; return(IDENTIFIER); }
\"(\\.|[^"\n])*\n               { return(UNTERMINATED_STRING); }
[^# \n\r\t\v=][^ \n\r\t\v=]*	{ yylval.sval = strdup(yytext); return(IDENTIFIER); }

#[^\n]+        // ignore comments
[ \n\r\t\v]+   // ignore whitespaces

