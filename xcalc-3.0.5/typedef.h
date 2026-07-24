/*
		XCALC simple RPN Calculator
	Copyright (C) 2011  Bernt Ribbum

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <cstdlib>
#include <complex>
#include <QMap>
#include <QKeySequence>

#if defined __linux || __BORLANDC__
#define __HAVELD__
#endif

#ifdef __HAVELD__
typedef long double LD; // for convenience
#else
typedef double LD;
#endif

typedef std::complex<LD> LC;
typedef QList<int> intlist;
typedef wchar_t wchar; // hate the _t suffix!

//--Fundamental enum types----------------------------------------------------------------

// radix
enum RadixType {
	rtDECIMAL,
	rtCOMPLEX,
	rtHEX,
	rtOCTAL,
	rtBINARY,
};

// binary word length
enum WordLength {
	wl8BIT,
	wl16BIT,
	wl32BIT,
	wl64BIT,
};

const int NWL = 4; //this won't work! sizeof(WordLength)/sizeof(wl8BIT);

// fix
enum FixType {
	ftFIX,
	ftSCI,
	ftENG,
};

// angle
enum AngType {
	atDEGREE,
	atRADIAN,
};

//--Engine functions (also for undo)------------------------------------------------------

enum funid {
	// total
	// local to input window, not undoable
	FUN_NONE, /* 0 is a good synonym */
	FUN_ABOUT,
	FUN_0, FUN_1, FUN_2, FUN_3, FUN_4, FUN_5, FUN_6, FUN_7, FUN_8, FUN_9,
	FUN_A, FUN_B, FUN_C, FUN_D, FUN_E, FUN_F, FUN_POINT, FUN_EEX,
	FUN_BACK, FUN_IMAGPART, FUN_COPY, FUN_PASTE,
	FUN_CONFIG, FUN_EXIT, FUN_HELP, FUN_KEYHELP, FUN_POPUP,
	// Program keys
	FUN_PROG1, FUN_PROG2, FUN_PROG3, FUN_PROG4, FUN_PROG5, FUN_PROG6,
	FUN_PROG7, FUN_PROG8, FUN_PROG9, FUN_PROG10, FUN_PROG11, FUN_PROG12,
	FUN_EDITPR,
	// synonyms (to allow several shortcuts for a function - fixed in translatekid)
	FUN_ROUND2,
	FUN_NOT2,
	FUN_OR2,
	// undoable from here
	FUN_FIX, FUN_SCI, FUN_ENG, FUN_FLOAT, FUN_CLEANFRAC,
	FUN_FIX0, FUN_FIX1, FUN_FIX2, FUN_FIX3, FUN_FIX4, FUN_FIX5, FUN_FIX6, FUN_FIX7, FUN_FIX8, FUN_FIX9,
	FUN_FIXP, FUN_FIXM,
	FUN_DEG, FUN_RAD, FUN_DECIMAL, FUN_COMPLEX, FUN_HEX, FUN_OCTAL, FUN_BINARY,
	FUN_INKEY, FUN_CHS,
	FUN_CLX, FUN_ENTER, FUN_ADD, FUN_SUB, FUN_MUL, FUN_DIV, FUN_DIVF, FUN_MOD, FUN_MODF, FUN_LASTX, FUN_UNDO, FUN_REDO,
	FUN_CLSTK, FUN_TOIJ, FUN_FRAC, FUN_DMS, FUN_TODMS, FUN_ROUND, FUN_XY, FUN_RUP, FUN_RDN, FUN_PERC,
	FUN_CONV, FUN_QCONV, FUN_CONS, FUN_QCONS, FUN_RAND, FUN_TOFRAC, FUN_TOINT, FUN_CART, FUN_POLAR, FUN_EXP,
	FUN_TEN, FUN_ROOT, FUN_SQRT, FUN_QROOT, FUN_LN, FUN_LOG, FUN_POW, FUN_SQR, FUN_CUBE, FUN_ABS,
	FUN_SIN, FUN_COS, FUN_TAN, FUN_ASIN, FUN_ACOS, FUN_ATAN,
	FUN_SINH, FUN_COSH, FUN_TANH, FUN_ARSINH, FUN_ARCOSH, FUN_ARTANH,
	FUN_RCP, FUN_PI, FUN_FACT,
	FUN_TOYX, FUN_REIM, FUN_CONJ,
	FUN_AND, FUN_OR, FUN_XOR, FUN_XOR2, FUN_NOT, FUN_SHL, FUN_SHR, FUN_ASHL, FUN_ASHR, FUN_ROTL, FUN_ROTR,
	FUN_STO0, FUN_STO1, FUN_STO2, FUN_STO3, FUN_STO4, FUN_STO5, FUN_STO6, FUN_STO7, FUN_STO8, FUN_STO9,
	FUN_RCL0, FUN_RCL1, FUN_RCL2, FUN_RCL3, FUN_RCL4, FUN_RCL5, FUN_RCL6, FUN_RCL7, FUN_RCL8, FUN_RCL9,
	FUN_CLR0, FUN_CLR1, FUN_CLR2, FUN_CLR3, FUN_CLR4, FUN_CLR5, FUN_CLR6, FUN_CLR7, FUN_CLR8, FUN_CLR9,
	FUN_64BIT, FUN_32BIT, FUN_16BIT, FUN_8BIT,
	FUN_CLRMEM,
	FUN_WLUP,
	FUN_WLDOWN,
	FUN_LAST // last entry works only as counter
};

typedef QMap<funid,QString> idtextmap; // map from keyid to text (caption, tooltip, function name)
typedef QMap<funid,QKeySequence> idseqmap; // map from funid to keysequence (for defining shortcuts)
typedef QList<funid> idList; // one list for buttons per mode and one for all legal actions (so far, buttons for mode, but should have one more with all allowed actions too!)

//--Error constants------------------------------------------------------

enum messagecode {
	__MESSAGE,		// any message: look up qmessage()
	__NOUNDO,		// no more undo information
	__NOREDO,		// no more redo information
	__UNDONE,		// undone
	__REDONE,		// redone
};

#endif // TYPEDEF_H
