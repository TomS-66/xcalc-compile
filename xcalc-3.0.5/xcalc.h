/*
		XCALC simple RPN Calculator
		Copyright (C) 1992-2012 Bernt Ribbum

		This program is free software: you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation, either version 3 of the License, or
		(at your option) any later version.

		This program is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

//-----------------------------------------------------------------------
//
// XCALC calculator internals.
// Bernt Ribbum
//
//-----------------------------------------------------------------------

#ifndef XCALC_H
#define XCALC_H

//--All include files----------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <QString>
#include <QMap>
#include "register.h"
#include "profile.h"
#include <iostream>
#include <QtGui>

#define UNUSED(x) (void)(x)

#if defined __linux__
// no extras
#elif defined __WIN32__
#include <windows.h>
#define M_PIl M_PI
#endif

//using namespace std;
extern QTextStream qcout;

//--Global constants for everything--------------------------------------

//const int ID_BLINKTIMER		=	100;
//const int BLINKMS			=	500;

//const int TOOLTIP_SHOWTIMER =	101;
//const int TOOLTIP_SHOWMS	=	500;
//const int TOOLTIP_FOLLOWSHOWMS =	10;
//const int TOOLTIP_HIDETIMER =	102;
//const int TOOLTIP_HIDEMS	=	30000;
//const int TOOLTIP_FOLLOWTIMER =	103;
//const int TOOLTIP_FOLLOWMS =	200;

const int MAXUNDO 		=	32;

const int MEMS=10;
const int STKS=4;

const int FLOATDISP = -1;

const int iLReg = -1;
const int iXreg = 0;
const int iYreg = 1;
const int iZreg = 2;
const int iTreg = STKS-1;

const int DECMAXDIGITS   = LDBL_DIG;			// for input
const int CPLXMAXDIGITS  = DBL_DIG;
const int DMSMAXINT      = 65535;				// max integer for DMS display
const int DMSMAXFIX      = 10;					// max decimal digits to display
const int HEXMAXDIGITS8  = 2;
const int OCTMAXDIGITS8  = 3;
const int BINMAXDIGITS8  = 8;
const int HEXMAXDIGITS16 = 4;
const int OCTMAXDIGITS16 = 6;
const int BINMAXDIGITS16 = 16;
const int HEXMAXDIGITS32 = 8;
const int OCTMAXDIGITS32 = 11;
const int BINMAXDIGITS32 = 32;
const int HEXMAXDIGITS64 = 16;
const int OCTMAXDIGITS64 = 22;
const int BINMAXDIGITS64 = 64;

#ifdef __HAVELD__ // If long double != double
const int FACT_MAX       = 449;			 		// max input x for x!
#else
const int FACT_MAX       = 166;			 		// max input x for x!
#endif

#ifdef __HAVELD__
const LD NUM_MAX         = 9.499999999999999e999L;	// odd value to avoid 1.e+1000
const LD NUM_MIN         = 1e-999L;					// Avoid smaller numbers on display
const LD MANT_MAX        = 9.499999999999999L;		// MANT_MAX E EXP_MAX == NUM_MAX
const int EXP_MAX        = 999;
#else
const LD NUM_MAX         = 9.999999999999999e300L;	// We make Micro$oft happy
const LD NUM_MIN         = 1e-300L;					// since they do not
const LD MANT_MAX        = 9.999999999999999L;		// support long double...
const int EXP_MAX        = 300;
#endif

const int DECOUTWIDTH    = 22;						// for output, before possible 1000s separator
const int CPLXOUTWIDTH   = 16;						// for complex display output

//--For unit conversions------------------------------------------------

const int NFIXEDCONVERSIONS = 5;
const int NUSERCONVERSIONS  = 5;
const int NCONVERSIONS      = (NFIXEDCONVERSIONS+NUSERCONVERSIONS); // Number of conversion types (added 5 user types Feb02)

//--Undo structure-------------------------------------------------------
// Note: use this structure also for register save!

struct UndoInfoStruct {
	funid		undone;
	Register	mem[MEMS];
	Register	st[STKS];
	Register	lastx;
	bool		sl;
	WordLength	wl;
	RadixType	radix;
	FixType		fixtype;
	int			fix;
	uint		DecDEG;
	bool		cleanFrac;
	//short		ConvType;
	//short		From;
	//short		To;
	//short 	SelectFrom[NCONVERSIONS];
	//short 	SelectTo[NCONVERSIONS];
	//LD		ldConstant;
};

template <class T> void SWAP(T &a, T &b)
{
	T t = a;
	a = b;
	b = t;
}

//--Undo/redo information------------------------------------------------

//--Remember selections for each conversion type-------------------------
//extern short		ConvType;
//extern short		From;
//extern short		To;
//extern short		SelectFrom[];
//extern short		SelectTo[];

//--Function Prototypes by file------------------------------------------

//--readwrit.c-----------------------------------------------------------
//void ReadRegisters(const char *const SaveFile);
//void SaveRegisters(const char *const SaveFile);

//--program.c------------------------------------------------------------
//void RunProgram(int p);

// for GCC - should be there!

#ifdef __linux
LD _hypot(const LD &x,const LD &y);
LD _hypotsq(const LD &x,const LD &y);
#endif

//--Constants for various windows----------------------------------------

//--Text for help contexts-----------------------------------------------

const QString INDEXHELP		= "Index.htm";
const QString FATALHELP		= "Fatal.htm";
const QString GENERALHELP	= "General.htm";
const QString CONSTANTSHELP	= "Constants.htm";
const QString CONVERSIONHELP	= "UnitConversion.htm";
const QString CONVSETUPHELP	= "ConversionSetup.htm";
const QString EDITKEYHELP	= "KeyboardEdit.htm";
const QString EDITKEYSHELP	= "EditKeys.htm";
const QString PROGRAMHELP	= "Programming.htm";
const QString DECIMALKEYHELP	= "QuickDecimalKeys.htm";
const QString COMPLEXKEYHELP	= "QuickComplexKeys.htm";
const QString BINARYKEYHELP	= "QuickBinaryKeys.htm";
const QString CODESHELP		= "ProgramCodes";

// Singletons

class QEngine;

class xcalc {
	QEngine *m_prnt;
	Register m_lreg,m_ST[STKS];
	Register m_mem[MEMS];
	Register m_noreg;			// segfault insurance
	Register *regptr(int reg);
	Register *memptr(int reg);
	UndoInfoStruct m_UndoInfo[MAXUNDO];
	int			m_UndoIndex;	// how many operations can be undone
	bool 		m_SL;			// Stacklift true/false
	funid		m_LastUndone;	// Last undone operation
	int			m_Undoes;		// how many operations can be redone
	bool		m_kuseComma;	// TRUE for keyboard decimal comma
	bool		m_duseComma;	// TRUE for display decimal comma
	bool		m_cuseComma;	// TRUE for clipboard decimal comma
	bool		m_Cstyle;		// TRUE to use 0x.. clipboard copy
	void		SetLastX() {m_lreg = m_ST[iXreg];}
	void		reset();
	messagecode	m_msg;			// message, noundo, undone, redone
	QString		m_smsg;			// any message to UI
private:
	// was macros, now functions
	void RUN(void (xcalc::*)());
	void STO(int n);
	void RCL(int n);
	void CLR(int n);
	void showundoes();
	// Private operations (e.g. math functions)
	void incWL();
	void decWL();
	void ToDecimal();
	void ToComplex();
	void ToBin();
	void ToCart();
	void ToPolar();
	void clx();
	void clstk();
	void drop();
	void RUP();
	void RDN();
	void Chs();
	void Add();
	void Sub();
	void Mul();
	void Div();
	void Divf();
	void Mod();
	void Modf();
	void Sqrt();
	void Sqr();
	void Qroot();
	void Cube();
	void Log();
	void Exp();
	void Log10();
	void Ten();
	void Pow();
	void Root();
	void Sin();
	void Cos();
	void Tan();
	void ASin();
	void ACos();
	void ATan();
	void Sinh();
	void Cosh();
	void Tanh();
	void ArSinh();
	void ArCosh();
	void ArTanh();
	void Abs();
	void Rcp();
	void Pi();
	void Fact();
	void TOij();
	void TOyx();
	void ReIm();
	void Conj();
	void And();
	void Or();
	void Not();
	void Xor();
	void Shl();
	void Shr();
	void Ashl();
	void Ashr();
	void Rotl();
	void Rotr();
	void Random();
	void ToFrac();
	void ToInt();
	void ToggleCleanFrac();
	void Round();
	void set8bit();
	void set16bit();
	void set32bit();
	void set64bit();
public:
	xcalc(QEngine *prnt);
	messagecode amessage() { return m_msg; }
	QString qmessage() { return m_smsg; }
	funid lastundone() { return m_LastUndone; }
	void ClearMsg() { m_smsg.clear(); }
	void resetmsg();
	void lift(); // need it public for Inkey operation
	// Getters/setters ( all names lc!)
	bool sl() { return m_SL; }
	void setsl(bool sl) { m_SL=sl; }
	int fix() { return xcalc_fix; }
	void setfix(int f) { if (f>=FLOATDISP && f<=9) xcalc_fix = f; }
	bool cleanfrac() { return xcalc_cleanFrac; }
	void setcleanfrac(bool c) { xcalc_cleanFrac = c; update(); }
	bool invtrigasdms() { return xcalc_invTrigAsDMS; }
	void setinvtrigasdms(bool d) { xcalc_invTrigAsDMS = d; }
	FixType fixtype() { return xcalc_fixtype; }
	void setfixtype(FixType ft) { xcalc_fixtype = ft; update(); }
	RadixType radixtype() { return xcalc_radix; }
	void setradixtype(RadixType r) { xcalc_radix = r; update(); }
	// degree/radian support (always radians in complex mode)
	AngType angtype() { return xcalc_radix==rtDECIMAL?xcalc_ang:atRADIAN; }
	Register RADTOANG(Register a);
	Register ANGTORAD(Register a);
	void setangtype(AngType a) { xcalc_ang = a; update(); }
	// Value getters
	Register &regr(int r) { return *regptr(r); }
	LD realval(int reg) { return regr(reg).realval(); }
	LD imagval(int reg) { return regr(reg).imagval(); }
	qint64 ival(int reg) { return regr(reg).ival(); }
	LD asfloat(int reg) { return regr(reg).asfloat(); }
	LD asreal(int reg) { return regr(reg).asreal(); }
	LD asimag(int reg) { return regr(reg).asimag(); }
	qint64 asint(int reg) { return regr(reg).asint(); }
	ContentType ct(int reg) { return regr(reg).ct(); }
	qint64 numval(int reg) { return regr(reg).numval(); }
	qint64 denval(int reg) { return regr(reg).denval(); }
	// Public operations
	messagecode func(funid k,bool redone=false);
	void SaveUndo(funid type,bool redone);
	messagecode Undo(funid &Undone);
	messagecode Redo(funid &Redone);
	void inkey(Register &r,bool undoable); // all comments read Inkey (!)
	void inkey(); // used by Redo
	void update(); // tell all viewers to refresh
	funid nextundo() { if (m_UndoIndex>0) return m_UndoInfo[m_UndoIndex-1].undone; else return FUN_NONE; }
	funid nextredo() { if (m_Undoes>0) return m_UndoInfo[m_UndoIndex].undone; else return FUN_NONE; }
};

class XCalcApp: public QApplication {
public:
	XCalcApp(int argc, char *argv[]);
	~XCalcApp();
	void ReadProfile();
	void SaveProfile();
	QEngine *m_qengine;
	Profile *m_profile;
};

// Variables needed everywhere

extern idtextmap fname;
class XCALCWindow;
extern XCALCWindow *xcalc_win;

#endif //XCALC_H
