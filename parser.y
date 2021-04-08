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
#include <stdio.h>
#include <string.h>

#include "filter.h"

static int yyerror(const char *s);
extern "C" int yylex();
extern FILE* yyin;
extern FILE* yyout;

%}
%define parse.error verbose

%union
{
	char *sval;
};

%token KEYWORD_GRAPH
%token KEYWORD_TEMPLATE

%token KEYWORD_CLUSTER
%token CLUSTER_START
%token CLUSTER_END

%token ARROW

%token <sval> IDENTIFIER
%token UNTERMINATED_STRING

%token ATTRIBUTES_START
%token ATTRIBUTES_ASSIGNMENT
%token ATTRIBUTES_END

%start File
%%

File: Statements 
    | Graph Statements ;
Statements: Statement Statements | Statement ;
Statement: Cluster | NodeFlow | Template ;

Graph: KEYWORD_GRAPH AttributesList                  { onGraph(); };

Template: KEYWORD_TEMPLATE IDENTIFIER AttributesList { onTemplateDefinition($2); free($2); } ;

Cluster:  ClusterHeader Statements ClusterFooter ;
ClusterHeader: KEYWORD_CLUSTER IDENTIFIER CLUSTER_START                { onClusterStart($2); free($2); }
             | KEYWORD_CLUSTER IDENTIFIER AttributesList CLUSTER_START { onClusterStart($2); free($2); }
             ;
ClusterFooter: CLUSTER_END   { onClusterEnd(); } ;

NodeFlow: Node Edge NodeFlow
        | Node               { onNodeFlowEnd(); }
        ;
Node: IDENTIFIER                  { onNode($1); free($1); }
    | IDENTIFIER AttributesList   { onNode($1); free($1); }
    ;

Edge: ARROW                  { onEdge(); }
    | ARROW AttributesList   { onEdge(); }
    ;

AttributesList: ATTRIBUTES_START Attributes ATTRIBUTES_END ;
Attributes: Attribute
          | Attribute Attributes
          ;
Attribute: IDENTIFIER ATTRIBUTES_ASSIGNMENT IDENTIFIER { onAttribute($1, $3); free($1); free($3); }
         | ATTRIBUTES_ASSIGNMENT IDENTIFIER            { onTemplateUse($2); free($2); }

%%

int yyerror(const char *s)
{
        fprintf(stderr, "%s\n", s);
        exit(1);
}

void usage()
{
        fprintf(stderr,
                "Usage: lazydot [OPTION] [FILE]\n"
                "\n"
                "Convert graphs written in the simplified lazydot minilanguage into\n"
                "Graphviz's DOT language.\n"
                "\n"
                "With no FILE, or when FILE is -, read standard input.\n"
                "\n"
                "The only option:\n"
                "  -o <file>                Place the output into <file>.\n"
                "\n"
                "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
                "\n"
                "This is free software; you are free to change and redistribute it.\n"
                "There is NO WARRANTY, to the extent permitted by law.\n");
}

int main(int argc, char *argv[])
{
        yyout = stdout;

        for(int i = 1; i < argc; i++) {
                if(argv[i][0] != '-') {
                        if((yyin = fopen(argv[i], "r")) == NULL) {
                                fprintf(stderr, "%s: cannot open file for read\n", argv[i]);
                                return(1);
                        }
                } else if(!strcmp(argv[i], "-")) {
                        yyin = stdin;
                } else if(!strcmp(argv[i], "-o") && argc > i + 1) {
                        i++;
                        if((yyout = fopen(argv[i], "w")) == NULL) {
                                fprintf(stderr, "%s: cannot open file for write\n", argv[i]);
                                return(1);
                        }
                } else {
                        usage();
                        return(1);
                }
        }

        onStart();
        yyparse();
        onEnd();

        return(0);
}
