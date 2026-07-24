/*
  XCALC simple RPN Calculator
  Copyright (C) 1992-2012  Bernt Ribbum

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

#include "xcalc.h"
#include "util.h"
#include "xcalcutil.h"
#include "xcalcrc.h"
#include "xcalcwindow.h"
#include <QMessageBox>
#include "qengine.h"
#include "aschar.h"
#include <QKeyEvent>
#include <QDialog>

	//Pushbutton layout, one list per mode (keyids)
	idList DB,CB,HB,OB,BB; // DecimalButs, ComplexButs etc... (places buttons and links to actions)
	idList DA,CA,HA,OA,BA; // Allowed ids for mode (no restrictions on size!)
	idtextmap Keycaps; // single mapping from key id to key caption
	idtextmap Keytips; // single mapping from key id to tooltip
	idtextmap fname; // single mapping from func id to func name (used everywhere)
/* funid defined in xcalc.h */
const int NIDS = 1+FUN_LAST-FUN_NONE;

// Shortcuts

using namespace Qt;

// Undo text (lookup by funid!)

idseqmap xcshortcuts;

// Keys to handle locally (don't send to engine) mainly help functions and number input (not undoable)

idList local_funs;

bool XCALCWindow::m_mapsok = false;

int MaxInputDigits(RadixType rt,WordLength wl) {
	if (rt==rtDECIMAL) return DECMAXDIGITS;
	else if (rt==rtCOMPLEX) return CPLXMAXDIGITS;
	else if (rt==rtHEX) return
			wl==wl8BIT?HEXMAXDIGITS8:wl==wl16BIT?HEXMAXDIGITS16:wl==wl32BIT?HEXMAXDIGITS32:HEXMAXDIGITS64;
	else if	(rt==rtOCTAL) return
			wl==wl8BIT?OCTMAXDIGITS8:wl==wl16BIT?OCTMAXDIGITS16:wl==wl32BIT?OCTMAXDIGITS32:OCTMAXDIGITS64;
	else if (rt==rtBINARY) return
			wl==wl8BIT?BINMAXDIGITS8:wl==wl16BIT?BINMAXDIGITS16:wl==wl32BIT?BINMAXDIGITS32:BINMAXDIGITS64;
	else return 42;
};

XCALCWindow::XCALCWindow(QWidget *parent):
	QMainWindow(parent),
	m_qengine(0),
	m_xengine(0),
	m_iState(0), // input key state (numbers, fractions)
	m_iState2(-4), // other part of key state when complex is being keyed
	m_fixtype(ftFIX),
	m_ang(atDEGREE) {
	// set help directory for everyone
	SetHelpDirDflt();
	// set icon
	QPixmap pm;
	QString pname=chext(appPath(),"gif");
	pm.load(pname);
	this->setWindowIcon(pm);
	m_idMapper = new QSignalMapper(this);
	if (!m_mapsok) InitMaps();
	m_qengine = ((XCalcApp*)qApp)->m_qengine;
	m_xengine = m_qengine->m_xengine;
	m_grid=0, m_lstk=0, m_but=0;

	SetupUi();

	setFocusPolicy(Qt::StrongFocus);
	connect(m_idMapper, SIGNAL(mapped(int)),this, SLOT(funcslot(int)));
	connect(m_qengine,SIGNAL(changed()),this,SLOT(updateslot()));
	connect(m_qengine,SIGNAL(radixchanged()),this,SLOT(newradixslot()));
}

XCALCWindow::~XCALCWindow()
{
}

RadixType radix()
{
	return xcalc_radix;
}

void setradix(RadixType r)
{
	xcalc_radix = r;
}

WordLength wordLength() {
	return xcalc_wordLength;
}

void setWordLength(WordLength wl) {
	xcalc_wordLength = wl;
}

void XCALCWindow::InitMaps()
{
	// one time only init of action lists and maps:
	// Set up function names (used for messages & debugging)
	m_mapsok = true;
	// Set up shortcuts, one by one... (safer than array!)
	xcshortcuts[FUN_NONE] = 0;
	xcshortcuts[FUN_ABOUT]=CTRL+Key_A;

	xcshortcuts[FUN_0]=Key_0;
	xcshortcuts[FUN_1]=Key_1;
	xcshortcuts[FUN_2]=Key_2;
	xcshortcuts[FUN_3]=Key_3;
	xcshortcuts[FUN_4]=Key_4;
	xcshortcuts[FUN_5]=Key_5;
	xcshortcuts[FUN_6]=Key_6;
	xcshortcuts[FUN_7]=Key_7;
	xcshortcuts[FUN_8]=Key_8;
	xcshortcuts[FUN_9]=Key_9;
	xcshortcuts[FUN_A]=Key_A;
	xcshortcuts[FUN_B]=Key_B;
	xcshortcuts[FUN_C]=Key_C;
	xcshortcuts[FUN_D]=Key_D;
	xcshortcuts[FUN_E]=Key_E;
	xcshortcuts[FUN_F]=Key_F;
	xcshortcuts[FUN_POINT]=Key_Period;
	xcshortcuts[FUN_BACK]=Key_Backspace;
	xcshortcuts[FUN_IMAGPART]=Key_Comma;
	xcshortcuts[FUN_COPY]=Key_Copy;
	xcshortcuts[FUN_PASTE]=Key_Paste;
	xcshortcuts[FUN_FIX]=ALT+Key_I;
	xcshortcuts[FUN_SCI]=ALT+Key_S;
	xcshortcuts[FUN_ENG]=ALT+Key_E;
	xcshortcuts[FUN_FLOAT]=ALT+Key_F;
	xcshortcuts[FUN_FIX0]=ALT+Key_0;
	xcshortcuts[FUN_FIX1]=ALT+Key_1;
	xcshortcuts[FUN_FIX2]=ALT+Key_2;
	xcshortcuts[FUN_FIX3]=ALT+Key_3;
	xcshortcuts[FUN_FIX4]=ALT+Key_4;
	xcshortcuts[FUN_FIX5]=ALT+Key_5;
	xcshortcuts[FUN_FIX6]=ALT+Key_6;
	xcshortcuts[FUN_FIX7]=ALT+Key_7;
	xcshortcuts[FUN_FIX8]=ALT+Key_8;
	xcshortcuts[FUN_FIX9]=ALT+Key_9;
	xcshortcuts[FUN_FIXP]=0; // routed from rotr
	xcshortcuts[FUN_FIXM]=0; // routed from rotl
	xcshortcuts[FUN_DEG]=ALT+Key_G;
	xcshortcuts[FUN_RAD]=ALT+Key_R;
	xcshortcuts[FUN_BINARY]=ALT+Key_B;
	xcshortcuts[FUN_OCTAL]=ALT+Key_O;
	xcshortcuts[FUN_DECIMAL]=ALT+Key_D;
	xcshortcuts[FUN_COMPLEX]=ALT+Key_C;
	xcshortcuts[FUN_HEX]=ALT+Key_H;
	xcshortcuts[FUN_8BIT]=Key_F9;
	xcshortcuts[FUN_16BIT]=Key_F10;
	xcshortcuts[FUN_32BIT]=Key_F11;
	xcshortcuts[FUN_64BIT]=Key_F12;
	xcshortcuts[FUN_WLUP]=ALT+SHIFT+Key_W;
	xcshortcuts[FUN_WLDOWN]=ALT+Key_W;

	//General config
	xcshortcuts[FUN_CONFIG]=ALT+Key_N;
	xcshortcuts[FUN_EXIT]=ALT+Key_X;
	xcshortcuts[FUN_HELP]=Key_F1;
	xcshortcuts[FUN_KEYHELP]=CTRL+Key_F1;
	xcshortcuts[FUN_POPUP]=CTRL+SHIFT+Key_M;

	//Program keys

	xcshortcuts[FUN_PROG1]=META+Key_F1;
	xcshortcuts[FUN_PROG2]=META+Key_F2;
	xcshortcuts[FUN_PROG3]=META+Key_F3;
	xcshortcuts[FUN_PROG4]=META+Key_F4;
	xcshortcuts[FUN_PROG5]=META+Key_F5;
	xcshortcuts[FUN_PROG6]=META+Key_F6;
	xcshortcuts[FUN_PROG7]=META+Key_F7;
	xcshortcuts[FUN_PROG8]=META+Key_F8;
	xcshortcuts[FUN_PROG9]=META+Key_F9;
	xcshortcuts[FUN_PROG10]=META+Key_F10;
	xcshortcuts[FUN_PROG11]=META+Key_F11;
	xcshortcuts[FUN_PROG12]=META+Key_F12;
	xcshortcuts[FUN_EDITPR]=ALT+Key_F1;
	xcshortcuts[FUN_CHS]=Key_M;
	xcshortcuts[FUN_CLX]=0; // routed from BACK
	xcshortcuts[FUN_ENTER]=Key_Return;
	xcshortcuts[FUN_ADD]=Key_Plus;
	xcshortcuts[FUN_SUB]=Key_Minus;
	xcshortcuts[FUN_MUL]=Key_Asterisk;
	xcshortcuts[FUN_DIV]=Key_Slash;
	xcshortcuts[FUN_DIVF]=SHIFT+Key_D;
	xcshortcuts[FUN_MOD]=Key_Percent;
	xcshortcuts[FUN_MODF]=CTRL+Key_M;
	xcshortcuts[FUN_LASTX]=Key_L;
	xcshortcuts[FUN_UNDO]=CTRL+Key_Z;
	xcshortcuts[FUN_REDO]=CTRL+Key_Y;

	xcshortcuts[FUN_CLSTK]=SHIFT+Key_Z;
	xcshortcuts[FUN_TOIJ]=Key_I;
	xcshortcuts[FUN_FRAC]=Key_Space;
	xcshortcuts[FUN_DMS]=0; // routed from CONJ
	xcshortcuts[FUN_TODMS]=CTRL+Key_Apostrophe;
	xcshortcuts[FUN_ROUND]=Key_Dollar; // also s-R
	xcshortcuts[FUN_ROUND2]=SHIFT+Key_R; // translated to FUN_ROUND in translatekid
	xcshortcuts[FUN_XY]=Key_Less;
	xcshortcuts[FUN_RUP]=Key_Up;
	xcshortcuts[FUN_RDN]=Key_Down;
	xcshortcuts[FUN_PERC]=0; // manually routed from mod

	xcshortcuts[FUN_CONV]=Key_V;
	xcshortcuts[FUN_QCONV]=SHIFT+Key_V;
	xcshortcuts[FUN_CONS]=Key_K;
	xcshortcuts[FUN_QCONS]=SHIFT+Key_K;
	xcshortcuts[FUN_RAND]=Key_Question;
	xcshortcuts[FUN_TOFRAC]=CTRL+Key_F;
	xcshortcuts[FUN_TOINT]=CTRL+Key_I;
	xcshortcuts[FUN_CLEANFRAC]=SHIFT+Key_F;
	xcshortcuts[FUN_CART]=Key_Colon;
	xcshortcuts[FUN_POLAR]=Key_Semicolon;
	xcshortcuts[FUN_EXP]=SHIFT+Key_E;
	xcshortcuts[FUN_TEN]=SHIFT+Key_N;
	xcshortcuts[FUN_ROOT]=Key_R;
	xcshortcuts[FUN_SQRT]=Key_Q;
	xcshortcuts[FUN_QROOT]=Key_U;
	xcshortcuts[FUN_LN]=SHIFT+Key_L;
	xcshortcuts[FUN_LOG]=SHIFT+Key_G;
	xcshortcuts[FUN_POW]=Key_O;
	xcshortcuts[FUN_SQR]=SHIFT+Key_Q;
	xcshortcuts[FUN_CUBE]=SHIFT+Key_U;
	xcshortcuts[FUN_ABS]=0; // routed from OR
	xcshortcuts[FUN_SIN]=Key_S;
	xcshortcuts[FUN_COS]=0; // routed from "C"
	xcshortcuts[FUN_TAN]=Key_T;
	xcshortcuts[FUN_ASIN]=SHIFT+Key_S;
	xcshortcuts[FUN_ACOS]=SHIFT+Key_C;
	xcshortcuts[FUN_ATAN]=SHIFT+Key_T;
	xcshortcuts[FUN_SINH]=META+ALT+Key_S; // need two modifiers to not crash with predefined keys
	xcshortcuts[FUN_COSH]=META+ALT+Key_C;
	xcshortcuts[FUN_TANH]=META+ALT+Key_T;
	xcshortcuts[FUN_ARSINH]=META+ALT+SHIFT+Key_S;
	xcshortcuts[FUN_ARCOSH]=META+ALT+SHIFT+Key_C;
	xcshortcuts[FUN_ARTANH]=META+ALT+SHIFT+Key_T;
	xcshortcuts[FUN_RCP]=Key_Backslash;
	xcshortcuts[FUN_PI]=Key_P;
	xcshortcuts[FUN_FACT]=Key_Exclam;
	xcshortcuts[FUN_TOYX]=SHIFT+Key_I;
	xcshortcuts[FUN_REIM]=SHIFT+Key_J;
	xcshortcuts[FUN_CONJ]=Key_Apostrophe;
	xcshortcuts[FUN_AND]=Key_Ampersand;
	xcshortcuts[FUN_OR]=Key_Bar;
	xcshortcuts[FUN_XOR]=Key_X;
	xcshortcuts[FUN_NOT]=Key_AsciiTilde;
	xcshortcuts[FUN_NOT2]=Key_N; // translated to FUN_NOT in translatekid
	xcshortcuts[FUN_SHL]=SHIFT+Key_Left;
	xcshortcuts[FUN_SHR]=SHIFT+Key_Right;
	xcshortcuts[FUN_ASHL]=ALT+SHIFT+Key_Left;
	xcshortcuts[FUN_ASHR]=ALT+SHIFT+Key_Right;
	xcshortcuts[FUN_ROTL]=Key_Left;
	xcshortcuts[FUN_ROTR]=Key_Right;
	xcshortcuts[FUN_STO0]=CTRL+Key_0;
	xcshortcuts[FUN_STO1]=CTRL+Key_1;
	xcshortcuts[FUN_STO2]=CTRL+Key_2;
	xcshortcuts[FUN_STO3]=CTRL+Key_3;
	xcshortcuts[FUN_STO4]=CTRL+Key_4;
	xcshortcuts[FUN_STO5]=CTRL+Key_5;
	xcshortcuts[FUN_STO6]=CTRL+Key_6;
	xcshortcuts[FUN_STO7]=CTRL+Key_7;
	xcshortcuts[FUN_STO8]=CTRL+Key_8;
	xcshortcuts[FUN_STO9]=CTRL+Key_9;
	xcshortcuts[FUN_RCL0]=CTRL+SHIFT+Key_0;
	xcshortcuts[FUN_RCL1]=CTRL+SHIFT+Key_1;
	xcshortcuts[FUN_RCL2]=CTRL+SHIFT+Key_2;
	xcshortcuts[FUN_RCL3]=CTRL+SHIFT+Key_3;
	xcshortcuts[FUN_RCL4]=CTRL+SHIFT+Key_4;
	xcshortcuts[FUN_RCL5]=CTRL+SHIFT+Key_5;
	xcshortcuts[FUN_RCL6]=CTRL+SHIFT+Key_6;
	xcshortcuts[FUN_RCL7]=CTRL+SHIFT+Key_7;
	xcshortcuts[FUN_RCL8]=CTRL+SHIFT+Key_8;
	xcshortcuts[FUN_RCL9]=CTRL+SHIFT+Key_9;
	xcshortcuts[FUN_CLR0]=CTRL+ALT+Key_0;
	xcshortcuts[FUN_CLR1]=CTRL+ALT+Key_1;
	xcshortcuts[FUN_CLR2]=CTRL+ALT+Key_2;
	xcshortcuts[FUN_CLR3]=CTRL+ALT+Key_3;
	xcshortcuts[FUN_CLR4]=CTRL+ALT+Key_4;
	xcshortcuts[FUN_CLR5]=CTRL+ALT+Key_5;
	xcshortcuts[FUN_CLR6]=CTRL+ALT+Key_6;
	xcshortcuts[FUN_CLR7]=CTRL+ALT+Key_7;
	xcshortcuts[FUN_CLR8]=CTRL+ALT+Key_8;
	xcshortcuts[FUN_CLR9]=CTRL+ALT+Key_9;
	xcshortcuts[FUN_CLRMEM]=CTRL+ALT+Key_M;

	// Check shortcut ambiguity
	intlist il;
	for (funid kid=FUN_NONE;kid<FUN_LAST;kid=(funid)(kid+1)) {
		int sc = xcshortcuts[kid];
		if (sc!=0) {
			if (!il.contains(sc))
				il.append(sc);
			else {
				QString msg = QString("Duplicate:")+QString::number(sc,16);
				QMessageBox::warning(this,"Shortcuts",msg);
			}
		}
	}
	// create all actions, set accelerator and map triggered() instead of buttons and clicked() as was done in SetupUi
	// note: it would be better to make a radix dependent mapping instead of filtering manually in translatekid().
	for (funid kid=FUN_NONE;kid<FUN_LAST;kid=(funid)(kid+1)) {
		QAction *act=new QAction(Keycaps.value(kid),this);
		act->setShortcutContext(Qt::ApplicationShortcut);
		act->setShortcut(xcshortcuts.value(kid));
		connect(act,SIGNAL(triggered()),m_idMapper,SLOT(map()));
		m_idMapper->setMapping(act,kid);
		addAction(act); // action must be connected to a widget to work!
	}
	// Add pushbuttons depending on mode (captions automatically generated based on keyid)
	// Note: must be exactly BUTROWS*BUTCOLS buttons (currently 10*9)
	// Note: FUN_ENTER, FUN_UNDO and FUN_REDO automatically occupy two adjacent locations - make sure there is a FUN_NONE to the right
	// Decimal
	DB.clear();
	DB.append(FUN_STO0);	DB.append(FUN_STO1);	DB.append(FUN_RCL0);	DB.append(FUN_RCL1);	DB.append(FUN_CLR0);	DB.append(FUN_CLR1);	DB.append(FUN_CART);	DB.append(FUN_POLAR);	DB.append(FUN_CLRMEM);
	DB.append(FUN_SIN);     DB.append(FUN_COS);     DB.append(FUN_TAN);     DB.append(FUN_SINH);    DB.append(FUN_COSH);    DB.append(FUN_TANH);    DB.append(FUN_POW);     DB.append(FUN_UNDO);    DB.append(FUN_NONE);
	DB.append(FUN_ASIN);    DB.append(FUN_ACOS);    DB.append(FUN_ATAN);    DB.append(FUN_ARSINH);  DB.append(FUN_ARCOSH);  DB.append(FUN_ARTANH);  DB.append(FUN_ROOT);    DB.append(FUN_REDO);    DB.append(FUN_NONE);
	DB.append(FUN_BINARY);  DB.append(FUN_FIX4);    DB.append(FUN_FIX);     DB.append(FUN_SCI);     DB.append(FUN_ENG);     DB.append(FUN_CHS);     DB.append(FUN_LN  );    DB.append(FUN_DMS);     DB.append(FUN_TODMS);
	DB.append(FUN_OCTAL);   DB.append(FUN_FLOAT);   DB.append(FUN_SQRT);    DB.append(FUN_SQR);     DB.append(FUN_CUBE);    DB.append(FUN_ABS);     DB.append(FUN_EXP );    DB.append(FUN_FRAC);    DB.append(FUN_LASTX);
	DB.append(FUN_HEX);     DB.append(FUN_WLUP);    DB.append(FUN_7);       DB.append(FUN_8);       DB.append(FUN_9);       DB.append(FUN_ROUND);   DB.append(FUN_LOG );    DB.append(FUN_ADD);     DB.append(FUN_RUP);
	DB.append(FUN_COMPLEX); DB.append(FUN_WLDOWN);	DB.append(FUN_4);       DB.append(FUN_5);       DB.append(FUN_6);       DB.append(FUN_RAND);    DB.append(FUN_TEN );    DB.append(FUN_SUB);     DB.append(FUN_RDN);
	DB.append(FUN_DECIMAL); DB.append(FUN_EEX);     DB.append(FUN_1);       DB.append(FUN_2);       DB.append(FUN_3);       DB.append(FUN_PI);      DB.append(FUN_RCP );    DB.append(FUN_MUL);     DB.append(FUN_BACK);
	DB.append(FUN_DEG);     DB.append(FUN_RAD);     DB.append(FUN_POINT);   DB.append(FUN_0);       DB.append(FUN_IMAGPART);DB.append(FUN_PERC);    DB.append(FUN_FACT);    DB.append(FUN_DIV);     DB.append(FUN_CLSTK);
	DB.append(FUN_EXIT);    DB.append(FUN_ABOUT);   DB.append(FUN_CONFIG);  DB.append(FUN_HELP);    DB.append(FUN_KEYHELP); DB.append(FUN_XY);      DB.append(FUN_TOIJ);    DB.append(FUN_ENTER);   DB.append(FUN_NONE);
	// Complex
	CB.clear();
	CB.append(FUN_STO0);	CB.append(FUN_STO1);	CB.append(FUN_RCL0);	CB.append(FUN_RCL1);	CB.append(FUN_CLR0);	CB.append(FUN_CLR1);	CB.append(FUN_CART);	CB.append(FUN_POLAR);	CB.append(FUN_CLRMEM);
	CB.append(FUN_SIN);     CB.append(FUN_COS);     CB.append(FUN_TAN);     CB.append(FUN_SINH);    CB.append(FUN_COSH);    CB.append(FUN_TANH);    CB.append(FUN_POW);     CB.append(FUN_UNDO);    CB.append(FUN_NONE);
	CB.append(FUN_ASIN);    CB.append(FUN_ACOS);    CB.append(FUN_ATAN);    CB.append(FUN_ARSINH);  CB.append(FUN_ARCOSH);  CB.append(FUN_ARTANH);  CB.append(FUN_ROOT);    CB.append(FUN_REDO);    CB.append(FUN_NONE);
	CB.append(FUN_BINARY);  CB.append(FUN_FIX4);    CB.append(FUN_FIX);     CB.append(FUN_SCI);     CB.append(FUN_ENG);     CB.append(FUN_CHS);     CB.append(FUN_LN  );    CB.append(FUN_CONJ);    CB.append(FUN_TOYX);
	CB.append(FUN_OCTAL);   CB.append(FUN_FLOAT);   CB.append(FUN_SQRT);    CB.append(FUN_SQR);     CB.append(FUN_CUBE);    CB.append(FUN_ABS);     CB.append(FUN_EXP );    CB.append(FUN_REIM);    CB.append(FUN_LASTX);
	CB.append(FUN_HEX);     CB.append(FUN_WLUP);	CB.append(FUN_7);       CB.append(FUN_8);       CB.append(FUN_9);       CB.append(FUN_ROUND);   CB.append(FUN_LOG );    CB.append(FUN_ADD);     CB.append(FUN_RUP);
	CB.append(FUN_COMPLEX); CB.append(FUN_WLDOWN);	CB.append(FUN_4);       CB.append(FUN_5);       CB.append(FUN_6);       CB.append(FUN_RAND);    CB.append(FUN_TEN );    CB.append(FUN_SUB);     CB.append(FUN_RDN);
	CB.append(FUN_DECIMAL); CB.append(FUN_EEX);     CB.append(FUN_1);       CB.append(FUN_2);       CB.append(FUN_3);       CB.append(FUN_PI);      CB.append(FUN_RCP );    CB.append(FUN_MUL);     CB.append(FUN_BACK);
	CB.append(FUN_DEG);     CB.append(FUN_RAD);     CB.append(FUN_POINT);   CB.append(FUN_0);       CB.append(FUN_IMAGPART);CB.append(FUN_PERC);    CB.append(FUN_FACT);    CB.append(FUN_DIV);     CB.append(FUN_CLSTK);
	CB.append(FUN_EXIT);    CB.append(FUN_ABOUT);   CB.append(FUN_CONFIG);  CB.append(FUN_HELP);    CB.append(FUN_KEYHELP); CB.append(FUN_XY);      CB.append(FUN_TOIJ);    CB.append(FUN_ENTER);   CB.append(FUN_NONE);
	// Hex
	HB.clear();
	HB.append(FUN_STO0);	HB.append(FUN_STO1);	HB.append(FUN_RCL0);	HB.append(FUN_RCL1);	HB.append(FUN_CLR0);	HB.append(FUN_CLR1);	HB.append(FUN_NONE);	HB.append(FUN_NONE);	HB.append(FUN_CLRMEM);
	HB.append(FUN_SHL);     HB.append(FUN_ASHL);    HB.append(FUN_ROTL);    HB.append(FUN_AND);     HB.append(FUN_NOT);     HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_UNDO);    HB.append(FUN_NONE);
	HB.append(FUN_SHR);     HB.append(FUN_ASHR);    HB.append(FUN_ROTR);    HB.append(FUN_OR);      HB.append(FUN_XOR);     HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_REDO);    HB.append(FUN_NONE);
	HB.append(FUN_BINARY);  HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_CHS);     HB.append(FUN_8BIT);    HB.append(FUN_NONE);    HB.append(FUN_NONE);
	HB.append(FUN_OCTAL);   HB.append(FUN_C);       HB.append(FUN_D);       HB.append(FUN_E);       HB.append(FUN_F);       HB.append(FUN_NONE);    HB.append(FUN_16BIT);   HB.append(FUN_MOD);     HB.append(FUN_LASTX);
	HB.append(FUN_HEX);     HB.append(FUN_8);       HB.append(FUN_9);       HB.append(FUN_A);       HB.append(FUN_B);       HB.append(FUN_NONE);    HB.append(FUN_32BIT);   HB.append(FUN_ADD);     HB.append(FUN_RUP);
	HB.append(FUN_COMPLEX); HB.append(FUN_4);       HB.append(FUN_5);       HB.append(FUN_6);       HB.append(FUN_7);       HB.append(FUN_NONE);    HB.append(FUN_64BIT);   HB.append(FUN_SUB);     HB.append(FUN_RDN);
	HB.append(FUN_DECIMAL); HB.append(FUN_0);       HB.append(FUN_1);       HB.append(FUN_2);       HB.append(FUN_3);       HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_MUL);     HB.append(FUN_BACK);
	HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_MOD );    HB.append(FUN_DIV);     HB.append(FUN_CLSTK);
	HB.append(FUN_EXIT);    HB.append(FUN_ABOUT);   HB.append(FUN_CONFIG);  HB.append(FUN_HELP);    HB.append(FUN_KEYHELP); HB.append(FUN_NONE);    HB.append(FUN_NONE);    HB.append(FUN_ENTER);   HB.append(FUN_NONE);
	// Octal
	OB.clear();
	OB.append(FUN_STO0);	OB.append(FUN_STO1);	OB.append(FUN_RCL0);	OB.append(FUN_RCL1);	OB.append(FUN_CLR0);	OB.append(FUN_CLR1);	OB.append(FUN_NONE);	OB.append(FUN_NONE);	OB.append(FUN_CLRMEM);
	OB.append(FUN_SHL);     OB.append(FUN_ASHL);    OB.append(FUN_ROTL);    OB.append(FUN_AND);     OB.append(FUN_NOT);     OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_UNDO);    OB.append(FUN_NONE);
	OB.append(FUN_SHR);     OB.append(FUN_ASHR);    OB.append(FUN_ROTR);    OB.append(FUN_OR);      OB.append(FUN_XOR);     OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_REDO);    OB.append(FUN_NONE);
	OB.append(FUN_BINARY);  OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_CHS);     OB.append(FUN_8BIT);    OB.append(FUN_NONE);    OB.append(FUN_NONE);
	OB.append(FUN_OCTAL);   OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_16BIT);   OB.append(FUN_MOD);     OB.append(FUN_LASTX);
	OB.append(FUN_HEX);     OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_32BIT);   OB.append(FUN_ADD);     OB.append(FUN_RUP);
	OB.append(FUN_COMPLEX); OB.append(FUN_4);       OB.append(FUN_5);       OB.append(FUN_6);       OB.append(FUN_7);       OB.append(FUN_NONE);    OB.append(FUN_64BIT);   OB.append(FUN_SUB);     OB.append(FUN_RDN);
	OB.append(FUN_DECIMAL); OB.append(FUN_0);       OB.append(FUN_1);       OB.append(FUN_2);       OB.append(FUN_3);       OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_MUL);     OB.append(FUN_BACK);
	OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_MOD );    OB.append(FUN_DIV);     OB.append(FUN_CLSTK);
	OB.append(FUN_EXIT);    OB.append(FUN_ABOUT);   OB.append(FUN_CONFIG);  OB.append(FUN_HELP);    OB.append(FUN_KEYHELP); OB.append(FUN_NONE);    OB.append(FUN_NONE);    OB.append(FUN_ENTER);   OB.append(FUN_NONE);
	// Binary
	BB.clear();
	BB.append(FUN_STO0);	BB.append(FUN_STO1);	BB.append(FUN_RCL0);	BB.append(FUN_RCL1);	BB.append(FUN_CLR0);	BB.append(FUN_CLR1);	BB.append(FUN_NONE);	BB.append(FUN_NONE);	BB.append(FUN_CLRMEM);
	BB.append(FUN_SHL);     BB.append(FUN_ASHL);    BB.append(FUN_ROTL);    BB.append(FUN_AND);     BB.append(FUN_NOT);     BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_UNDO);    BB.append(FUN_NONE);
	BB.append(FUN_SHR);     BB.append(FUN_ASHR);    BB.append(FUN_ROTR);    BB.append(FUN_OR);      BB.append(FUN_XOR);     BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_REDO);    BB.append(FUN_NONE);
	BB.append(FUN_BINARY);  BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_CHS);     BB.append(FUN_8BIT);    BB.append(FUN_NONE);    BB.append(FUN_NONE);
	BB.append(FUN_OCTAL);   BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_16BIT);   BB.append(FUN_MOD);     BB.append(FUN_LASTX);
	BB.append(FUN_HEX);     BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_32BIT);   BB.append(FUN_ADD);     BB.append(FUN_RUP);
	BB.append(FUN_COMPLEX); BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_64BIT);   BB.append(FUN_SUB);     BB.append(FUN_RDN);
	BB.append(FUN_DECIMAL); BB.append(FUN_0);       BB.append(FUN_1);       BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_MUL);     BB.append(FUN_BACK);
	BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_MOD );    BB.append(FUN_DIV);     BB.append(FUN_CLSTK);
	BB.append(FUN_EXIT);    BB.append(FUN_ABOUT);   BB.append(FUN_CONFIG);  BB.append(FUN_HELP);    BB.append(FUN_KEYHELP); BB.append(FUN_NONE);    BB.append(FUN_NONE);    BB.append(FUN_ENTER);   BB.append(FUN_NONE);
	// Populate maps with every allowable action per mode. This is used to lookup if action is allowed (solves '8' in octal, translatekid solves '%' - legality tested also after translation).
	// Decimal
	DA.clear();
	DA.append(DB); // All buttons, plus some without buttons
	DA.append(FUN_NONE);
	DA.append(FUN_FIXP);
	DA.append(FUN_FIXM);
	DA.append(FUN_FIX0);
	DA.append(FUN_FIX1);
	DA.append(FUN_FIX2);
	DA.append(FUN_FIX3);
	DA.append(FUN_FIX4);
	DA.append(FUN_FIX5);
	DA.append(FUN_FIX6);
	DA.append(FUN_FIX7);
	DA.append(FUN_FIX8);
	DA.append(FUN_FIX9);
	DA.append(FUN_SINH);
	DA.append(FUN_COSH);
	DA.append(FUN_TANH);
	DA.append(FUN_ARSINH);
	DA.append(FUN_ARCOSH);
	DA.append(FUN_ARTANH);
	DA.append(FUN_DIVF);
	DA.append(FUN_MODF);
	DA.append(FUN_QROOT);
	DA.append(FUN_WLDOWN);
	DA.append(FUN_STO0); DA.append(FUN_STO1); DA.append(FUN_STO2); DA.append(FUN_STO3); DA.append(FUN_STO4); DA.append(FUN_STO5); DA.append(FUN_STO6); DA.append(FUN_STO7); DA.append(FUN_STO8); DA.append(FUN_STO9);
	DA.append(FUN_STO0); DA.append(FUN_RCL1); DA.append(FUN_RCL2); DA.append(FUN_RCL3); DA.append(FUN_RCL4); DA.append(FUN_RCL5); DA.append(FUN_RCL6); DA.append(FUN_RCL7); DA.append(FUN_RCL8); DA.append(FUN_RCL9);
	DA.append(FUN_CLR0); DA.append(FUN_CLR1); DA.append(FUN_CLR2); DA.append(FUN_CLR3); DA.append(FUN_CLR4); DA.append(FUN_CLR5); DA.append(FUN_CLR6); DA.append(FUN_CLR7); DA.append(FUN_CLR8); DA.append(FUN_CLR9);
	DA.append(FUN_CLRMEM); DA.append(FUN_8BIT); DA.append(FUN_16BIT); DA.append(FUN_32BIT); DA.append(FUN_64BIT); DA.append(FUN_TOINT); DA.append(FUN_TOFRAC);
	DA.append(FUN_CLEANFRAC);
	DA.append(FUN_CART);
	DA.append(FUN_POLAR);
	// Complex
	CA.clear();
	CA.append(CB); // All buttons, plus some without buttons
	CA.append(FUN_NONE);
	//CA.append(FUN_FRAC); -- needs work in Evaluate
	CA.append(FUN_FIXP);
	CA.append(FUN_FIXM);
	CA.append(FUN_FIX0);
	CA.append(FUN_FIX1);
	CA.append(FUN_FIX2);
	CA.append(FUN_FIX3);
	CA.append(FUN_FIX4);
	CA.append(FUN_FIX5);
	CA.append(FUN_FIX6);
	CA.append(FUN_FIX7);
	CA.append(FUN_FIX8);
	CA.append(FUN_FIX9);
	CA.append(FUN_SINH);
	CA.append(FUN_COSH);
	CA.append(FUN_TANH);
	CA.append(FUN_ARSINH);
	CA.append(FUN_ARCOSH);
	CA.append(FUN_ARTANH);
	CA.append(FUN_DIVF);
	CA.append(FUN_MODF);
	CA.append(FUN_QROOT);
	CA.append(FUN_WLDOWN);
	CA.append(FUN_STO0); CA.append(FUN_STO1); CA.append(FUN_STO2); CA.append(FUN_STO3); CA.append(FUN_STO4); CA.append(FUN_STO5); CA.append(FUN_STO6); CA.append(FUN_STO7); CA.append(FUN_STO8); CA.append(FUN_STO9);
	CA.append(FUN_STO0); CA.append(FUN_RCL1); CA.append(FUN_RCL2); CA.append(FUN_RCL3); CA.append(FUN_RCL4); CA.append(FUN_RCL5); CA.append(FUN_RCL6); CA.append(FUN_RCL7); CA.append(FUN_RCL8); CA.append(FUN_RCL9);
	CA.append(FUN_CLR0); CA.append(FUN_CLR1); CA.append(FUN_CLR2); CA.append(FUN_CLR3); CA.append(FUN_CLR4); CA.append(FUN_CLR5); CA.append(FUN_CLR6); CA.append(FUN_CLR7); CA.append(FUN_CLR8); CA.append(FUN_CLR9);
	CA.append(FUN_CLRMEM); CA.append(FUN_8BIT); CA.append(FUN_16BIT); CA.append(FUN_32BIT); CA.append(FUN_64BIT); CA.append(FUN_TOINT); CA.append(FUN_TOFRAC);
	CA.append(FUN_CART);
	CA.append(FUN_POLAR);
	// Hex
	HA.clear();
	HA.append(HB); // All buttons, plus some without buttons
	HA.append(FUN_NONE);
	HA.append(FUN_STO0); HA.append(FUN_STO1); HA.append(FUN_STO2); HA.append(FUN_STO3); HA.append(FUN_STO4); HA.append(FUN_STO5); HA.append(FUN_STO6); HA.append(FUN_STO7); HA.append(FUN_STO8); HA.append(FUN_STO9);
	HA.append(FUN_STO0); HA.append(FUN_RCL1); HA.append(FUN_RCL2); HA.append(FUN_RCL3); HA.append(FUN_RCL4); HA.append(FUN_RCL5); HA.append(FUN_RCL6); HA.append(FUN_RCL7); HA.append(FUN_RCL8); HA.append(FUN_RCL9);
	HA.append(FUN_CLR0); HA.append(FUN_CLR1); HA.append(FUN_CLR2); HA.append(FUN_CLR3); HA.append(FUN_CLR4); HA.append(FUN_CLR5); HA.append(FUN_CLR6); HA.append(FUN_CLR7); HA.append(FUN_CLR8); HA.append(FUN_CLR9);
	HA.append(FUN_CLRMEM); HA.append(FUN_WLUP); HA.append(FUN_WLDOWN);
	// Octal
	OA.clear();
	OA.append(OB); // All buttons, plus some without buttons
	OA.append(FUN_NONE);
	OA.append(FUN_STO0); OA.append(FUN_STO1); OA.append(FUN_STO2); OA.append(FUN_STO3); OA.append(FUN_STO4); OA.append(FUN_STO5); OA.append(FUN_STO6); OA.append(FUN_STO7); OA.append(FUN_STO8); OA.append(FUN_STO9);
	OA.append(FUN_STO0); OA.append(FUN_RCL1); OA.append(FUN_RCL2); OA.append(FUN_RCL3); OA.append(FUN_RCL4); OA.append(FUN_RCL5); OA.append(FUN_RCL6); OA.append(FUN_RCL7); OA.append(FUN_RCL8); OA.append(FUN_RCL9);
	OA.append(FUN_CLR0); OA.append(FUN_CLR1); OA.append(FUN_CLR2); OA.append(FUN_CLR3); OA.append(FUN_CLR4); OA.append(FUN_CLR5); OA.append(FUN_CLR6); OA.append(FUN_CLR7); OA.append(FUN_CLR8); OA.append(FUN_CLR9);
	OA.append(FUN_CLRMEM); OA.append(FUN_WLUP); OA.append(FUN_WLDOWN);
	// Binary
	BA.clear();
	BA.append(BB); // All buttons, plus some without buttons
	BA.append(FUN_NONE);
	BA.append(FUN_STO0); BA.append(FUN_STO1); BA.append(FUN_STO2); BA.append(FUN_STO3); BA.append(FUN_STO4); BA.append(FUN_STO5); BA.append(FUN_STO6); BA.append(FUN_STO7); BA.append(FUN_STO8); BA.append(FUN_STO9);
	BA.append(FUN_STO0); BA.append(FUN_RCL1); BA.append(FUN_RCL2); BA.append(FUN_RCL3); BA.append(FUN_RCL4); BA.append(FUN_RCL5); BA.append(FUN_RCL6); BA.append(FUN_RCL7); BA.append(FUN_RCL8); BA.append(FUN_RCL9);
	BA.append(FUN_CLR0); BA.append(FUN_CLR1); BA.append(FUN_CLR2); BA.append(FUN_CLR3); BA.append(FUN_CLR4); BA.append(FUN_CLR5); BA.append(FUN_CLR6); BA.append(FUN_CLR7); BA.append(FUN_CLR8); BA.append(FUN_CLR9);
	BA.append(FUN_CLRMEM); BA.append(FUN_WLUP); DA.append(FUN_WLDOWN);
	//Local keys (handled here, not to be send to engine)
	//number input only
	local_funs.clear();
	local_funs.append(FUN_0);
	local_funs.append(FUN_1);
	local_funs.append(FUN_2);
	local_funs.append(FUN_3);
	local_funs.append(FUN_4);
	local_funs.append(FUN_5);
	local_funs.append(FUN_6);
	local_funs.append(FUN_7);
	local_funs.append(FUN_8);
	local_funs.append(FUN_9);
	local_funs.append(FUN_A);
	local_funs.append(FUN_B);
	local_funs.append(FUN_C); //translated to non-local FUN_COS in decimal/complex mode - no need to bother here
	local_funs.append(FUN_D);
	local_funs.append(FUN_E); //translated to local FUN_EEX in decimal/complex mode - no need to bother here
	local_funs.append(FUN_F);
	local_funs.append(FUN_POINT);
	local_funs.append(FUN_CHS); //sent to engine in iState==0,-4 and binary modes
	local_funs.append(FUN_EEX);
	local_funs.append(FUN_BACK); // sent to engine in iState==0,-4
	local_funs.append(FUN_DMS);
	local_funs.append(FUN_FRAC);
	local_funs.append(FUN_IMAGPART);
	//local_funs.append(FUN_COPY);
	//local_funs.append(FUN_PASTE);
	//fnames,
	fname[FUN_NONE]="None";
	fname[FUN_ABOUT]="About";
	fname[FUN_0]="0"; fname[FUN_1]="1"; fname[FUN_2]="2"; fname[FUN_3]="3";	fname[FUN_4]="4";	fname[FUN_5]="5";	fname[FUN_6]="6";	fname[FUN_7]="7";	fname[FUN_8]="8";	fname[FUN_9]="9";
	fname[FUN_A]="A"; fname[FUN_B]="B"; fname[FUN_C]="C"; fname[FUN_D]="D";	fname[FUN_E]="E";	fname[FUN_F]="F";	fname[FUN_POINT]=".";	fname[FUN_EEX]="EEX";
	fname[FUN_BACK]="Back";	fname[FUN_IMAGPART]="imagpart";	fname[FUN_COPY]="Copy"; fname[FUN_PASTE]="Paste";
	fname[FUN_FIX]="Fix"; fname[FUN_SCI]="Sci"; fname[FUN_ENG]="Eng";	fname[FUN_FLOAT]="Float";
	fname[FUN_FIX0]="Fix0"; fname[FUN_FIX1]="Fix1"; fname[FUN_FIX2]="Fix2";	fname[FUN_FIX3]="Fix3";	fname[FUN_FIX4]="Fix4";	fname[FUN_FIX5]="Fix5"; fname[FUN_FIX6]="Fix6";
	fname[FUN_FIX7]="Fix7"; fname[FUN_FIX8]="Fix8"; fname[FUN_FIX9]="Fix9";
	fname[FUN_FIXP]="Fix+"; fname[FUN_FIXM]="Fix-";
	fname[FUN_DEG]="Deg"; fname[FUN_RAD]="Rad"; fname[FUN_DECIMAL]="Decimal"; fname[FUN_COMPLEX]="Complex"; fname[FUN_HEX]="Hex"; fname[FUN_OCTAL]="Octal"; fname[FUN_BINARY]="Bin";
	fname[FUN_CONFIG]="Config"; fname[FUN_EXIT]="Exit"; fname[FUN_HELP]="Help"; fname[FUN_KEYHELP]="Keyhelp"; fname[FUN_POPUP]="Popup";
	fname[FUN_PROG1]="Prog1"; fname[FUN_PROG2]="Prog2"; fname[FUN_PROG3]="Prog3"; fname[FUN_PROG4]="Prog4"; fname[FUN_PROG5]="Prog5"; fname[FUN_PROG6]="Prog6";
	fname[FUN_PROG7]="Prog7"; fname[FUN_PROG8]="Prog8"; fname[FUN_PROG9]="Prog9"; fname[FUN_PROG10]="Prog10"; fname[FUN_PROG11]="Prog11"; fname[FUN_PROG12]="Prog12";
	fname[FUN_EDITPR]="Editprog";
	fname[FUN_ROUND2]="Round";
	fname[FUN_NOT2]="Not";
	fname[FUN_OR2]="Or";
	fname[FUN_INKEY]="Inkey"; fname[FUN_CHS]="Change sign";
	fname[FUN_CLX]="Clear X"; fname[FUN_ENTER]="Enter"; fname[FUN_ADD]="Add";	fname[FUN_SUB]="Sub";	fname[FUN_MUL]="Mul";	fname[FUN_DIV]="Div";	fname[FUN_DIVF]="Divf";	fname[FUN_MOD]="Mod";
	fname[FUN_MODF]="Modf";	fname[FUN_LASTX]="Lastx";	fname[FUN_UNDO]="Undo";	fname[FUN_REDO]="Redo";
	fname[FUN_CLSTK]="Clstk"; fname[FUN_TOIJ]="Y+iX"; fname[FUN_FRAC]="Frac"; fname[FUN_DMS]="dms"; fname[FUN_TODMS]="todms"; fname[FUN_ROUND]="Round"; fname[FUN_XY]="x<>y"; fname[FUN_RUP]="Rup"; fname[FUN_RDN]="Rdn"; fname[FUN_PERC]="Percent";
	fname[FUN_CONV]="Conv"; fname[FUN_QCONV]="Qconv"; fname[FUN_CONS]="Const"; fname[FUN_QCONS]="Qconst"; fname[FUN_RAND]="Random"; fname[FUN_TOFRAC]="Tofrac";
	fname[FUN_TOINT]="Toint"; fname[FUN_CART]="Cart"; fname[FUN_POLAR]="Pol"; fname[FUN_EXP]="exp";
	fname[FUN_TEN]="ten"; fname[FUN_ROOT]="Root"; fname[FUN_SQRT]="Sqrt"; fname[FUN_QROOT]="Qroot"; fname[FUN_LN]="Ln"; fname[FUN_LOG]="Log10"; fname[FUN_POW]="pow";
	fname[FUN_SQR]="sqr"; fname[FUN_CUBE]="cube"; fname[FUN_ABS]="abs";
	fname[FUN_SIN]="sin"; fname[FUN_COS]="cos"; fname[FUN_TAN]="tan"; fname[FUN_ASIN]="asin"; fname[FUN_ACOS]="acos"; fname[FUN_ATAN]="atan";
	fname[FUN_SINH]="sinh"; fname[FUN_COSH]="cosh"; fname[FUN_TANH]="tanh"; fname[FUN_ARSINH]="arsinh"; fname[FUN_ARCOSH]="arcosh"; fname[FUN_ARTANH]="artanh";
	fname[FUN_RCP]="rcp"; fname[FUN_PI]="Pi"; fname[FUN_FACT]="fact";
	fname[FUN_TOYX]="Split"; fname[FUN_REIM]="ReIm"; fname[FUN_CONJ]="Conj";
	fname[FUN_AND]="And"; fname[FUN_OR]="Or"; fname[FUN_XOR]="Xor"; fname[FUN_XOR2]="Xor"; fname[FUN_NOT]="Not"; fname[FUN_SHL]="Shl"; fname[FUN_SHR]="Shr"; fname[FUN_ASHL]="Ashl"; fname[FUN_ASHR]="Ashr"; fname[FUN_ROTL]="Rotl"; fname[FUN_ROTR]="Rotr";
	fname[FUN_STO0]="Sto0"; fname[FUN_STO1]="Sto1"; fname[FUN_STO2]="Sto2";	fname[FUN_STO3]="Sto3";	fname[FUN_STO4]="Sto4";
	fname[FUN_STO5]="Sto5";	fname[FUN_STO6]="Sto6";	fname[FUN_STO7]="Sto7";	fname[FUN_STO8]="Sto8";	fname[FUN_STO9]="Sto9";
	fname[FUN_RCL0]="Rcl0"; fname[FUN_RCL1]="Rcl1"; fname[FUN_RCL2]="Rcl2";	fname[FUN_RCL3]="Rcl3";	fname[FUN_RCL4]="Rcl4";
	fname[FUN_RCL5]="Rcl5";	fname[FUN_RCL6]="Rcl6";	fname[FUN_RCL7]="Rcl7";	fname[FUN_RCL8]="Rcl8";	fname[FUN_RCL9]="Rcl9";
	fname[FUN_CLR0]="Clr0"; fname[FUN_CLR1]="Clr1"; fname[FUN_CLR2]="Clr2";	fname[FUN_CLR3]="Clr3";	fname[FUN_CLR4]="Clr4";
	fname[FUN_CLR5]="Clr5";	fname[FUN_CLR6]="Clr6";	fname[FUN_CLR7]="Clr7";	fname[FUN_CLR8]="Clr8";	fname[FUN_CLR9]="Clr9";
	fname[FUN_64BIT]="64bit"; fname[FUN_32BIT]="32bit"; fname[FUN_16BIT]="16bit"; fname[FUN_8BIT]="8bit";
	fname[FUN_WLUP]="WL up";
	fname[FUN_WLDOWN]="WL down";
	fname[FUN_CLRMEM]="ClrMem";
	fname[FUN_CLEANFRAC]="CleanFrac";
	fname[FUN_CART]="Cart";
	fname[FUN_POLAR]="Polar";
	fname[FUN_LAST]="Last";
	if (fname.count()!= FUN_LAST+1) {
		QMessageBox::critical(this,"Error",QString("Fname defined %1 of %2 items: XCALCWindow.cpp").arg(fname.count()).arg(FUN_LAST+1));
	}
	//Keycaps,
	Keycaps[FUN_NONE]="";
	Keycaps[FUN_ABOUT]="About";
	Keycaps[FUN_0]="0"; Keycaps[FUN_1]="1"; Keycaps[FUN_2]="2"; Keycaps[FUN_3]="3";	Keycaps[FUN_4]="4";	Keycaps[FUN_5]="5";	Keycaps[FUN_6]="6";	Keycaps[FUN_7]="7";	Keycaps[FUN_8]="8";	Keycaps[FUN_9]="9";
	Keycaps[FUN_A]="A"; Keycaps[FUN_B]="B"; Keycaps[FUN_C]="C"; Keycaps[FUN_D]="D";	Keycaps[FUN_E]="E";	Keycaps[FUN_F]="F";	Keycaps[FUN_POINT]=".";	Keycaps[FUN_CHS]=QString(L'±');
	Keycaps[FUN_EEX]="EEX";
	Keycaps[FUN_BACK]="<-"; Keycaps[FUN_ENTER]="Enter"; Keycaps[FUN_ADD]="+";	Keycaps[FUN_SUB]="-";	Keycaps[FUN_MUL]=QString(L'×');	Keycaps[FUN_DIV]="/";	Keycaps[FUN_DIVF]="Divf";	Keycaps[FUN_MOD]="Mod";
	Keycaps[FUN_MODF]="Mod";	Keycaps[FUN_LASTX]="Lastx";	Keycaps[FUN_UNDO]="Undo";	Keycaps[FUN_REDO]="Redo";
	Keycaps[FUN_CLSTK]="Clstk"; Keycaps[FUN_TOIJ]="Y+iX"; Keycaps[FUN_TOYX]="Split"; Keycaps[FUN_FRAC]="a b/c";	Keycaps[FUN_DMS]=QString(L'°')+"'\"";	Keycaps[FUN_TODMS]="-> "+QString(L'°')+"'\"";
	Keycaps[FUN_ROUND]="Round";
	Keycaps[FUN_XY]="x<>y";
	Keycaps[FUN_RUP]="Rup"+QString(L'↑');	Keycaps[FUN_RDN]="Rdn"+QString(L'↓');	Keycaps[FUN_PERC]="%";
	Keycaps[FUN_CONV]="Conv"; Keycaps[FUN_QCONV]="Quick conv"; Keycaps[FUN_CONS]="Const";	Keycaps[FUN_QCONS]="Quick const";	Keycaps[FUN_RAND]="Random";	Keycaps[FUN_TOFRAC]="Frac";
	Keycaps[FUN_TOINT]="Int";	Keycaps[FUN_CART]="Cart";	Keycaps[FUN_POLAR]="Pol";	Keycaps[FUN_EXP]="e^x";
	Keycaps[FUN_TEN]="10^x"; Keycaps[FUN_ROOT]="x"+QString(L'√')+"y"; Keycaps[FUN_SQRT]=QString(L'√')+"x";	Keycaps[FUN_QROOT]=QString(L'³')+QString(L'√')+"x";
	Keycaps[FUN_LN]="Ln";	Keycaps[FUN_LOG]="Log10";	Keycaps[FUN_POW]="y^x";	Keycaps[FUN_SQR]="x"+QString(L'²');	Keycaps[FUN_CUBE]="x"+QString(L'³');	Keycaps[FUN_ABS]="|x|";
	Keycaps[FUN_SIN]="sin"; Keycaps[FUN_COS]="cos"; Keycaps[FUN_TAN]="tan";	Keycaps[FUN_ASIN]="asin";	Keycaps[FUN_ACOS]="acos";	Keycaps[FUN_ATAN]="atan";
	Keycaps[FUN_SINH]="sinh"; Keycaps[FUN_COSH]="cosh"; Keycaps[FUN_TANH]="tanh";	Keycaps[FUN_ARSINH]="arsinh";	Keycaps[FUN_ARCOSH]="arcosh";	Keycaps[FUN_ARTANH]="artanh";
	Keycaps[FUN_RCP]="1/x";	Keycaps[FUN_PI]=QString(L'π');	Keycaps[FUN_FACT]="!";
	Keycaps[FUN_IMAGPART]=","; Keycaps[FUN_REIM]="Re<>Im";	Keycaps[FUN_CONJ]="Conj";
	Keycaps[FUN_AND]="And"; Keycaps[FUN_OR]="Or"; Keycaps[FUN_XOR]="Xor";	Keycaps[FUN_NOT]="Not";	Keycaps[FUN_SHL]="Sh <";	Keycaps[FUN_SHR]="Sh >";	Keycaps[FUN_ASHL]="Ash <";
	Keycaps[FUN_ASHR]="Ash >";	Keycaps[FUN_ROTL]="Rot <";	Keycaps[FUN_ROTR]="Rot >";
	Keycaps[FUN_STO0]="Sto 0"; Keycaps[FUN_STO1]="Sto 1"; Keycaps[FUN_STO2]="Sto 2";	Keycaps[FUN_STO3]="Sto 3";	Keycaps[FUN_STO4]="Sto 4";	Keycaps[FUN_STO5]="Sto 5";	Keycaps[FUN_STO6]="Sto 6";
	Keycaps[FUN_STO7]="Sto 7";	Keycaps[FUN_STO8]="Sto 8";	Keycaps[FUN_STO9]="Sto 9";
	Keycaps[FUN_RCL0]="Rcl 0"; Keycaps[FUN_RCL1]="Rcl 1"; Keycaps[FUN_RCL2]="Rcl 2";	Keycaps[FUN_RCL3]="Rcl 3";	Keycaps[FUN_RCL4]="Rcl 4";	Keycaps[FUN_RCL5]="Rcl 5";	Keycaps[FUN_RCL6]="Rcl 6";
	Keycaps[FUN_RCL7]="Rcl 7";	Keycaps[FUN_RCL8]="Rcl 8";	Keycaps[FUN_RCL9]="Rcl 9";
	Keycaps[FUN_CLR0]="Clr 0"; Keycaps[FUN_CLR1]="Clr 1"; Keycaps[FUN_CLR2]="Clr 2";	Keycaps[FUN_CLR3]="Clr 3";	Keycaps[FUN_CLR4]="Clr 4";	Keycaps[FUN_CLR5]="Clr 5";	Keycaps[FUN_CLR6]="Clr 6";
	Keycaps[FUN_CLR7]="Clr 7";	Keycaps[FUN_CLR8]="Clr 8";	Keycaps[FUN_CLR9]="Clr 9";
	Keycaps[FUN_FIX]="Fix"; Keycaps[FUN_SCI]="Sci"; Keycaps[FUN_ENG]="Eng";	Keycaps[FUN_FLOAT]="Float";
	Keycaps[FUN_FIX0]="Fix 0"; Keycaps[FUN_FIX1]="Fix 1"; Keycaps[FUN_FIX2]="Fix 2";	Keycaps[FUN_FIX3]="Fix 3";	Keycaps[FUN_FIX4]="Fix 4";	Keycaps[FUN_FIX5]="Fix 5"; Keycaps[FUN_FIX6]="Fix 6";
	Keycaps[FUN_FIX7]="Fix 7"; Keycaps[FUN_FIX8]="Fix 8"; Keycaps[FUN_FIX9]="Fix 9";
	Keycaps[FUN_DEG]="Deg"; Keycaps[FUN_RAD]="Rad"; Keycaps[FUN_DECIMAL]="Decimal"; Keycaps[FUN_COMPLEX]="Complex"; Keycaps[FUN_HEX]="Hex"; Keycaps[FUN_OCTAL]="Octal"; Keycaps[FUN_BINARY]="Bin";
	Keycaps[FUN_64BIT]="64 bit"; Keycaps[FUN_32BIT]="32 bit"; Keycaps[FUN_16BIT]="16 bit"; Keycaps[FUN_8BIT]="8 bit";
	Keycaps[FUN_CONFIG]="Config"; Keycaps[FUN_EXIT]="Exit"; Keycaps[FUN_HELP]="Help"; Keycaps[FUN_KEYHELP]="Keyhelp"; Keycaps[FUN_COPY]="Copy"; Keycaps[FUN_PASTE]="Paste"; Keycaps[FUN_POPUP]="Popup";
	Keycaps[FUN_PROG1]="Prog 1"; Keycaps[FUN_PROG2]="Prog 2"; Keycaps[FUN_PROG3]="Prog 3"; Keycaps[FUN_PROG4]="Prog 4"; Keycaps[FUN_PROG5]="Prog 5"; Keycaps[FUN_PROG6]="Prog 6";
	Keycaps[FUN_PROG7]="Prog 7"; Keycaps[FUN_PROG8]="Prog 8"; Keycaps[FUN_PROG9]="Prog 9"; Keycaps[FUN_PROG10]="Prog 10"; Keycaps[FUN_PROG11]="Prog 11"; Keycaps[FUN_PROG12]="Prog 12";
	Keycaps[FUN_EDITPR]="Edit prog"; Keycaps[FUN_CLRMEM]="ClrMem"; Keycaps[FUN_WLUP]="WL up"; Keycaps[FUN_WLDOWN]="WL down";
	Keycaps[FUN_FIXP]="Fix +"; Keycaps[FUN_FIXM]="Fix -";
	Keycaps[FUN_CART]="Cartesian";
	Keycaps[FUN_POLAR]="Polar";
	//Tooltips,
	Keytips[FUN_NONE]="nothing!";
	Keytips[FUN_ABOUT]="Information about XCALC and the GNU Public License ctrl-a]";
	Keytips[FUN_0]="Number 0"; Keytips[FUN_1]="Number 1"; Keytips[FUN_2]="Number 2"; Keytips[FUN_3]="Number 3";
	Keytips[FUN_4]="Number 4"; Keytips[FUN_5]="Number 5"; Keytips[FUN_6]="Number 6"; Keytips[FUN_7]="Number 7";
	Keytips[FUN_8]="Number 8"; Keytips[FUN_9]="Number 9"; Keytips[FUN_A]="Number A"; Keytips[FUN_B]="Number B";
	Keytips[FUN_C]="Number C"; Keytips[FUN_D]="Number D"; Keytips[FUN_E]="Number E"; Keytips[FUN_F]="Number F";
	Keytips[FUN_POINT]="Decimal point [.]";
	Keytips[FUN_CHS]="Change sign [m]"; Keytips[FUN_EEX]="Enter exponent [e]"; Keytips[FUN_BACK]="Backspace/Clx [Backspace]";
	Keytips[FUN_ENTER]="Enter [Return]"; Keytips[FUN_ADD]="Addition [+]"; Keytips[FUN_SUB]="Subtraction [-]";
	Keytips[FUN_MUL]="Multiplication [*]"; Keytips[FUN_DIV]="Division [/]";
	Keytips[FUN_DIVF]="Integer division [shift-D]"; Keytips[FUN_MOD]="Modulo [%]";
	Keytips[FUN_MODF]="Modulo [shift-M]"; Keytips[FUN_LASTX]="Lastx [l]"; Keytips[FUN_UNDO]="Undo last operation [ctrl-Z]";  Keytips[FUN_REDO]="Redo last undone operation [ctrl-Y]";
	Keytips[FUN_CLSTK]="Clear stack [shift-Z]"; Keytips[FUN_TOIJ]="Combine (y,x) to complex [i]"; Keytips[FUN_TOYX]="Split complex to (y,x) [shift-I]";
	Keytips[FUN_FRAC]="Enter fractions [<spc>,f]"; Keytips[FUN_DMS]="Enter DMS (degrees/minutes/seconds) [']";
	Keytips[FUN_TODMS]="Convert to DMS [ctrl-']";
	Keytips[FUN_ROUND]="Remove fraction or DMS, or use displayed value [$,shift-R]";
	Keytips[FUN_XY]="swap x and y [<,>]";
	Keytips[FUN_RUP]="Roll Up [Up]"; Keytips[FUN_RDN]="Roll down [Down]"; Keytips[FUN_PERC]="Percent [%]";
	Keytips[FUN_CONV]="Convert [v]"; Keytips[FUN_QCONV]="Quick convert [shift-V]"; Keytips[FUN_CONS]="Constants [k]"; Keytips[FUN_QCONS]="Quick constant [shift-K]"; Keytips[FUN_RAND]="Random [?]";
	Keytips[FUN_TOFRAC]="Fractional part [ctrl-f]"; Keytips[FUN_TOINT]="Integer part [ctrl-i]";
	Keytips[FUN_CART]="Cartesian [:]"; Keytips[FUN_POLAR]="Polar [;]"; Keytips[FUN_EXP]="Exponent [shift-E]";
	Keytips[FUN_TEN]="10th power 10^x [shift-T]"; Keytips[FUN_ROOT]="xth root of y, y^(1/x) [r]"; Keytips[FUN_SQRT]="Square root [q]"; Keytips[FUN_QROOT]="Cube root [u]";
	Keytips[FUN_LN]="Natural logarithm (ln) [shift-L]"; Keytips[FUN_LOG]="10th logaritm [shift-G]"; Keytips[FUN_POW]="y to xth power [o,^]";
	Keytips[FUN_SQR]="Square [shift-Q]"; Keytips[FUN_CUBE]="Cube [shift-U]"; Keytips[FUN_ABS]="Absolute value [|]";
	Keytips[FUN_SIN]="sine [s]"; Keytips[FUN_COS]="cosine [c]"; Keytips[FUN_TAN]="tangent [t]"; Keytips[FUN_ASIN]="inverse sine [shift-S]"; Keytips[FUN_ACOS]="inverse cosine [shift-C]"; Keytips[FUN_ATAN]="inverse tangent [shift-T]";
	Keytips[FUN_SINH]="hyperbolic sine [meta-alt-s]"; Keytips[FUN_COSH]="hyperbolic cosine [meta-alt-c]"; Keytips[FUN_TANH]="hyperbolic tangent [meta-alt-t]"; Keytips[FUN_ARSINH]="inverse hyperbolic sine [meta-shift-S]"; Keytips[FUN_ARCOSH]="inverse hyperbolic cosine  [meta-shift-C]"; Keytips[FUN_ARTANH]="inverse hyperbolic tangent [meta-shift-T]";
	Keytips[FUN_RCP]="1/x [\\]"; Keytips[FUN_PI]="Pi [p]"; Keytips[FUN_FACT]="Factorial [!]";
	Keytips[FUN_IMAGPART]="Switch real/imag input [,]"; Keytips[FUN_REIM]="Swap real and imaginary part [shift-J]"; Keytips[FUN_CONJ]="Conjugate [']";
	Keytips[FUN_AND]="And [a,&]"; Keytips[FUN_OR]="Or [o,|]"; Keytips[FUN_XOR]="Xor [x,^]"; Keytips[FUN_NOT]="Not [n,!]";
	Keytips[FUN_SHL]="Shift left [shift-left]"; Keytips[FUN_SHR]="Shift right [shift-right]"; Keytips[FUN_ASHL]="Arithmetic shift left [alt-shift-left]";
	Keytips[FUN_ASHR]="Arithmetic shift right [alt-shift-right]"; Keytips[FUN_ROTL]="Rotate left [left]"; Keytips[FUN_ROTR]="Rotate right [right]";
	Keytips[FUN_STO0]="Store to register 0 [ctrl-0]"; Keytips[FUN_STO1]="Store 1 [ctrl-1]"; Keytips[FUN_STO2]="Store 2 [ctrl-2]"; Keytips[FUN_STO3]="Store 3 [ctrl-3]"; Keytips[FUN_STO4]="Store 4 [ctrl-4]";
	Keytips[FUN_STO5]="Store 5 [ctrl-5]"; Keytips[FUN_STO6]="Store 6 [ctrl-6]"; Keytips[FUN_STO7]="Store 7 [ctrl-7]"; Keytips[FUN_STO8]="Store 8 [ctrl-8]"; Keytips[FUN_STO9]="Store 9 [ctrl-9]";
	Keytips[FUN_RCL0]="Recall register 0 [ctrl-shift-0]"; Keytips[FUN_RCL1]="Recall 1 [ctrl-shift-1]"; Keytips[FUN_RCL2]="Recall 2 [ctrl-shift-2]"; Keytips[FUN_RCL3]="Recall 3 [ctrl-shift-3]"; Keytips[FUN_RCL4]="Recall 4 [ctrl-shift-4]";
	Keytips[FUN_RCL5]="Recall 5 [ctrl-shift-5]"; Keytips[FUN_RCL6]="Recall 6 [ctrl-shift-6]"; Keytips[FUN_RCL7]="Recall 7 [ctrl-shift-7]"; Keytips[FUN_RCL8]="Recall 8 [ctrl-shift-8]"; Keytips[FUN_RCL9]="Recall 9 [ctrl-shift-9]";
	Keytips[FUN_CLR0]="Clear register 0 [ctrl-alt-0]"; Keytips[FUN_CLR1]="Clear 1 [ctrl-alt-1]"; Keytips[FUN_CLR2]="Clear 2 [ctrl-alt-2]"; Keytips[FUN_CLR3]="Clear 3 [ctrl-alt-3]"; Keytips[FUN_CLR4]="Clear 4 [ctrl-alt-4]";
	Keytips[FUN_CLR5]="Clear 5 [ctrl-alt-5]"; Keytips[FUN_CLR6]="Clear 6 [ctrl-alt-6]"; Keytips[FUN_CLR7]="Clear 7 [ctrl-alt-7]"; Keytips[FUN_CLR8]="Clear 8 [ctrl-alt-8]"; Keytips[FUN_CLR9]="Clear 9 [ctrl-alt-9]";
	Keytips[FUN_FIX]="Fix display [alt-i]"; Keytips[FUN_SCI]="Scientific display [alt-s]"; Keytips[FUN_ENG]="Engineering display [alt-e]"; Keytips[FUN_FLOAT]="Float display [alt-f]";
	Keytips[FUN_FIX0]="Fix 0 [alt-0]"; Keytips[FUN_FIX1]="Fix 1 [alt-1]"; Keytips[FUN_FIX2]="Fix 2 [alt-2]"; Keytips[FUN_FIX3]="Fix 3 [alt-3]"; Keytips[FUN_FIX4]="Fix 4 [alt-4]";
	Keytips[FUN_FIX5]="Fix 5 [alt-5]"; Keytips[FUN_FIX6]="Fix 6 [alt-6]"; Keytips[FUN_FIX7]="Fix 7 [alt-7]"; Keytips[FUN_FIX8]="Fix 8 [alt-8]"; Keytips[FUN_FIX9]="Fix 9 [alt-9]";
	Keytips[FUN_DEG]="Degrees [alt-g]"; Keytips[FUN_RAD]="Radians [alt-r]"; Keytips[FUN_DECIMAL]="Decimal [alt-d]"; Keytips[FUN_COMPLEX]="Complex [alt-c]"; Keytips[FUN_HEX]="Hexadecimal [alt-h]"; Keytips[FUN_OCTAL]="Octal [alt-o]"; Keytips[FUN_BINARY]="Binary [alt-b]";
	Keytips[FUN_64BIT]="64 bit word length [f12]"; Keytips[FUN_32BIT]="32 bit word length [f11]"; Keytips[FUN_16BIT]="16 bit word length [f10]"; Keytips[FUN_8BIT]="8 bit word length [f9]";
	Keytips[FUN_CONFIG]="Configure [alt-n]"; Keytips[FUN_EXIT]="Exit, quit, close, [alt-x]"; Keytips[FUN_HELP]="xcalc online help [f1]"; Keytips[FUN_KEYHELP]="Help for current keyboard [ctrl-f1]";
	Keytips[FUN_COPY]="Copy to clipboard (not implemented)"; Keytips[FUN_PASTE]="Paste from clipboard (not implemented)"; Keytips[FUN_POPUP]="Memory popup (not implemented)";
	Keytips[FUN_PROG1]="Prog 1"; Keytips[FUN_PROG2]="Prog 2"; Keytips[FUN_PROG3]="Prog 3"; Keytips[FUN_PROG4]="Prog 4"; Keytips[FUN_PROG5]="Prog 5"; Keytips[FUN_PROG6]="Prog 6";
	Keytips[FUN_PROG7]="Prog 7"; Keytips[FUN_PROG8]="Prog 8"; Keytips[FUN_PROG9]="Prog 9"; Keytips[FUN_PROG10]="Prog 10"; Keytips[FUN_PROG11]="Prog 11"; Keytips[FUN_PROG12]="Prog 12";
	Keytips[FUN_EDITPR]="Edit prog"; Keytips[FUN_CLRMEM]="Clear all memory registers [ctrl-alt-m]";
	Keytips[FUN_WLUP]="Increment word length (integer modes) [alt-shift-W]"; Keytips[FUN_WLDOWN]="Decrement word length (integer modes) [alt-w]";
	Keytips[FUN_FIXP]="More decimals [right]"; Keytips[FUN_FIXM]="Less decimals [left]";
	Keytips[FUN_CART]="Cartesian (or rectangular) coordinates (x,y) [:]";
	Keytips[FUN_POLAR]="Polar coordinates (radius,angle) [;]";
}

// clearinput resets all number input
void XCALCWindow::clearinput()
{
	m_STR.clear();
	m_STR2.clear();
	m_iState=0;
	m_iState2=-4;
	m_inputreg.clear();
}

void XCALCWindow::SetupUi()
{
	// Check number of buttons
	if (DB.size()!=NBUTTONS) QMessageBox::critical(this,"Error","Decimalbuttons wrong size");
	if (CB.size()!=NBUTTONS) QMessageBox::critical(this,"Error","Complexbuttons wrong size");
	if (HB.size()!=NBUTTONS) QMessageBox::critical(this,"Error","Hexbuttons wrong size");
	if (OB.size()!=NBUTTONS) QMessageBox::critical(this,"Error","Octalbuttons wrong size");
	if (BB.size()!=NBUTTONS) QMessageBox::critical(this,"Error","Binarybuttons wrong size");
	DeleteUi();

	m_iMaxDigits=MaxInputDigits(xcalc_radix,xcalc_wordLength);
	idList *buts, *acts;
	switch (xcalc_radix) {
	case rtDECIMAL: buts=&DB; acts=&DA; break;
	case rtCOMPLEX: buts=&CB; acts=&CA; break;
	case rtHEX: buts=&HB; acts=&HA; break;
	case rtOCTAL: buts=&OB; acts=&OA; break;
	case rtBINARY: buts=&BB; acts=&BA; break;
	default: buts=0; acts=0; break;
	}

	QWidget *ctr = new QWidget(this);
	setCentralWidget(ctr);
	m_stkdispsize=STKDISPSIZE;
	m_butrows=BUTROWS;
	m_butcols=BUTCOLS;
	QFont *numfont = new QFont("Monospace",18,2);
	QGridLayout *grid = new QGridLayout(ctr);
	m_lstk = new QLabel*[m_stkdispsize];
	for (int w=iXreg;w<m_stkdispsize;w++) {
		QLabel *l=new QLabel(QString("%1.0000").arg(w),ctr);
		l->setFont(*numfont);
		grid->addWidget(l,m_stkdispsize-1-w,0,1,m_butcols+1);
		m_lstk[w]=l;
	}
	m_mems=new QLabel*[m_butrows];
	m_but=new QPushButton**[m_butrows];
	for (int i=0;i<m_butrows;i++) {
		m_mems[i]=new QLabel(QString::number(i));
		grid->addWidget(m_mems[i],m_stkdispsize+i,0,1,1);
		m_but[i]=new QPushButton*[m_butcols];
		for (int j=0;j<m_butcols;j++) {
			int k=i*m_butcols+j;
			funid kid = buts->value(k);
			if (acts->contains(kid)) {
				QPushButton *but=new QPushButton(Keycaps.value(kid),ctr);
				if (kid==FUN_NONE) but->hide();
				int colspan = 1;
				if (kid==FUN_UNDO || kid==FUN_REDO || kid==FUN_ENTER) {
					colspan = 2;
				}
				if (xcalc_radix==rtCOMPLEX && (kid==FUN_DEG || kid==FUN_RAD)) {
					but->setEnabled(false); // this won't disable the shortcut, though
				}
				but->setFocusPolicy(Qt::NoFocus);
				but->setToolTip(Keytips.value(kid));
				grid->addWidget(but,m_stkdispsize+i,j+1,1,colspan);
				connect(but,SIGNAL(clicked()),m_idMapper,SLOT(map()));
				m_idMapper->setMapping(but,kid);
				m_but[i][j]=but;
			}
		}
	}
	m_lmsg = new QLabel("",ctr);
	m_lWL = new QLabel("",ctr);
	m_lfix = new QLabel("?",ctr);
	m_lradix = new QLabel("?",ctr);
	m_lWL->setAlignment(Qt::AlignRight);
	m_lfix->setAlignment(Qt::AlignRight);
	m_lradix->setAlignment(Qt::AlignRight);
	grid->addWidget(m_lmsg,m_stkdispsize+m_butrows,0,1,m_butcols-3);
	grid->addWidget(m_lWL,m_stkdispsize+m_butrows,m_butcols-3,1,1);
	grid->addWidget(m_lfix,m_stkdispsize+m_butrows,m_butcols-2,1,1);
	grid->addWidget(m_lradix,m_stkdispsize+m_butrows,m_butcols-1,1,2);

	centralWidget()->setLayout(grid);
}

void XCALCWindow::DeleteUi()
{
	// Central widget owns everything except actions
	if (centralWidget()) {
		delete centralWidget();
		setCentralWidget(0);
	}
	int i;
	if (m_lstk) {
		delete[] m_lstk;
		m_lstk=0;
		m_stkdispsize=0;
	}
	if (m_but) {
		for (i=0;i<m_butrows;i++) {
			delete [] m_but[i];
		}
		delete[] m_but;
		m_but=0;
		m_butrows=m_butcols=0;
	}
}

QString fixname(FixType f) {
	switch (f) {
	case ftFIX: return "Fix";
	case ftSCI: return "Sci";
	case ftENG: return "Eng";
	default: return "42";
	}
}

QString radixname(uint m) {
	switch (m) {
	case rtDECIMAL: return "Decimal";
	case rtCOMPLEX: return "Complex";
	case rtHEX: return "Hex";
	case rtOCTAL: return "Octal";
	case rtBINARY: return "Binary";
	default: return "42";
	}
}

QString wlname(WordLength b) {
	switch (b) {
	case wl8BIT: return "8 bit";
	case wl16BIT: return "16 bit";
	case wl32BIT: return "32 bit";
	case wl64BIT: return "64 bit";
	default: return "42 bit";
	}
}

QPushButton *XCALCWindow::FindButton(funid kid)
{
	idList *buts = 0;
	switch (xcalc_radix) {
	case rtDECIMAL: buts=&DB; break;
	case rtCOMPLEX: buts=&CB; break;
	case rtHEX: buts=&HB; break;
	case rtOCTAL: buts=&OB; break;
	case rtBINARY: buts=&BB; break;
	}
	int ix = buts->indexOf(kid);
	if (ix>=0) {
		int r=ix/m_butcols;
		int c=ix%m_butcols;
		return m_but[r][c];
	}
	return 0;
}

void XCALCWindow::newradixslot()
{
	SetupUi();
	clearinput();
	updateslot();
}

void XCALCWindow::mailclickedslot()
{
	SendMail(EMAIL,QString("XCALC %1 config").arg(XCALCVERSION));
}

void XCALCWindow::updateslot()
{
	int reg0 = iXreg;
	// ask engine for values to display, but do iXreg here
	if (!m_STR.isEmpty() || !m_STR2.isEmpty()) {
		if (m_iState<0)
			m_lstk[iXreg]->setText(m_STR2+"  "+m_STR+"i");
		else if (m_STR2.isEmpty())
			m_lstk[iXreg]->setText(m_STR);
		else
			m_lstk[iXreg]->setText(m_STR+" "+m_STR2+"i");
		reg0 = iYreg;
	}
	for (int i=reg0;i<STKS;i++) {
		m_lstk[i]->setText(MakeString(i));
	}
	QString fFix,sFix,sWL;
	if (maxval()==10) {
		fFix = m_xengine->fix()<0?"F":QString::number(m_xengine->fix(),10);
		sFix = QString("%1:%2").arg(fixname(m_fixtype)).arg(fFix);
		sWL = QString("(%1)").arg(wlname(Register::wordLength()));
	}
	else
		sFix = wlname(Register::wordLength());
	QString sradix = QString("Mode:%1").arg(radixname(xcalc_radix));

	messagecode msg = m_xengine->amessage();
	QString smsg;
	if (msg==__UNDONE)
		smsg = QString("Undone %1").arg(fname.value(m_xengine->lastundone()));
	else if (msg==__REDONE)
		smsg = QString("Redone %1").arg(fname.value(m_xengine->lastundone()));
	else if (msg==__NOUNDO)
		smsg = "No more undo information";
	else if (msg==__NOREDO)
		smsg = "No more redo information";
	else
		smsg = m_xengine->qmessage();
	m_lfix->setText(sFix);
	m_lWL->setText(sWL);
	m_lradix->setText(sradix);
	m_lmsg->setText(smsg);
	for (int i=0;i<10;i++) {
		QLabel *l = m_mems[i];
		bool zero = m_xengine->regr(100+i).iszero();
		l->setStyleSheet(zero?"":"color:red");
		l->setToolTip(QString("Mem%1: %2").arg((i)).arg(quicknum(m_xengine->regr(100+i))));
	}
	QPushButton *dbut=FindButton(FUN_DECIMAL);
	QPushButton *cbut=FindButton(FUN_COMPLEX);
	QPushButton *hbut=FindButton(FUN_HEX);
	QPushButton *obut=FindButton(FUN_OCTAL);
	QPushButton *bbut=FindButton(FUN_BINARY);
	if (dbut) dbut->setStyleSheet(xcalc_radix==rtDECIMAL?"color:red":"");
	if (cbut) cbut->setStyleSheet(xcalc_radix==rtCOMPLEX?"color:red":"");
	if (hbut) hbut->setStyleSheet(xcalc_radix==rtHEX?"color:red":"");
	if (obut) obut->setStyleSheet(xcalc_radix==rtOCTAL?"color:red":"");
	if (bbut) bbut->setStyleSheet(xcalc_radix==rtBINARY?"color:red":"");
	QPushButton *Fibut=FindButton(FUN_FIX);
	QPushButton *Scbut=FindButton(FUN_SCI);
	QPushButton *Enbut=FindButton(FUN_ENG);
	QPushButton *F0but=FindButton(FUN_FIX0);
	QPushButton *F2but=FindButton(FUN_FIX2);
	QPushButton *F4but=FindButton(FUN_FIX4);
	QPushButton *Flbut=FindButton(FUN_FLOAT);
	if (Fibut) Fibut->setStyleSheet(m_xengine->fixtype()==ftFIX?"color:red":"");
	if (Scbut) Scbut->setStyleSheet(m_xengine->fixtype()==ftSCI?"color:red":"");
	if (Enbut) Enbut->setStyleSheet(m_xengine->fixtype()==ftENG?"color:red":"");
	if (F0but) F0but->setStyleSheet(m_xengine->fix()==0?"color:red":"");
	if (F2but) F2but->setStyleSheet(m_xengine->fix()==2?"color:red":"");
	if (F4but) F4but->setStyleSheet(m_xengine->fix()==4?"color:red":"");
	if (Flbut) Flbut->setStyleSheet(m_xengine->fix()==FLOATDISP?"color:red":"");
	QPushButton *b8but=FindButton(FUN_8BIT);
	QPushButton *b16but=FindButton(FUN_16BIT);
	QPushButton *b32but=FindButton(FUN_32BIT);
	QPushButton *b64but=FindButton(FUN_64BIT);
	if (b8but) b8but->setStyleSheet(xcalc_wordLength==wl8BIT?"color:red":"");
	if (b16but) b16but->setStyleSheet(xcalc_wordLength==wl16BIT?"color:red":"");
	if (b32but) b32but->setStyleSheet(xcalc_wordLength==wl32BIT?"color:red":"");
	if (b64but) b64but->setStyleSheet(xcalc_wordLength==wl64BIT?"color:red":"");
	QPushButton *bdegbut=FindButton(FUN_DEG);
	QPushButton *bradbut=FindButton(FUN_RAD);
	if (bdegbut) bdegbut->setStyleSheet(m_xengine->angtype()==atDEGREE?"color:red":"");
	if (bradbut) bradbut->setStyleSheet(m_xengine->angtype()==atRADIAN?"color:red":"");
	QString nextundo = "No undo";
	QString nextredo = "No redo";
	funid nu = m_xengine->nextundo();
	funid nr = m_xengine->nextredo();
	if (nu!=FUN_NONE) nextundo = "Undo "+fname.value(nu);
	if (nr!=FUN_NONE) nextredo = "Redo "+fname.value(nr);
	QPushButton *bundo=FindButton(FUN_UNDO);
	if (bundo) bundo->setText(nextundo);
	QPushButton *bredo=FindButton(FUN_REDO);
	if (bredo) bredo->setText(nextredo);
}

//-----------------------------------------------------------------------
// set(c) sets member STR to c (only) for keyboard input
//-----------------------------------------------------------------------
void XCALCWindow::set(wchar c)
{
	m_STR = c;
}

//-----------------------------------------------------------------------
// app(c) appends c to STR for keyboard input
//-----------------------------------------------------------------------
void XCALCWindow::app(wchar c)
{
	m_STR += c;
}

//-----------------------------------------------------------------------
// insbeg(c) inserts c left in STR
//-----------------------------------------------------------------------
void XCALCWindow::insbeg(wchar c)
{
	m_STR = c+m_STR;
}

//-----------------------------------------------------------------------
// delbeg() clears first char in STR
//-----------------------------------------------------------------------
void XCALCWindow::delbeg(void)
{
	m_STR = m_STR.mid(1);
}

uint XCALCWindow::maxval()
{
	switch (xcalc_radix) {
	case rtDECIMAL: return 10;
	case rtCOMPLEX: return 10;
	case rtHEX: return 16;
	case rtOCTAL: return 8;
	case rtBINARY: return 2;
	default: return 42;
	}
}

//void XCALCWindow::evalnew() {
//	if (m_xengine->sl()) m_xengine->lift();
//	//m_xengine->setsl(true);
//	Evaluate();
//}

void XCALCWindow::swap(void) {SWAP(m_STR,m_STR2); SWAP(m_iState,m_iState2); xcalc_radix=rtCOMPLEX; updateslot();} // must have complex radix to swap

void XCALCWindow::translatekid(funid &kid) { // kid translations to allow some binary keystrokes for decimal/complex, differences dec/cplx and duplicate keystrokes
	// note: ^ doesn't seem to work with norwegian keyboard! Luckily we have alternatives as pow (o) or xor (x)
	// also, disallows setting complex mode if input string contains a '/'
	switch (m_xengine->radixtype()) {
	case rtDECIMAL:
		// Decimal only
		if (kid==FUN_CONJ) kid=FUN_DMS;
		else if (kid==FUN_F) kid=FUN_FRAC;
		// fall through
	case rtCOMPLEX:
		// Decimal and complex
		if (m_STR.indexOf('/')>=0 && (kid==FUN_COMPLEX || kid==FUN_IMAGPART)) kid = FUN_NONE;
		if (kid==FUN_MOD) kid=FUN_PERC;
		else if (kid==FUN_A) kid=FUN_ABS;
		else if (kid==FUN_C) kid=FUN_COS;
		else if (kid==FUN_E) kid=FUN_EEX;
		else if (kid==FUN_XOR2) kid=FUN_POW;
		else if (kid==FUN_OR) kid=FUN_ABS;
		else if (kid==FUN_ROTL) kid=FUN_FIXM;
		else if (kid==FUN_ROTR) kid=FUN_FIXP;
		else if (kid==FUN_ROUND2) kid=FUN_ROUND;
		break;
	default:
		// Binary modes
		if (kid==FUN_NOT2) kid=FUN_NOT;
		else if (kid==FUN_POW) kid=FUN_OR;
		else if (kid==FUN_FACT) kid=FUN_NOT;
		else if (kid==FUN_OR2) kid=FUN_OR;
		break;
	}
}

bool XCALCWindow:: kidallowed(funid kid) {
	switch (m_xengine->radixtype()) {
	case rtDECIMAL: return DA.contains(kid);
	case rtCOMPLEX: return CA.contains(kid);
	case rtHEX: return HA.contains(kid);
	case rtOCTAL: return OA.contains(kid);
	case rtBINARY: return BA.contains(kid);
	}
	return false;
}

void XCALCWindow::funcslot(int id) // signalmapper can only send ints
{
	func((funid)id);
}

void XCALCWindow::func(funid kid)
{
	/* Keypresses come here first, as key id. Function names found in map fname.
	check input and do action:
	- is the key allowed? (listed in BA, CA etc.)
	- ambiguous key? Translate in translatekid
	- translate funid (kid) to character for number input
	- translate BACK to CLX unless handled here
	- UNDO clears STR/STR2 if iState!=0, otherwise sent to engine
	- manage iState:
	A negative iState (-1,-2,-3) means we are keying in the imag part.
	iState=-4: special mode similar to 0 for complex part, aka -0
	default: iState2 -4
	swap = STR(2),iState(2)
	iState=0: normal operation, may change to any other mode during input (val=numeric value of key, 0 - r-1, or 42)
	iState=1s: keying in integer part
	iState=2s: keying in fractional part (after dec.point)
	iState=3s: keying in exponent part (after 'e')
	iState=-4 , aka -0:
	iState=5: keying in first fractional part (x in a/x)
	iState=6: keying in second fractional part (x in a/b/x)
	iState=7: keying in minutes (x in a°x)
	iState=8: keying in integer part of seconds (x in a°b'x)
	iState=9: keying in fractional part of seconds (x in a°b's.x)

	If the state input found no valid input key, the X register is updated if input
	has been given previously, iState is set to 0, and the key is sent
	to the engina as a function key (such as +, -, c for cosine etc...)
	- local keys are these used to fill/maintain STR/STR2 or help/config. Do not send to engine
	- if non-local key, Inkey inputreg, then send func.
	- manage STR as queue for input number, swap STR/STR2 when inputting imag part.
	*/
	// First, disable FUN_NONE completely
	if (kid==FUN_NONE) return;
	bool allowed = kidallowed(kid);// checks validity for radix - fixes '8' for octal
	translatekid(kid); // modifies some kids to decimal/complex - fixes '%','c','e','f','^','|',left,right,'
	//qcout << QString("XCALCWindow::func = '%1' (%2)").arg(fname.value(kid)).arg(kid) << endl;
	allowed |= kidallowed(kid); // allows translated kid as well
	if (!allowed) return;

	bool local = local_funs.contains(kid); // handle local keys here - don't send to engine
	if (kid==FUN_CHS && maxval()!=10) local=false; // CHS not lokal in bin modes
	int sSign = m_iState>=0?1:-1;
	m_xengine->ClearMsg();

	int L = m_STR.length()+m_STR2.length(); // if L, Inkey is in progress
	uint val = 42;

	wchar c = L'0'; // need which key was pressed to generate val>0
	if ((kid>=FUN_0)&&(kid<=FUN_9)) { val = kid-FUN_0; c=val+'0'; }
	if ((kid>=FUN_A)&&(kid<=FUN_F)) { val = kid-FUN_A+10; c=val-10+'A'; }

	// UNDO/REDO clears any input, and also send to engine
	if (kid==FUN_UNDO||kid==FUN_REDO) {
		clearinput();
	}
	// Enter clears iState, and sent on to engine (Inkey is sent first unless STR/STR2 are empty, in second local test)
	else if (kid==FUN_ENTER) {
		m_iState = 0; // and send to engine
	}

	// breakpoint for FUN_ROUND (debug!)
	if (kid==FUN_ROUND) {
		val = val;
	}
	// local key? Decode here (number input only), do not in general send to engine
	if (local) {
		// Keys for number input - do state-by-state check
		if (m_iState==0) { // no Inkey yet, real value
			if (kid==FUN_BACK || kid==FUN_CHS) {
				// pass BACK (as CLX) and CHS on to engine in this state (and in -4) if str2 is empty
				if (m_STR2.isEmpty()) local = false;
				if (kid==FUN_BACK) kid=FUN_CLX;
			}
			else if (val<maxval()) {
				set(c);	// set() sets STR to c alone
				Evaluate();
				m_iState=1;
			}
			else if (maxval()==10) {
				if (kid==FUN_POINT) {
					set('.');
					Evaluate();
					m_iState=2;
				}
				else if (kid==FUN_EEX) {
					set('1');
					app('e');
					app('+');
					Evaluate();
					m_iState=3;
				}
				else if (kid==FUN_DMS) {
					set('0');
					app(L'°');
					Evaluate();
					m_iState=7;
				}
				else if ((kid==FUN_IMAGPART)) {
					set('0');
					m_iState=1;
					Evaluate();
					// old radix in undoinfo now seems to have been solved by fixing Inkey()
					swap(); // forces complex radix
					Evaluate();
				}
			}
			if (local) {
				if (m_xengine->sl()) m_xengine->lift();
				m_xengine->inkey(m_inputreg,true); // should be undoable?
				updateslot(); // show new input
			}
		}
		else if (m_iState==-4) { // no Inkey yet, imag value
			if (kid==FUN_BACK || kid==FUN_CHS) {
				// pass BACK (as CLX) and CHS on to engine in this state if str2 is empty
				// 10jun12: this should be if str is empty... (swapped already)
				if (m_STR.isEmpty()) local = false;
				if (kid==FUN_BACK) kid=FUN_CLX;
			}
			else if (val<maxval()) {
				set(c);
				Evaluate();
				m_iState=-1;
			}
			else if (kid==FUN_POINT) {
				set('.');
				Evaluate();
				m_iState=-2;
			}
			else if (kid==FUN_EEX) {
				set('1');
				app('e');
				app('+');
				Evaluate();
				m_iState=-3;
			}
			else if (kid==FUN_IMAGPART) {
				Evaluate();
				swap();
				Evaluate();
			}
		}
		else if (abs(m_iState)==1) { // integer part
			bool haszero = (m_iState>0?m_inputreg.realval():m_inputreg.imagval())==0;
			if (kid==FUN_BACK) {
				if ((L==1)||((L==2)&&(m_STR[0].toAscii()=='-'))) {
					kid = FUN_CLX;
					local = false;
					m_iState = m_iState>0 ? 0:-4;
				}
				else {
					m_STR.chop(1);
					Evaluate();
				}
			}
			else if (val<maxval()) {
				if (xcalc_radix==rtDECIMAL || xcalc_radix==rtCOMPLEX) {
					if (haszero) // avoid "0x"
						set(c);
					else
						app(c);
					Evaluate();
				}
				else {
					if ((m_inputreg.ival()==0)) // avoid "0x"
						set(c);
					else
						app(c);
					Evaluate();
				}
			}
			else if (maxval()==10) {
				if (kid==FUN_POINT) {
					if (L<m_iMaxDigits) {
						if (haszero) // avoid "0."
							set('.');
						else
							app('.');
						Evaluate();
						m_iState = 2*sSign;
					}
				}
				else if (kid==FUN_EEX) {
					if (L>m_iMaxDigits-3) goto end;	// make sure there is room for at least "e+1"
					app('e');
					app('+');
					Evaluate();
					m_iState = 3*sSign;
				}
				else if (kid==FUN_CHS) {
					if (m_STR[0].toAscii()=='-') delbeg(); else insbeg('-');
					Evaluate();
				}
				else if (kid==FUN_IMAGPART) {
					Evaluate();
					swap();
					Evaluate();
				}
				else if (!haszero  && kid==FUN_FRAC) {
					app('/');
					Evaluate();
					m_iState = 5*sSign;
				}
				else if (kid==FUN_DMS) {
					app(L'°');
					Evaluate();
					m_iState = 7*sSign;
				}
			}
		}
		else if (abs(m_iState)==2) { // after "."
			if (kid==FUN_BACK) {
				if (L==1) { // had only "."
					kid = FUN_CLX;
					local = false;
					m_iState=m_iState>0 ? 0:-4;
				}
				else if (m_STR[L-1].toAscii()=='.') {
					if ((L==2)&&(m_STR[0].toAscii()=='-')) { // had "-."
						kid = FUN_CLX;
						local = false;
						m_iState=m_iState>0 ? 0:-4;
					}
					else {
						m_STR.chop(1);
					}
					Evaluate();
					m_iState=1*sSign;
				}
				else {
					m_STR.chop(1);
					Evaluate();
				}
			}
			else if (val<maxval()) {
				if (L>=m_iMaxDigits) goto end;
				app(c);
				Evaluate();
			}
			else if (kid==FUN_EEX) {
				if (L>=m_iMaxDigits-3) goto end;
				app('e');
				app('+');
				Evaluate();
				m_iState=3*sSign;
			}
			else if (kid==FUN_CHS) {
				if (m_STR[0].toAscii()=='-') delbeg(); else insbeg('-');
				Evaluate();
			}
			else if (kid==FUN_IMAGPART) {
				Evaluate();
				swap();
				Evaluate();
			}
		}
		else if (abs(m_iState)==3) { // after "e"
			bool ok;
			if (kid==FUN_BACK) {
				if ((m_STR[L-1].toAscii()=='+')||(m_STR[L-1].toAscii()=='-')) {
					m_STR.chop(2); // remove sign + 'e'
					bool Dot = m_STR.indexOf(".")>=0;
					m_iState = (Dot?2:1)*sSign;
				} else {
					m_STR.chop(1);
				}
				Evaluate();
			}
			else if (val<maxval()) {
				if (L>=m_iMaxDigits) goto end;
				app(c);
				int e = m_STR.indexOf("e");
				QString sexp = m_STR.mid(e+1);
				if (abs(sexp.toInt())>EXP_MAX) {
					m_STR.chop(1);     // Exponent too big
					goto end;
				}
				QString smant=m_STR.left(e);
				LD mant,value;
				if (mant=toLDRobust(smant,ok),!ok){
					m_STR.chop(1);
					goto end;
				}
				if (value=toLDRobust(m_STR,ok),!ok){
					m_STR.chop(1);
					goto end;
				}
				if (mant<0) mant = -mant;
				if (value<0) value = -value;
				if (value>NUM_MAX) {
					m_STR.chop(1);			// total number too big
					goto end;
				}
				if (value<NUM_MIN) {
					m_STR.chop(1);			// total number too small
					goto end;
				}
				Evaluate();
			}
			else if (kid==FUN_CHS) {
				int i=L-1;
				while ((m_STR[i].toAscii()!='+')&&(m_STR[i].toAscii()!='-')) i--;
				m_STR[i]=(char)(m_STR[i].toAscii()=='-'?'+':'-');
				Evaluate();
			}
			else if (kid==FUN_IMAGPART) {
				Evaluate();
				swap();
				Evaluate();
			}
		}
		else if (m_iState==5) { // after first '/'
			if (kid==FUN_BACK) {
				if (m_STR[L-1].toAscii()=='/') m_iState=1;
				m_STR.chop(1);
				Evaluate();
			}
			else if (val<maxval()) {
				app(c);
				Evaluate();
			}
			else if (kid==FUN_CHS) {
				if (m_STR[0]=='-') delbeg(); else insbeg('-');
				Evaluate();
			}
			else if (kid==FUN_FRAC) {
				app('/');
				Evaluate();
				m_iState=6;
			}
		}
		else if (m_iState==6) { // after second '/'
			if (kid==FUN_BACK) {
				if (m_STR[L-1].toAscii()=='/') m_iState=5;
				m_STR.chop(1);
				Evaluate();
			}
			else if (val<maxval()) {
				if (L>=m_iMaxDigits) goto end;
				app(c);
				Evaluate();
			}
			else if (kid==FUN_CHS) {
				if (m_STR[0]=='-') delbeg(); else insbeg('-');
				Evaluate();
			}
		}
		else if (m_iState==7) { // after first '
			if (kid==FUN_BACK) {
				if (m_STR[L-1].unicode()==L'°') m_iState=1;
				m_STR.chop(1);
				Evaluate();
			}
			else if (val<maxval()) {
				if (L>=m_iMaxDigits) goto end;
				app(c);
				Evaluate();
			}
			else if (kid==FUN_CHS) {
				if (m_STR[0].toAscii()=='-') delbeg(); else insbeg('-');
				Evaluate();
			}
			else if (kid==FUN_DMS) {
				if (m_STR[L-1].unicode()==L'°') app('0');
				app('\'');
				Evaluate();
				m_iState=8;
			}
		}
		else if (m_iState==8) { // after second '
			if (kid==FUN_BACK) {
				if (m_STR[L-1].toAscii()=='\'') m_iState=7;
				m_STR.chop(1);
				Evaluate();
			}
			else if (kid==FUN_POINT) {
				if (L>=m_iMaxDigits) goto end;
				if (m_STR[L-1].toAscii()=='\'') app('0');
				app('.');
				Evaluate();
				m_iState=9;
			}
			else if (val<maxval()) {
				if (L>=m_iMaxDigits) goto end;
				app(c);
				Evaluate();
			}
			else if (kid==FUN_CHS) {
				if (m_STR[0]=='-') delbeg(); else insbeg('-');
				Evaluate();
			}
		}
		else if (m_iState==9) { // after second ' and .
			if (kid==FUN_BACK) {
				if (m_STR[L-1].toAscii()=='.') m_iState=8;
				m_STR.chop(1);
				Evaluate();
			}
			else if (val<maxval()) {
				if (L>=m_iMaxDigits) goto end;
				app(c);
				Evaluate();
			}
			else if (kid==FUN_CHS) {
				if (m_STR[0]=='-') delbeg(); else insbeg('-');
				Evaluate();
			}
		} // end if iState...
	} // end if local
	// Second local test - some keys are local but change the local status above (maybe based on state)
	if (!local) {
		// nothing fit so far (except possibly enter,back,chs,fix). Must be a function key. Check if STR/STR2 has something and send to engine first with an UT_INKEY
		if (L && kid!=FUN_UNDO) {
			//qcout << "func::Inkey inputreg"  << endl;
			//overwrite what was input at beginning of local keys
			m_xengine->inkey(m_inputreg,false);
			clearinput();
		}
		// **** Function Keys **** send straight to engine
		m_qengine->docommand(kid);
		// complex result in DECIMAL mode?
		if (xcalc_radix==rtDECIMAL && m_xengine->asimag(iXreg)!=0) {
			xcalc_radix=rtCOMPLEX;
			m_qengine->newradix();
		}
	} // if (!local)
	// Make sure display is always updated...
	updateslot();

	// UNDO/REDO might change radix. Easiest way to fix that without really knowing...:
	if (kid==FUN_UNDO||kid==FUN_REDO) {
		m_qengine->newradix(); // updates all displays
	}
	// Label end is used when a normally legal key is rejected (overflow, '..', '//' etc.). Key shall not be sent to engine, and update not needed I think
end:;
	// probably not always needed, but it is needed if e.g. complex is triggered by a calculation
	updateslot();
} // end func (keyid)

void XCALCWindow::wheelEvent(QWheelEvent *event) {
	if (event->delta()>0) func(FUN_RUP);
	else func(FUN_RDN);
	event->accept();
}

//-----------------------------------------------------------------------
// Evaluate():
//		evaluate current input string STR/STR2 using CURR_RADIX (DECIMAL,COMPLEX,HEX,OCT,BIN)
//		delete rightmost character if an error (overflow and such)
//		update inputreg with current value (automatic reduction)
//-----------------------------------------------------------------------
void XCALCWindow::Evaluate()
{
	LD dval;
	int L;
	bool ok;
	WordLength wl = Register::wordLength();

	if (m_STR.startsWith("(")) m_STR=m_STR.mid(m_STR.indexOf(")")+2); // remove "(int) " and other indicators
	L = m_STR.length();

	//qcout << "XCALCWindow::Evaluate, " << endl;
	//qcout << "           radix = " << radixname(xcalc_radix) << endl;
	//qcout << "           m_STR = " << m_STR  << endl;
	//qcout << "           m_STR2 = " << m_STR2  << endl;
	//qcout << "           wl = " << (int)wl  << endl;
	switch (xcalc_radix) {
	case rtDECIMAL:
	{
		if (L>MaxInputDigits(rtDECIMAL,wl8BIT)) {
			m_STR.chop(1);
			break;
		}
		// First check for fractions (w/n/d)
		// (We can always set num/den to zero first)
		int i1,i2;
		//qcout << "clear inputreg" << endl;
		m_inputreg.setv1v2DMS(0.L,0.L,false);
		if ((i1=m_STR.indexOf("/"))!=-1) {
			qint64 w,n,d;
			if (w = toLongRobust(m_STR,ok),!ok) {
				m_STR.chop(1);
				break;
			}
			if (w==0) {	// Cannot have '0/' or '-0/'
				m_STR.chop(1);
				break;
			}
			QString nstr=m_STR.mid(i1+1);
			if (nstr.length()==0) break; // ok but incomplete
			if (n = toLongRobust(nstr,ok),!ok) {
				m_STR.chop(1);
				break;
			}
			if (n!=0) {
				if ((i2=nstr.indexOf("/"))!=-1) {
					QString dstr = nstr.mid(i2+1);
					if (dstr.length()==0) break; // ok but incomplete
					if (d = toLongRobust(dstr,ok),!ok) {
						m_STR.chop(1);
						break;
					}
					if (d!=0) {
						LD ldnum = (LD)w*(LD)d + (w<0? -n : n);
						if (ldnum != (long)ldnum) {
							// number too big for fraction
							m_STR.chop(1);
							break;
						}
						//qcout << "setting inputreg num/den " << w*d + (w<0? -n : n) << "/" << d << endl;
						m_inputreg.setfrac(w*d + (w<0? -n : n),d);
					}
					else {
						//qcout << "setting inputreg num/den " << w << "/" << n << endl;
						m_inputreg.setfrac(w,n);
					}
				}
				else {
					//qcout << "setting inputreg num/den " << w << "/" << n << endl;
					m_inputreg.setfrac(w,n);
				}
			}
			else {
				//qcout << "setting inputreg real " << (double)w << endl;
				m_inputreg.setv1v2DMS((LD)w,0,false);
				if (nstr[0].toAscii()=='0' || nstr[0].toAscii()=='/') { // Cannot enter '/0' or '//'
					m_STR.chop(1);
					break;
				}
			}
			// Only fraction if num!=0
			break;
		} // if contains '/'

		// Next check for DMS (d°m's)
		if ((i1=m_STR.indexOf(L'°'))!=-1) {
			int sign;
			long d,m;
			LD s=0;
			if (d = abs(toLongRobust(m_STR,ok)),!ok) {
				m_STR.chop(1);
				break;
			}
			if (!candms((LD)d)) {
				// d too large
				m_STR.chop(1);
				break;
			}
			sign = m_STR[0].toAscii()=='-'?-1:1;
			QString mstr = m_STR.mid(i1+1);
			if (m = toLongRobust(mstr,ok),!ok) {
				m_STR.chop(1);
				break;
			}
			if (m>=60) {
				// no min>59
				m_STR.chop(1);
				break;
			}
			if ((i2=mstr.indexOf("'"))!=-1) {
				QString sstr = mstr.mid(i2+1);
				if (s = toLDRobust(sstr,ok),!ok) {
					break;
				}
				if (s>=60) {
					// no sec>=60
					m_STR.chop(1);
					break;
				}
			}
			//qcout << "setting inputreg real DMS " << (double)(d + m*sign/60.0L + s*sign/3600.0L) << endl;
			m_inputreg.setv1v2DMS((LD)(d + m*sign/60.0L + s*sign/3600.0L),0,true);
			break;
		} // if contains dms L"°"

		if (dval=toLDRobust(m_STR,ok),!ok) {
			m_STR.chop(1);
			break;
		}
		if (m_iState>=0) {
			//qcout << "setting inputreg real dval " << (double)dval << endl;
			m_inputreg.setv1v2DMS(dval,0,false);
		}
		break;
	}
	case rtCOMPLEX: // called twice, first with real then imag part
	{
		if (L>MaxInputDigits(rtCOMPLEX,wl8BIT)) {
			m_STR.chop(1);
			break;
		}
		if (((L==1)&&(m_STR[0]==L'.')) || ((L==2)&&(m_STR[0]==L'-')&&(m_STR[1]==L'.'))) {
			dval = 0;
		}
		else if (dval=toLDRobust(m_STR,ok),!ok) {
			m_STR.chop(1);
			break;
		}
		if (m_iState>=0) {
			//qcout << "setting inputreg real dval " << (double)dval << endl;
			m_inputreg.setv1v2DMS(dval,0,false);
		}
		else {
			//qcout << "setting inputreg imag dval " << (double)dval << endl;
			m_inputreg.setv1v2DMS(m_inputreg.realval(),dval,false);
		}
		break;
	}
	case rtHEX:
	{
		if (L>MaxInputDigits(rtHEX,wl)) {
			m_STR.chop(1);
			break;
		}
		qint64 v = m_STR.toULongLong(&ok,16);
		if (!ok) {
			m_STR.chop(1);
			break;
		}
		//qcout << "setting inputreg hex " << v << endl;
		m_inputreg.setlonglong(v);
		break;
	}
	case rtOCTAL:
	{
		if (L>MaxInputDigits(rtOCTAL,wl)) {
			m_STR.chop(1);
			break;
		}
		qint64 v=m_STR.toULongLong(&ok,8);
		if (!ok) {
			m_STR.chop(1);
			break;
		}
		m_inputreg.setlonglong(v);
		break;
	}
	case rtBINARY:
	{
		if (L>MaxInputDigits(rtBINARY,wl)) {
			m_STR.chop(1);
			break;
		}
		qint64 v=m_STR.toULongLong(&ok,2);
		if (!ok) {
			m_STR.chop(1);
			break;
		}
		m_inputreg.setlonglong(v);
		break;
	}
	} // end switch radix()
	//qcout << "End evaluate, inputreg = ("  << inputreg.realval() << "," << inputreg.imagval() << ")," << inputreg.ival() << ", " << inputreg.numval() << "/" << inputreg.denval() << endl;
}

//-----------------------------------------------------------------------
// MakeString converts number in register iReg to string, using
// local mode variables (radix, sFix ..)
//-----------------------------------------------------------------------

QString XCALCWindow::MakeString(int iReg)
{
	Register &rr = m_xengine->regr(iReg);
	QString bstr;
	if (xcalc_showtype) bstr = QString("(%1) ").arg(ctnm(rr.ct()));
	if(maxval()==10) {
		if (xcalc_radix == rtDECIMAL) {
			// Separate routines to format fractions and DMS
			if (rr.isproperfrac())
				bstr+=mkfstr(rr);
			else if (rr.isdms())
				bstr+=mkdmsstr(rr);
			else
				bstr+=mkstr(rr.asreal(),DECOUTWIDTH);
		}
		else {
			QString sign = rr.asimag()<0?"":"+";
			bstr+=mkstr(rr.asreal(),CPLXOUTWIDTH)+" "+sign+mkstr(rr.asimag(),CPLXOUTWIDTH)+"i";
		}
	} else {
		bstr += snumber(rr.asint(),xcalc_wordLength,xcalc_radix).toUpper();
	}
	return bstr;
}

//-----------------------------------------------------------------------
// mkfstr converts fractional value to string
//-----------------------------------------------------------------------

QString XCALCWindow::mkfstr(Register &rr)
{
	QString bstr;
	if (m_xengine->cleanfrac()) {
		qint64 n = rr.numval();
		qint64 d = rr.denval();
		bstr = QString::number(n)+"/"+QString::number(d);
	}
	else {
		qint64 n = rr.numval();
		qint64 d = rr.denval();
		qint64 i = n/d;
		n = abs(n%d);
		bstr = QString::number(i)+" "+QString::number(n)+"/"+QString::number(d);
	}
	return bstr;
}

//-----------------------------------------------------------------------
// mkdmsstr converts DMS value to string
//-----------------------------------------------------------------------

QString XCALCWindow::mkdmsstr(Register &rr)
{
	LD V = rr.asreal();
	// Only called with (LD)rr within range
	LD D = fabs(V);
	long d=D;
	LD M = (D-d)*60;
	long m=M;
	LD S = (M-m)*60;
	int localFix;
	LD dmsdiv = 1.0L;
	LD dmsmul = 1.0L;

	// Round s to max DMSMAXFIX decimal places (dependent on Fix setting) and add carry if needed. Looks good!
	if (m_xengine->fix()==FLOATDISP)
		localFix = DMSMAXFIX;
	else
		localFix = m_xengine->fix();
	for (int i=0;i<localFix;i++) {
		dmsdiv /= 10.0L;
		dmsmul *= 10.0L;
	}
	S = dmsdiv * floor(S*dmsmul+0.5L);

	// Take 1: numeric value of S >= 60
	if (S>=60.0L) {
		S -= 60.0L;
		M++;
		if (M>=60) {
			M -= 60;
			D++;
		}
	}
	d=D;
	m=M;
	QString mstr; mstr.sprintf("%02d'",(int)m);
	QString sstr; sstr.sprintf("%#0*.*f\"",localFix+3,localFix,(double)S);

	// Take 2: S displays as "60*" because of rounding
	if (sstr.startsWith("60")) {
		m++;
		if (m>=60) {
			m -= 60;
			d++;
		}
		mstr.sprintf("%02d'",(int)m);
		sstr.sprintf("%0*.*f\"",localFix+3,localFix,(double)0);
	}
	QString bstr = QString::number(d)+QString(L'°')+mstr+sstr;

	// Taylor for FLOAT by removing trailing 0 and "."
	if (m_xengine->fix()==FLOATDISP) {
		bstr.chop(1); // remove "
		while (bstr.endsWith('0')) bstr.chop(1);
		if (bstr.endsWith('.')) bstr.chop(1);
		bstr += '"';
	}
	return bstr;
}

//-----------------------------------------------------------------------
// mkstr converts value to string at maximum wid characters
//-----------------------------------------------------------------------

// Define a small value to add to numbers before display

const LD LDBL_EPSILONL = 1.E-18L;

QString XCALCWindow::mkstr(LD value,int wid)
{
	bool zero,sign,UseExp,ForceExp,ForceEng;
	int i,Exp,EngExp,EngExtra,localFix,prec;
	LD Mant;
	QString tmp,etmp;

	// First check for infinities and NANs
	if (isnan(value)) return "NAN";
	else if (!std::isfinite(value)) return "Inf";

	if (m_xengine->fix()==FLOATDISP)
		localFix = DBL_DIG-1;  // DBL_DIG = 15
	else
		localFix = m_xengine->fix();

	ForceExp = m_xengine->fixtype()==ftSCI;
	ForceEng = m_xengine->fixtype()==ftENG;

	zero = (value==0);
	sign = (value<0);
	value = fabsl(value);

	if (zero) {
		Exp = 0;
		Mant = 0;
	}

	else {
		Exp=(int)floorl(log10(value));		// Exp is true exponent of ten
		Mant=value*pow(10.0L,-Exp-1);		// Mant now 0.1 <= mant < 1
		// add a small value to Mant so that e.g. 2.445 is displayed as "2.45"
		// using two decimals. This is the only way I can make it work with
		// LD numbers...
		Mant+=LDBL_EPSILONL;
		// We some times have rounding errors here. A few numbers might
		// end up with the exponent one off, and the mantissa equal to 1.
		// So far I know no better solution than this:
		if (Mant>=1.0L) {
			Mant *= 0.1L;
			Exp++;
		}
	}

	EngExp = 3*(int)(floor((double)Exp/3));
	EngExtra = ForceEng*(Exp-EngExp);
	// Use Exponential notation if
	UseExp = (localFix+Exp-sign<0 /*&& Mant<0.5L*/) // number too small
			// (or too wide with "-"),
			// (removed -sign: doesn't seem right!)
			// Ahh... Put that one back again...
			// It failed on "-4e-14" in float.
			// added 0.5L test to display "0.5" as "1."
			// Removed it again. Need more thinking.
			|| (sign+1+Exp+1>wid) // too big to display,
			|| (Exp>=DBL_DIG)         // more than sign. digits,
			|| ForceExp || ForceEng;  // or forced display.

	prec = std::min(localFix+1+EngExtra,DBL_DIG);			// restrict prec. for accuracy

	if (UseExp) {
		prec = std::min(prec,wid-sign-1-5);				// reduce prec. for "[-]e+000"
	}
	else {
		prec += Exp;
		if (m_xengine->fix()==FLOATDISP) prec-=sign;
		prec = std::min(std::min(prec,wid-1-sign),DBL_DIG);	// allow for - and .
	}

#ifdef __HAVELD__
	tmp.sprintf("%*.*Lf",prec,prec,Mant);
#else
	tmp.sprintf("%*.*lf",prec,prec,Mant);
#endif
	// fix possible rounding error if mant~<1 (Not elegant!!!)

	if ((Mant<1.0L)&&(tmp[0]=='1')) {
		Exp++;
		Mant*=0.1L;
		EngExp = 3*(int)(floor((double)Exp/3));
		EngExtra = ForceEng*(Exp-EngExp);

		UseExp = (localFix+Exp-sign<0)
				|| (sign+1+Exp+1>wid)
				|| (Exp>=DBL_DIG)
				|| ForceExp || ForceEng;

		prec = std::min(localFix+1+EngExtra,DBL_DIG);

		if (UseExp) {
			prec = std::min(prec,wid-sign-1-5);				// reduce prec. for "e+000"
		}
		else {
			prec += Exp;
			if (m_xengine->fix()==FLOATDISP) prec-=sign;
			prec = std::min(std::min(prec,wid-1-sign),DBL_DIG);	// allow for - and .
		}
#ifdef __HAVELD__
		tmp.sprintf("%*.*Lf",prec,prec,Mant);
#else
		tmp.sprintf("%*.*lf",prec,prec,Mant);
#endif
	}
	QString bstr;

	if (sign) bstr.append('-');

	if ((!UseExp)&&(Exp<0)) {
		bstr.append("0.");
		for (i=Exp+1;i<0;i++) bstr.append("0");
		bstr.append(tmp.mid(2));
		// Format is now fixed - taylor for FLOAT by removing trailing 0 and "."
		if (m_xengine->fix()==FLOATDISP) {
			while (bstr.endsWith("0")) bstr.chop(1);
			if (bstr.endsWith(".")) bstr.chop(1);
		}
	}
	else if (ForceEng) {
		bstr.append(tmp.mid(2,1+EngExtra));
		bstr.append(".");
		bstr.append(tmp.mid(3+EngExtra));
		// Format is now fixed - taylor for FLOAT by removing trailing 0 and "."
		if (m_xengine->fix()==FLOATDISP) {
			while (bstr.endsWith("0")) bstr.chop(1);
			if (bstr.endsWith(".")) bstr.chop(1);
		}
		bstr.append("e");
		etmp.sprintf("%+3.3d",EngExp);
		bstr.append(etmp);
	}
	else if (UseExp) {
		bstr.append(tmp.mid(2,1));
		bstr.append(".");
		bstr.append(tmp.mid(3));
		// Format is now fixed - taylor for FLOAT by removing trailing 0 and "."
		if (m_xengine->fix()==FLOATDISP) {
			while (bstr.endsWith("0")) bstr.chop(1);
			if (bstr.endsWith(".")) bstr.chop(1);
		}
		bstr.append("e");
		etmp.sprintf("%+3.3d",Exp);
		bstr.append(etmp);
	}
	else {
		bstr.append(tmp.mid(2,Exp+1));
		bstr.append(".");
		bstr.append(tmp.mid(3+Exp));
		// Format is now fixed - taylor for FLOAT by removing trailing 0 and "."
		if (m_xengine->fix()==FLOATDISP) {
			while (bstr.endsWith("0")) bstr.chop(1);
			if (bstr.endsWith(".")) bstr.chop(1);
		}
	}


	/*
  // Nov96: add thousands separator if decimal (but not complex)

  if (ThousandSep && CURR_RADIX == DECIMAL) {
	char *lastmant;
	if ((lastmant=strchr(bstr,'.'))==0 && (lastmant=strchr(bstr,'e'))==0)
	  lastmant = bstr+lstrlen(bstr);
	// lastmant now points one right of last mantissa digit
	lastmant -= 3;
	while (lastmant>bstr && isdigit(*(lastmant-1))) {
	  memmove(lastmant+1,lastmant,lstrlen(lastmant)+1);
	  *lastmant = ',';
	  lastmant -= 3;
	}
  }

  // Mar08: add space separator in fraction if decimal (but not complex)

  if (ThousandthSep && CURR_RADIX == DECIMAL) {
	char *dot;
	int i;
	if ((dot=strchr(bstr,'.'))!=0) {
	  // have decimal separator
	  while (dot<bstr+lstrlen(bstr)-4) {
		for (i=1;i<=4;i++)
		  if (dot[i]=='e')
			goto nomore;
		dot+=4;
		memmove(dot+1,dot,lstrlen(dot)+1);
		*dot=' ';
	  }
nomore:;
	}
  }
  */
	return bstr;
}

void XCALCWindow::SetSTR(QString s)
{
	// to accomodate Round, ignore any '(...) ' at start of s, possibly containing (real), (int) etc...
	if (s.startsWith('(')) s=s.section(' ',1);
	if (s.contains('/')) {
		m_STR = s;
		m_STR2.clear();
	} else {
		m_STR = s.section(' ',0,0);
		m_STR2 = s.section(' ',1,1);
	}
}
