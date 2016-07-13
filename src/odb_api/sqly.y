%pure-parser
%lex-param {yyscan_t scanner}
%lex-param {odb::sql::SQLSession* session}
%parse-param {yyscan_t scanner}
%parse-param {odb::sql::SQLSession* session}

%{

/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#define YYMAXDEPTH 500

#include <unistd.h>

#include "eckit/eckit.h"
#include "odb_api/SQLSession.h"
#include "odb_api/SQLStatement.h"

#include <iostream>

//#include "odb_api/odblib_lex.h"

#ifdef YYBISON
//#define YYSTYPE_IS_DECLARED
//int yylex(); 
//int yylex(YYSTYPE*, void*);
//int yylex ( YYSTYPE * lvalp, YYLTYPE * llocp, yyscan_t scanner );

/// # define YYLEX odblib_lex (&odblib_lval, scanner, session)

//#define YY_DECL int odblib_lex (YYSTYPE *, yyscan_t, odb::sql::SQLSession*);

//int yylex ( void * lvalp, void * llocp, void* scanner );
//int yydebug;
extern "C" int isatty(int);
#endif

Expressions emptyExpressionList;

%}

%token <val>STRING
%token <val>EMBEDDED_CODE
%token <val>IDENT
%token <val>VAR

%token <num>DOUBLE

%token SEMICOLON
%token LINK
%token TYPEOF
%token READONLY
%token UPDATED
%token NOREORDER
%token SAFEGUARD

%token EXIT

%token SELECT
%token INTO
%token FROM
%token SET
%token DATABASE
%token COUNT
%token WHERE
%token GROUP
%token ORDER
%token BY

%token CREATE
%token SCHEMA
%token VIEW
%token INDEX
%token TABLE
%token TYPE
%token TYPEDEF
%token TEMPORARY
%token INHERITS
%token DEFAULT

%token CONSTRAINT
%token UNIQUE
%token PRIMARY
%token FOREIGN
%token KEY
%token REFERENCES

%token EQ
%token GE
%token LE
%token NE
%token IN
%token NOT
%token AND
%token OR
%token ON
%token IS
%token AS
%token NIL
%token ALL
%token DISTINCT
%token BETWEEN

%token ASC
%token DESC

%token HASH
%token LIKE
%token RLIKE
%token MATCH
%token QUERY

%type <exp>column;
%type <exp>vector_index;

%type <exp>condition;
%type <exp>atom_or_number;
%type <exp>expression;

%type <exp>factor;
%type <exp>power;
%type <exp>term;
%type <exp>conjonction;
%type <exp>disjonction;
%type <exp>optional_hash;

%type <exp>where;

%type <exp>assignment_rhs;
%type <explist>expression_list;

%type <tablist>table_list;
%type <table>table
%type <val>table_name;
%type <val>table_reference;
%type <tablist>from;

%type <explist>group_by;

%type <orderlist>order_by;
%type <orderlist>order_list;
%type <orderexp>order;

%type <explist>select_list;
%type <explist>select_list_;
%type <exp>select;
%type <exp>select_;

%type <bol>distinct;
%type <bol>all;
%type <val>into;
%type <val>func;
%type <val>relational_operator;

%type <r>vector_range_decl;
%type <val>column_name;
%type <val>bitfield_ref;
%type <coldefs>column_def_list;
%type <coldefs>column_def_list_;
%type <coldef>column_def;
%type <coldefs>bitfield_def_list;
%type <coldefs>bitfield_def_list_;
%type <coldef>bitfield_def;
%type <val>data_type;
%type <val>default_value;
%type <bol>temporary;
%type <val>location;
%type <list>inherits;
%type <list>inheritance_list;
%type <list>inheritance_list_;

%type <condefs>constraint_list;
%type <condefs>constraint_list_;
%type <condef>constraint;
%type <val>constraint_name;
%type <condef>primary_key;
%type <condef>foreign_key;
%type <list>column_reference_list;
%type <val>column_reference;
%type <select_statement>select_statement;
%type <select_statement>create_view_statement;

%%

start : statements { session->currentDatabase().setLinks(); }
	;

statements : statement ';'
		   | statements statement ';'
		   ;

statement: select_statement
           {
               SelectAST a($1);
               session->statement(session->selectFactory().create(*session, a));
           }
		 | set_statement
		 | create_schema_statement
		 | create_view_statement
           {
               SelectAST a($1);
               session->statement(session->selectFactory().create(*session, a));
           }
		 | create_index_statement 
		 | create_type_statement
		 | create_table_statement
		 | readonly_statement
		 | updated_statement
		 | noreorder_statement
		 | safeguard_statement
		 | exit_statement
		 | error
		 | empty
	;

exit_statement: EXIT { return 0; }
	;

readonly_statement: READONLY
	;

updated_statement: UPDATED
	;

noreorder_statement: NOREORDER
	;

safeguard_statement: SAFEGUARD
	;

create_schema_statement: CREATE SCHEMA schema_name schema_element_list
        {
            session->currentDatabase().schemaAnalyzer().endSchema();
        }
        ;

schema_name: IDENT
        {
           session->currentDatabase().schemaAnalyzer().beginSchema($1);
        }
       ;

schema_element_list: schema_element
                   | schema_element_list schema_element
                   ;

schema_element: create_table_statement
              | create_view_statement
              | empty
              ;

create_index_statement: CREATE INDEX IDENT TABLE IDENT
	{
        session->createIndex($3,$5);
	}
	| CREATE INDEX IDENT '@' IDENT
	{
        session->createIndex($3,$5);
	}
	;

create_type_statement: create_type IDENT as_or_eq '(' bitfield_def_list ')'
	{
        std::string typeName ($2);
        ColumnDefs	colDefs ($5);
        FieldNames	fields;
        Sizes		sizes;
        for (ColumnDefs::size_type i (0); i < colDefs.size(); ++i)
        {
            std::string name (colDefs[i].name());
            std::string memberType (colDefs[i].type());

            fields.push_back(name);

            int size (::atof(memberType.c_str() + 3)); // bit[0-9]+
			ASSERT(size > 0);

            sizes.push_back(size);
        }
        std::string typeSignature (type::SQLBitfield::make("Bitfield", fields, sizes, typeName.c_str()));

        session->currentDatabase()
        .schemaAnalyzer().addBitfieldType(typeName, fields, sizes, typeSignature);
		
		//cout << "CREATE TYPE " << typeName << " AS " << typeSignature << ";" << std::endl;		
	}
	;

create_type_statement: create_type IDENT as_or_eq IDENT { type::SQLType::createAlias($4, $2); }
	;

create_type: CREATE TYPE
           | TYPEDEF
           ;

as_or_eq: AS
        | EQ
        ;

bitfield_def_list: bitfield_def_list_ { $$ = $1; }
                 | bitfield_def_list_ ',' { $$ = $1; }
	;

bitfield_def_list_: bitfield_def { $$ = ColumnDefs(1, $1); }
                  | bitfield_def_list_ ',' bitfield_def { $$ = $1; $$.push_back($3); }
	;

bitfield_def: column_def
	{
		/* Type should look like bit[0-9]+ '*/
		$$ = $1;
	}
	;

temporary: TEMPORARY  { $$ = true; }
         | empty      { $$ = false; }
         ;

inheritance_list: inheritance_list_       { $$ = $1; }
                 | inheritance_list_ ','  { $$ = $1; }
                 ;

inheritance_list_: IDENT                         { $$ = std::vector<std::string>(); $$.insert($$.begin(), $1); }
                 | inheritance_list_ ',' IDENT   { $$ = $1; $$.push_back($3); }

inherits: INHERITS '(' inheritance_list ')'   { $$ = $3;               }
        | empty                               { $$ = std::vector<std::string>(); }
        ;

constraint_list: constraint_list_ { $$ = $1; }
               | constraint_list_ ',' { $$ = $1; }
               ;

constraint_list_: constraint { $$ = ConstraintDefs(1, $1); }
                | constraint_list_ ',' constraint { $$ = $1; $$.push_back($3); }
                | empty { $$ = ConstraintDefs(0); }
                ;

constraint: primary_key { $$ = $1; }
          | foreign_key { $$ = $1; }
          ;

primary_key: constraint_name UNIQUE '(' column_reference_list ')' { $$ = ConstraintDef($1, $4); }
           | constraint_name PRIMARY KEY '(' column_reference_list ')' { $$ = ConstraintDef($1, $5); }
           ;

foreign_key: constraint_name FOREIGN KEY '(' column_reference_list ')' REFERENCES IDENT '(' column_reference_list ')'
           {
                $$ = ConstraintDef($1, $5, $8, $10);
           }
           ;

constraint_name: CONSTRAINT IDENT { $$ = $2; }
               | empty { $$ = std::string(); }
               ;

column_reference_list: column_reference { $$ = std::vector<std::string>(1, $1); }
                | column_reference_list ',' column_reference { $$ = $1; $$.push_back($3); }
                ;

column_reference: IDENT table_reference { $$ = $1 + $2; }
           ;

location: ON STRING { $$ = $2; }
        | empty     { $$ = ""; }

create_table_statement: CREATE temporary TABLE table_name AS '(' column_def_list constraint_list ')' inherits location
	{
        bool temporary ($2);
		std::string name ($4);

        ColumnDefs cols ($7);
        ConstraintDefs contraints ($8);
		std::vector<std::string> inheritance($10);
        std::string location($11);
        TableDef tableDef(name, cols, /*constraints*/ $8, inheritance, location);

        session->currentDatabase().schemaAnalyzer().addTable(tableDef);
	}
	;

table_name: IDENT { $$ = $1; }
          | IDENT '.' IDENT { $$ = $1 + std::string(".") + $3; }
          | expression { SQLExpression* e($1); $$ = e->title(); }
          ;

column_def_list: column_def_list_     { $$ = $1; }
               | column_def_list_ ',' { $$ = $1; }
               | empty                { $$ = ColumnDefs(); }
               ;
	 
column_def_list_: column_def                      { $$ = ColumnDefs(1, $1); }
                | column_def_list_ ',' column_def { $$ = $1; $$.push_back($3); }
                ;

column_def: column_name vector_range_decl data_type default_value
	{
		$$ = ColumnDef($1, $3, $2, $4);
	}
	;

vector_range_decl: '[' DOUBLE ']'            { $$ = std::make_pair(1, $2); }
                 | '[' DOUBLE ':' DOUBLE ']' { $$ = std::make_pair($2, $4); }
                 | empty                     { $$ = std::make_pair(0, 0); }
                 ;

column_name: IDENT { $$ = $1; }
           ;

data_type: IDENT                 { $$ = $1; }
         | LINK                  { $$ = "@LINK"; } 
         | TYPEOF '(' column ')' { $$ = ($3)->type()->name(); }
         ;

default_value: DEFAULT expression { SQLExpression* e($2); $$ = e->title(); }
             | empty { $$ = std::string(); }
             ;

create_view_statement: CREATE VIEW IDENT AS select_statement { $$ = $5; }
	;

select_statement: SELECT distinct all select_list into from where group_by order_by
                {   
                    bool                                      distinct($2);
                    bool                                      all($3);
                    Expressions                               select_list($4);
                    std::string                               into($5);
                    std::vector<Table>                        from ($6);
                    SQLExpression*                            where($7);
                    Expressions                               group_by($8);
                    std::pair<Expressions,std::vector<bool> > order_by($9);

                    $$ = SelectAST(distinct, all, select_list, into, from, where, group_by, order_by);
                }
                ;

distinct: DISTINCT { $$ = true; }
		| empty    { $$ = false; }
		;

all: ALL      { $$ = true; }
   | empty    { $$ = false; }
   ;


into: INTO IDENT   { $$ = $2; }
    | INTO STRING  { $$ = $2; }
    | empty        { $$ = ""; }
    ;

from : FROM table_list    { $$ = $2; }
     | FROM EMBEDDED_CODE 
        { 
            std::cerr << "FROM EMBEDDED_CODE: " << $2 << std::endl;
            $$ = std::vector<Table>();
            $$.push_back(Table($2, "", true));
        } 
     | empty              { $$ = std::vector<Table>(); }
	 ;

where : WHERE expression { $$ = $2; }
	  |                  { $$ = 0;  } 
	  ;

assignment_rhs	: expression
		;

set_statement : SET DATABASE STRING { session->openDatabase($3); }
	; 

set_statement : SET DATABASE STRING AS IDENT { session->openDatabase($3,$5); }
	; 

set_statement : SET VAR EQ assignment_rhs
	{ 
        //using namespace std;
		//cout << "== set variable " << $2 << " to "; if ($4) cout << *($4) << std::endl; else cout << "NULL" << std::endl;
		session->currentDatabase().setVariable($2, $4);
	}
	; 

bitfield_ref: '.' IDENT  { $$ = $2; }
                        |            { $$ = std::string(); }

column: IDENT vector_index table_reference optional_hash
		  {

			std::string columnName($1);
			SQLExpression* vectorIndex($2);
			Table table($3, "", false); // TODO: table_reference should handle .<database> suffix
			SQLExpression* pshift($4);

            bool missing;
			std::string bitfieldName;
			$$ = session->selectFactory().createColumn(columnName, bitfieldName, vectorIndex, table /*TODO: handle .<database> */, pshift);
		  }
	   | IDENT bitfield_ref table_reference optional_hash
		{

			std::string columnName      ($1);
			std::string bitfieldName    ($2);
			SQLExpression* vectorIndex  (0); 
			Table table($3, "", false); // TODO: table_reference should handle .<database> suffix
			SQLExpression* pshift       ($4);

            bool missing;
			$$ = session->selectFactory().createColumn(columnName, bitfieldName, vectorIndex, table /*TODO: handle .<database> */, pshift);
		 }
	  ;

vector_index : '[' expression ']'    { $$ = $2; }
             | empty                 { $$ = NULL; }
             ;

table_reference: '@' IDENT   { $$ = std::string("@") + $2; }
               | empty       { $$ = std::string(""); }
               ;

table : IDENT '.' IDENT { $$ = Table(std::string($1), std::string($3), false); } 
      | IDENT           { $$ = Table(std::string($1), std::string(""), false); } 
      | STRING			{ $$ = Table(std::string($1), std::string(""), false); } 
      ;

table_list : table                  { $$ = std::vector<Table>(); $$.push_back($1); }
	       | table_list  ',' table  { $$ = $1; $$.push_back($3); }
	       ;

/*================= SELECT =========================================*/

select_list: select_list_     { $$ = $1; }
           | select_list_ ',' { $$ = $1; }
           ;

select_list_ : select         { $$ = Expressions(1, $1); }
	| select_list_ ',' select { $$ = $1; $$.push_back($3); }
	;

select: select_ access_decl { $$ = $1; }
      ;

select_: '*' table_reference                             { $$ = new ColumnExpression("*", $2);  } 
	   | IDENT '.' '*' table_reference                   { $$ = new BitColumnExpression($1, "*", $4); }
	   | IDENT '[' expression ':' expression ']' table
		{
			// TODO: Add simillar rule for BitColumnExpression.
			bool missing (false);
			int begin ($3->eval(missing)); //ASSERT(!missing);
			int end ($5->eval(missing)); //ASSERT(!missing);
            Table table ($7);
			$$ = new ColumnExpression($1, table.name /*TODO: handle .<database> */, begin, end);
		}
	   | expression AS IDENT table_reference { $$ = $1; $$->title($3 + $4); }
	   | expression
	   ;

access_decl: UPDATED
           | READONLY
           | empty
           ;

/*================= GROUP BY ======================================*/
group_by: GROUP BY expression_list { $$ = $3;                       }
        | empty                    { $$ = Expressions(); }
	    ;

/*================= ORDER =========================================*/

order_by: ORDER BY order_list { $$ = $3;                       }
        | empty               { $$ = std::make_pair(Expressions(),std::vector<bool>()); }
	    ;


order_list : order                  { $$ = std::make_pair(Expressions(1, $1 .first),std::vector<bool>(1, $1 .second)); }
           | order_list ',' order   { $$ = $1; $$.first.push_back($3 .first); $$.second.push_back($3 .second); }
		   ;

order : expression DESC      { $$ = std::make_pair($1, false); }
      | expression ASC       { $$ = std::make_pair($1, true); }
      | expression			 { $$ = std::make_pair($1, true); }
	  ;


/*================= EXPRESSION =========================================*/

expression_list : expression         {  $$ = Expressions(1, $1); }
				| expression_list ',' expression { $$ = $1; $$.push_back($3); }
				;

optional_hash : HASH DOUBLE { $$ = new NumberExpression($2); }
			  |             { $$ = new NumberExpression(0); }
			  ;


atom_or_number : '(' expression ')'           { $$ = $2; }
			   | '-' expression               { $$ = ast("-",$2); }
			   | DOUBLE                       { $$ = new NumberExpression($1); }
			   | column                   
			   | VAR                          { $$ = session->currentDatabase().getVariable($1); } 
			   | '?' DOUBLE                   { $$ = new ParameterExpression($2); }
			   | func '(' expression_list ')' { $$ = ast($1, $3); }
			   | func '(' empty ')'           { $$ = ast($1, emptyExpressionList); }
			   | func '(' '*' ')'             
				{
                    if (std::string("count") != $1)
                        throw eckit::UserError(std::string("Only function COUNT can accept '*' as parameter (") + $1 + ")");

                    $$ = ast("count", new NumberExpression(1.0));
				}
			   | STRING                       { $$ = new StringExpression($1); }
			   | EMBEDDED_CODE                { $$ = new EmbeddedCodeExpression($1); }
			   ;


func : IDENT { $$ = $1;      }
	 | COUNT { $$ = "count"; }
	 ;

/* note: a^b^c -> a^(b^c) as in fortran */

power       : atom_or_number
			;

factor      : factor '*' power          { $$ = ast("*",$1,$3);   }
            | factor '/' power          { $$ = ast("/",$1,$3); }
            /* | factor '%' power          { $$ = new CondMOD($1,$3); } */
            | power
            ;

term        : term '+' factor           { $$ = ast("+",$1,$3);   }
            | term '-' factor           { $$ = ast("-",$1,$3);   }
            /* | term '&' factor */
            | factor
            ;

relational_operator: '>' { $$ = ">"; }
                   | EQ  { $$ = "="; }
                   | '<' { $$ = "<"; }
                   | GE  { $$ = ">="; }
                   | LE  { $$ = "<="; }
                   | NE  { $$ = "<>"; }
                   ;

condition   : term relational_operator term relational_operator term { $$ = ast("and", ast($2,$1,$3), ast($4,$3,$5)); }
            | term relational_operator term                          { $$ = ast($2, $1, $3); }
            | MATCH '(' expression_list ')' IN QUERY '(' select_statement ')'
            { 
                const Expressions& matchList ($3);
                const SelectAST& subquery ($8);
                $$ = ast("match", matchList, subquery);
            }
            | condition  IN '(' expression_list ')'                  { $4.push_back($1); $$ = ast("in",$4);   }
            | condition  IN VAR         
			{ 
                SQLExpression* v = session->currentDatabase().getVariable($3);
                ASSERT(v && v->isVector());
                Expressions e(v->vector());
                e.push_back($1);
                $$ = ast("in", e);
			}
            | condition  NOT IN '(' expression_list ')'  { $5.push_back($1); $$ = ast("not_in",$5);   }
            | condition  NOT IN VAR  
			{ 
                // This has not been implemented yet.
                throw UserError("Syntax: 'condition NOT IN VAR' not yet supported");

				SQLExpression* v = session->currentDatabase().getVariable($4);
				ASSERT(v && v->isVector());
				Expressions e(v->vector());
				e.push_back($1);
				$$ = ast("not_in", e);
			}

            | NOT condition             { $$ = ast("not",$2);   }
			| condition IS NIL          { $$ = ast("null",$1);   }
			| condition IS NOT NIL      { $$ = ast("not_null",$1);   }
			| condition BETWEEN term AND term { $$ = ast("between",$1,$3,$5); }
			| condition NOT BETWEEN term AND term { $$ = ast("not_between",$1,$4,$6); }
            | condition LIKE term       { $$ = ast("like", $1, $3); }
            | condition RLIKE term      { $$ = ast("rlike", $1, $3); }
            | term
            ;

conjonction : conjonction AND condition       { $$ = ast("and",$1,$3);   }
            | condition
            ;

disjonction : disjonction OR conjonction      { $$ = ast("or",$1,$3);   }
            | conjonction
            ;

expression  : disjonction
			| expression '[' expression ']' 
			{
                // This has not been implemented yet.
                throw UserError("Syntax: 'expression [ expression ]' not yet supported");

				SQLExpression* container = $1;
				SQLExpression* index = $3;

				bool missing = false;
				long i = index->eval(missing);
				ASSERT(! missing);
				if (container->isVector())
				{
					//cerr << "==== index: '" << i << "'" << std::endl;
					$$ = container->vector()[i];
				}
				else
				if (container->isDictionary())
				{
                    std::string key (index->title());
                    if (container->dictionary().find(key) == container->dictionary().end())
                    {
                        std::stringstream ss;
                        ss << "Key '" << key << "' not found.";
                        throw eckit::UserError(ss.str());
                    }
					
                    $$ = container->dictionary()[key];
				}
			}
            ;

empty :
      ;	

%%
#include "sqll.c"

