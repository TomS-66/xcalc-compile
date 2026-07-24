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

#ifndef XCALCWINDOW_H
#define XCALCWINDOW_H

const int STKDISPSIZE = 4;
const int BUTROWS = 10;
const int BUTCOLS = 9;
const int NBUTTONS = BUTROWS*BUTCOLS;

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMainWindow>
#include <QSignalMapper>
#include <iostream>
#include "xcalc.h"

const char dPoint[] = {'.',','};
const char Comma[] =  {',','.'};

//--Limit digits on input-------------------------------------------------

//int MaxInputDigits(RadixType rt,WordLength wl);

class QEngine;

class XCALCWindow : public QMainWindow
{
	Q_OBJECT

public:
	XCALCWindow(QWidget *parent = 0);
	~XCALCWindow();
private:
	void DeleteUi();
	void SetupUi(); // using wRadix
	void InitMaps(); // one-time init of maps and action lists
public:
	void func(funid kid); // xcalc key ids
public slots:
	void funcslot(int id); // signalmapper can only send ints
	void updateslot();
	void newradixslot();
	void mailclickedslot();
	void wheelEvent(QWheelEvent *);
private:
	int m_iMaxDigits;	// For current mode of operation, gets value in SetupUi. Only used for dec/cpx - TODO looks like extra work done in both func and evaluate
	LD m_ldConstant;		// Temporary used to paste constants
	QSignalMapper *m_idMapper; // all action-like signals mapped through this
	QGridLayout *m_grid;
	QLabel **m_mems;
	QPushButton ***m_but;
	QLabel *m_lmsg;
	QLabel *m_lWL;
	QLabel *m_lfix;
	QLabel *m_lradix;
	int m_stkdispsize;
	int m_butrows;
	int m_butcols;
	QEngine *m_qengine;
	xcalc *m_xengine;
	int m_iState; // input key state (numbers, fractions)
	int m_iState2; // other part of key state when complex is being keyed
	RadixType radix(); // DECIMAL,COMPLEX,HEX,OCTAL,BINARY
	void setradix(RadixType r);
	WordLength wordLength();
	void setWordLength(WordLength wl);
	uint maxval(); // this is the REAL radix - 2 for binary etc.
	FixType m_fixtype; // FIX,SCI,ENG
	AngType m_ang; // DEG,RAD (for decimal mode; always rad in complex mode)
	QString m_STR; // input string (real then imag)
	QString m_STR2; // other part of input string when complex is being keyed
	void set(wchar c);
	inline void set(char c) { set (wchar(c)); }
	void app(wchar c);
	inline void app(char c) { app(wchar(c)); }
	void insbeg(wchar c);
	inline void insbeg(char c) { insbeg(wchar(c)); }
	void delbeg(void);
	void evalnew(void); // {if (m_xengine->sl()) m_xengine->lift(); m_xengine->setsl(true); Evaluate();}
	void swap(void); // {SWAP(m_STR,m_STR2); SWAP(m_iState,m_iState2); setradix(rtCOMPLEX); updateslot();}
	int  strindex(char, const char * const);
	QString MakeString(int iReg);
	QString mkstr(LD ld,int wid);
	QString mkfstr(Register &rr);
	QString mkdmsstr(Register &rr);
	void clearinput();
	// Shortcut fixes (radix based)
	void translatekid(funid &kid);
	bool kidallowed(funid kid);
	QPushButton *FindButton(funid kid);
public:
	QLabel **m_lstk; // needed public for round to fix
	Register m_inputreg; // for keying in values, needed public for round to fix
	void SetSTR(QString s);
	void Evaluate(); // needed as public for round to fix
	static bool m_mapsok;
};

#endif // XCALCWINDOW_H
