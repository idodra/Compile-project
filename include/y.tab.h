/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTV = 258,
     FLOATV = 259,
     EQ = 260,
     ADD = 261,
     SUB = 262,
     MUL = 263,
     FDIV = 264,
     IDIV = 265,
     MOD = 266,
     ID = 267,
     LRB = 268,
     RRB = 269,
     LAB = 270,
     RAB = 271,
     LSB = 272,
     RSB = 273,
     COM = 274,
     SEM = 275
   };
#endif
/* Tokens.  */
#define INTV 258
#define FLOATV 259
#define EQ 260
#define ADD 261
#define SUB 262
#define MUL 263
#define FDIV 264
#define IDIV 265
#define MOD 266
#define ID 267
#define LRB 268
#define RRB 269
#define LAB 270
#define RAB 271
#define LSB 272
#define RSB 273
#define COM 274
#define SEM 275




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 12 "example1.y"
{
	int Ival;
	float Fval;
	char* Sval;
	struct TreeNode* TNval;
}
/* Line 1529 of yacc.c.  */
#line 96 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

