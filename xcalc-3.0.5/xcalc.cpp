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
#include "aschar.h"
#include "qengine.h"
#include "xcalcrc.h"
#include "xcalcutil.h"
#include "xcalcwindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <exception>
#include "xcalcconfig.h"
#include "ui_xcalcconfig.h"

// All persistent global state variables
XCALCWindow *xcalc_win;
RadixType xcalc_radix;
WordLength xcalc_wordLength;
bool xcalc_cleanFrac;
bool xcalc_invTrigAsDMS;
FixType xcalc_fixtype;
int xcalc_fix;
AngType xcalc_ang;
bool xcalc_copytop;
bool xcalc_showtype;
QTextStream qcout(stdout);

QString xcalc_helpdir;
QString xcalc_appPath;

QString ABOUT;// initialised in xcalcwindow constructor

QString GPL = "\
This program is free software: you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation, either version 3 of the License, or \
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License \
along with this program: see copying.txt. If not, see <http://www.gnu.org/licenses/>.";

// Constant text variables
const char *XCALCVERSION	=	"3.0.5";
const char *XCALCDATE		=	"10 Jun 2012";
const char *COPYRIGHT		=	"Copyright (c) Bernt Ribbum 1992-2012";
const char *EMAIL			=	"bernt.ribbum@gmail.com";
const char *WEBADDR			=	"http://www.tordivel.no/xcalc";
QString TEXTVERSION			=	QString("XCALC ")+XCALCVERSION;
QString MAILTO				=	QString("mailto:Bernt Ribbum<")+EMAIL+">?subject=XCALC "+XCALCVERSION;

xcalc::xcalc(QEngine *prnt) : m_prnt(prnt)
{
	ABOUT = QString(TEXTVERSION)+"\n"+XCALCDATE+"\n"+COPYRIGHT+"\nEmail "+EMAIL+"\nWeb "+WEBADDR+"\n\n";
	reset();
	qcout.setCodec("UTF-8");
	//qcout << QString("starting xcalc") << endl;
}

void xcalc::resetmsg()
{
	m_smsg = QString("Welcome to ")+TEXTVERSION;
}

void xcalc::reset()
{
	resetmsg();
	m_UndoIndex = 0;
	m_Undoes = 0;
	m_SL=true;
	xcalc_ang = atDEGREE;
	xcalc_radix = rtDECIMAL;
	xcalc_wordLength = wl32BIT;
	xcalc_cleanFrac = true;
	xcalc_invTrigAsDMS = false;
	xcalc_fixtype = ftFIX;
	xcalc_fix = 4;
	xcalc_copytop = true;
	xcalc_showtype = false;
	m_msg=__MESSAGE;
	m_LastUndone=FUN_NONE;
}

Register *xcalc::regptr(int reg)
{
	if (reg>=0 && reg<STKS) return &m_ST[reg];			// 0 up is stack
	else if (reg==iLReg) return &m_lreg;				// -1 is lastx
	else if (reg>=100&&reg<100+MEMS) return memptr(reg-100);	// 100+ is memory
	else return &m_noreg;						// don't segfault
}

Register *xcalc::memptr(int reg)
{
	return &m_mem[reg];
}

// degree/radian support (always radians in complex mode or with complex registers)
Register xcalc::RADTOANG(Register a) { // a as represented in current angle measure
	if (angtype()==atDEGREE && !a.ispropercomplex()) {
		return Register(a*180/M_PIl,0L,xcalc_invTrigAsDMS);
	}
	else return a;
}

Register xcalc::ANGTORAD(Register a) { // a as represented in radians
	if (angtype()==atDEGREE && !a.ispropercomplex()) {
		return Register(a*M_PIl/180,0L,false);
	}
	else return a;
}

// Private operations (e.g. math functions)
void xcalc::incWL(){
	xcalc_wordLength = (WordLength)((xcalc_wordLength+1) % NWL);
	for (int i=iXreg;i<iTreg;i++)
		m_ST[i] = m_ST[i];
}

void xcalc::decWL(){
	xcalc_wordLength = (WordLength)((xcalc_wordLength-1+NWL) % NWL);
	for (int i=iXreg;i<iTreg;i++)
		m_ST[i] = m_ST[i];
}

// radix conversions involve whole stack + lastx (without lastx better, but requires more work when it is recalled.)
void xcalc::ToDecimal(){
	for (int i=iLReg;i<STKS;i++) {
		Register &rr = *regptr(i);
		rr=rr.setv1v2DMS(rr.asfloat(),0,rr.isdms());
	}
}

void xcalc::ToComplex() {
	for (int i=iLReg;i<STKS;i++) {
		Register &rr = *regptr(i);
		rr=rr.setv1v2DMS(rr.asreal(),rr.asimag(),rr.isdms());
	}
}

void xcalc::ToBin() {
	for (int i=iLReg;i<STKS;i++) {
		Register &rr = *regptr(i);
		rr=rr.setlonglong(rr.asint());
	}
}

void xcalc::ToCart(){
	// if complex, use xreg real,imag else xreg,yreg
	//TODO
	Register &rx = m_ST[iXreg];
	Register &ry = m_ST[iYreg];
	LD r,th,x,y;
	if (xcalc_radix==rtCOMPLEX) {
		r = rx.asreal();
		th = rx.asimag();
	}
	else {
		r = rx.asfloat();
		th = ry.asfloat();
	}
	// convert r,th to x,y
	y = r*sin(ANGTORAD(th));
	x = r*cos(ANGTORAD(th));
	//
	if (xcalc_radix==rtCOMPLEX) {
		rx.setv1v2DMS(x,y,false);
	}
	else {
		rx.setv1v2DMS(x,0,false);
		ry.setv1v2DMS(y,0,false);
	}
}

void xcalc::ToPolar(){
	// if complex, use xreg real,imag else xreg,yreg
	//TODO
	Register &rx = m_ST[iXreg];
	Register &ry = m_ST[iYreg];
	LD rr,th,x,y;
	if (xcalc_radix==rtCOMPLEX) {
		x = rx.asreal();
		y = rx.asimag();
	}
	else {
		x = rx.asfloat();
		y = ry.asfloat();
	}
	// convert y,x to rr,th
	th = RADTOANG(Register(atan2(y,x),0L,false));
	rr = hypot(y,x);
	if (xcalc_radix==rtCOMPLEX) {
		rx.setv1v2DMS(rr,th,false);
	}
	else {
		rx.setv1v2DMS(rr,0,false);
		ry.setv1v2DMS(th,0,xcalc_ang==atDEGREE && xcalc_invTrigAsDMS);
	}
}

void xcalc::clx() {
	m_ST[iXreg].clear();
}

void xcalc::clstk() {
	for (int i=iXreg; i<STKS;i++)
		m_ST[i].clear();
	m_lreg.clear();
}

void xcalc::Chs() {
	m_ST[iXreg]=Register(-m_ST[iXreg].ascomplex());
}

void xcalc::Add() {
	m_ST[iYreg]+=m_ST[iXreg];
	drop();
}

void xcalc::Sub() {
	m_ST[iYreg]-=m_ST[iXreg];
	drop();
}

void xcalc::Mul() {
	m_ST[iYreg]*=m_ST[iXreg];
	drop();
}

void xcalc::Div() {
	m_ST[iYreg]/=m_ST[iXreg];
	drop();
}

void xcalc::Divf() {
	m_ST[iYreg]/=m_ST[iXreg];
	drop();
}

void xcalc::Mod() {
	m_ST[iXreg]%=m_ST[iYreg];
	drop();
}

void xcalc::Modf() {
	m_ST[iXreg]%=m_ST[iYreg];
	drop();
}

void xcalc::Sqrt() {
	m_ST[iXreg]=m_ST[iXreg].sqrt();
}

void xcalc::Sqr() {
	m_ST[iXreg]*=m_ST[iXreg];
}

void xcalc::Qroot() {
	m_ST[iXreg]=Register(introot(m_ST[iXreg].ascomplex(),3));
}

void xcalc::Cube() {
	m_ST[iXreg]=Register(m_ST[iXreg].ascomplex()*m_ST[iXreg].ascomplex()*m_ST[iXreg].ascomplex());
}

void xcalc::Log() {
	m_ST[iXreg]=Register(log(m_ST[iXreg].ascomplex()));
}

void xcalc::Exp() {
	m_ST[iXreg]=Register(exp(m_ST[iXreg].ascomplex()));
}

void xcalc::Log10() {
	m_ST[iXreg]=Register(log10(m_ST[iXreg].ascomplex()));
}

void xcalc::Ten() {
	/*
	Register r(m_ST[iXreg].ten());
	m_ST[iXreg]=r;
	*/
	m_ST[iXreg]=Register(m_ST[iXreg].ten());
}

void xcalc::Pow() {
	m_ST[iYreg]=m_ST[iYreg].pow(m_ST[iXreg]);
	drop();
}

void xcalc::Root() {
	m_ST[iYreg]=m_ST[iYreg].root(m_ST[iXreg]);
	drop();
}
void xcalc::Sin() {
	m_ST[iXreg]=ANGTORAD(m_ST[iXreg]).sin();
}
void xcalc::Cos() {
	m_ST[iXreg]=ANGTORAD(m_ST[iXreg]).cos();
}
void xcalc::Tan() {
	m_ST[iXreg]=ANGTORAD(m_ST[iXreg]).tan();
}
void xcalc::ASin() {
	m_ST[iXreg]=RADTOANG(m_ST[iXreg].asin());
}
void xcalc::ACos() {
	m_ST[iXreg]=RADTOANG(m_ST[iXreg].acos());
}
void xcalc::ATan() {
	m_ST[iXreg]=RADTOANG(m_ST[iXreg].atan());
}

void xcalc::Sinh() {
	m_ST[iXreg]=m_ST[iXreg].sinh();
}
void xcalc::Cosh() {
	m_ST[iXreg]=m_ST[iXreg].cosh();
}
void xcalc::Tanh() {
	m_ST[iXreg]=m_ST[iXreg].tanh();
}
void xcalc::ArSinh() {
	m_ST[iXreg]=m_ST[iXreg].arsinh();
}
void xcalc::ArCosh() {
	m_ST[iXreg]=m_ST[iXreg].arcosh();
}
void xcalc::ArTanh() {
	m_ST[iXreg]=m_ST[iXreg].artanh();
}

void xcalc::Abs() {
	m_ST[iXreg]=m_ST[iXreg].abs();
}

void xcalc::Rcp() {
	m_ST[iXreg]=m_ST[iXreg].rcp();
}

void xcalc::Pi() {
	m_ST[iXreg]=Register(M_PIl,0L,false);
}

void xcalc::Fact() {
	m_ST[iXreg]=m_ST[iXreg].fact();
}

void xcalc::TOij() {
	Register &rX(m_ST[iXreg]);
	Register &rY(m_ST[iYreg]);
	rY=Register(rY.asreal(),rX.asreal(),false);
	drop();
}

void xcalc::TOyx() {
	lift();
	Register &rX(m_ST[iXreg]);
	Register &rY(m_ST[iYreg]);
	rY=Register(rY.asreal(),0L,false);
	rX=Register(rX.asimag(),0L,false);
}

void xcalc::ReIm() {
	Register &rX(m_ST[iXreg]);
	rX=Register(rX.asimag(),rX.asreal(),rX.isdms());
}

void xcalc::Conj() {
	Register &rX(m_ST[iXreg]);
	rX=Register(rX.asreal(),-rX.asimag(),false);
}

void xcalc::lift()
{
	for (int i=iTreg;i>=iYreg;i--)
		m_ST[i]=m_ST[i-1];
	m_ST[iXreg] = m_ST[iXreg]; // reduce Xreg
}

void xcalc::drop()
{
	for (int i=iXreg;i<iTreg;i++)
		m_ST[i]=m_ST[i+1];
	if (!xcalc_copytop)
		m_ST[iTreg].clear();
	else
		m_ST[iTreg] = m_ST[iTreg]; // reduce Treg
}

void xcalc::RUP(){
	Register tmp(m_ST[iTreg]);
	lift();
	m_ST[iXreg]=tmp;
}

void xcalc::RDN(){
	Register tmp(m_ST[iXreg]);
	drop();
	m_ST[iTreg]=tmp;
}

void xcalc::Random(){
	m_ST[iXreg]=m_ST[iXreg].random();
}

void xcalc::ToFrac(){
	Register &rX(m_ST[iXreg]);
	if (rX.isproperfrac()) rX.setfrac(rX.asnum()%rX.asden(),rX.asden());
	else rX.setv1v2DMS(rX.asfloat()-rX.asint(),0,rX.isdms());
}

void xcalc::ToInt(){
	Register &rX(m_ST[iXreg]);
	rX.setv1v2DMS((LD)rX.asint(),0.,rX.isdms());
}

void xcalc::ToggleCleanFrac(){
	xcalc_cleanFrac = !xcalc_cleanFrac;
	update();
}

void xcalc::Round(){
	// Remove Frac or DMS from Xreg, or round to fix (has problems with undo/redo!)
	Register &rX(m_ST[iXreg]);
	if (rX.isfrac()||rX.isdms())
		rX.setcpx(rX.ascomplex());
	else {
		xcalc_win->SetSTR(xcalc_win->m_lstk[0]->text());
		xcalc_win->Evaluate();
		inkey(xcalc_win->m_inputreg,true); // must be undoable?
	}
}

void xcalc::And(){
	m_ST[iYreg]=m_ST[iYreg].And(m_ST[iXreg]);
	drop();
}

void xcalc::Or(){
	m_ST[iYreg]=m_ST[iYreg].Or(m_ST[iXreg]);
	drop();
}

void xcalc::Xor(){
	m_ST[iYreg]=m_ST[iYreg].Xor(m_ST[iXreg]);
	drop();
}

void xcalc::Not(){
	m_ST[iXreg]=m_ST[iXreg].Not();
}

void xcalc::Shl(){
	m_ST[iXreg]=m_ST[iXreg].shl();
}

void xcalc::Shr(){
	m_ST[iXreg]=m_ST[iXreg].shr();
}

void xcalc::Ashl(){
	m_ST[iXreg]=m_ST[iXreg].ashl();
}

void xcalc::Ashr(){
	m_ST[iXreg]=m_ST[iXreg].ashr();
}

void xcalc::Rotl(){
	m_ST[iXreg]=m_ST[iXreg].rotl();
}

void xcalc::Rotr(){
	m_ST[iXreg]=m_ST[iXreg].rotr();
}

void xcalc::set8bit(){
	Register::setWordLength(wl8BIT);
	update();
}

void xcalc::set16bit(){
	Register::setWordLength(wl16BIT);
	update();
}

void xcalc::set32bit(){
	Register::setWordLength(wl32BIT);
	update();
}

void xcalc::set64bit(){
	Register::setWordLength(wl64BIT);
	update();
}

// Public operations

void xcalc::update()
{
	m_prnt->update();
}

int inkeynext = 0;
QList<Register> inkeylist;

void xcalc::inkey() // used only when redone
{
	Register r = inkeylist.at(inkeynext++);
	// SaveUndo done in func(), not needed here!
	m_SL = true;
	qcout << QString("redone xcalc::inkey [%1] (%2,%3) %4/%5").arg(ctName(r.ct())).arg((double)r.realval()).arg((double)r.imagval()).arg(r.numval()).arg(r.denval()) << endl;
	m_ST[iXreg]=r;
	update();
}

void xcalc::inkey(Register &r,bool undoable)
{
	if (undoable) {
		// 10jun2012: moved SaveUndo FIRST
		SaveUndo(FUN_INKEY,false);
		//if (m_SL) lift();
		m_SL = true;
		qcout << QString("xcalc::inkey [%1] undoable (%2,%3) %4/%5").arg(ctName(r.ct())).arg((double)r.realval()).arg((double)r.imagval()).arg(r.numval()).arg(r.denval()) << endl;
		inkeylist.append(r);
		inkeynext++;
	} else {
		qcout << QString("xcalc::inkey [%1] once (%2,%3) %4/%5").arg(ctName(r.ct())).arg((double)r.realval()).arg((double)r.imagval()).arg(r.numval()).arg(r.denval()) << endl;
		inkeylist[inkeynext-1]=r;
	}
	m_ST[iXreg]=r;
	update();
}

// RUN sets lastx prior to executing f
void xcalc::RUN(void (xcalc::*f)()) {
	Register t=m_ST[iXreg];
	(this->*f)();
	m_lreg=t;
}

void xcalc::STO(int n) {
	regr(n+100)=m_ST[iXreg];
}

void xcalc::RCL(int n) {
	if (m_SL) lift(); m_ST[iXreg]=regr(n+100);
}

void xcalc::CLR(int n) {
	regr(n+100).clear();
}

messagecode xcalc::func(funid id,bool redone)
{
	// Real maths function. All input massaging (edit) is done in xcalcwindow. When a number is finished the result is sent here first via inkey, then func is called
	// Only legal ids are sent: mapping from char to keyid is done in xcalcwindow.
	// There are no more errors from maths functions - overflows return INF, singularities return NAN, ints are chopped to wordlength
	Register &X(m_ST[iXreg]);
	Register &Y(m_ST[iYreg]);
	Register &L(m_lreg);
	try {
		// Always update undo queue - when full, push oldest undo away
		qcout << "xcalc::func id = " << fname.value(id) << endl;
		SaveUndo(id,redone); // does the right thing (nothing!) if id is UNDO, REDO, ABOUT, CONFIG or any HELP
		Register r;
		// Remove possible Undo/Redo messages
		m_msg = __MESSAGE;
		/*if (m_msg==__MESSAGE)*/ m_smsg = fname.value(id);
		xcalc_win->updateslot();
		int res;
		Ui_XCalcConfig *conf;
		QDialog *dlg;
		QString helpfile;
		switch (id) {

		case FUN_CONFIG:
			dlg = new QDialog(xcalc_win);
			conf = new Ui_XCalcConfig;
			conf->setupUi(dlg);
			conf->cbcleanFrac->setChecked(xcalc_cleanFrac);
			conf->cbDMS->setChecked(xcalc_invTrigAsDMS);
			conf->leHelpDir->setText(xcalc_helpdir);
			conf->cbCopyTop->setChecked(xcalc_copytop);
			conf->mbShowType->setChecked(xcalc_showtype);
			dlg->connect(conf->mailButton,SIGNAL(clicked()),xcalc_win,SLOT(mailclickedslot()));
			res = dlg->exec();
			if (res==QDialog::Accepted) {
				xcalc_cleanFrac=conf->cbcleanFrac->isChecked();
				xcalc_invTrigAsDMS=conf->cbDMS->isChecked();
				SetHelpDir(conf->leHelpDir->text());
				xcalc_copytop = conf->cbCopyTop->isChecked();
				xcalc_showtype = conf->mbShowType->isChecked();
				xcalc_win->updateslot();
			}
			resetmsg();
			break;
		case FUN_HELP:
			ShowHelp(INDEXHELP);
			break;
		case FUN_KEYHELP:
			switch (xcalc_radix) {
			case rtDECIMAL: helpfile = DECIMALKEYHELP; break;
			case rtCOMPLEX: helpfile = COMPLEXKEYHELP; break;
			default: helpfile = BINARYKEYHELP;
			}
			ShowHelp(helpfile);
			break;
		case FUN_UNDO: m_msg=Undo(m_LastUndone); break;
		case FUN_REDO: m_msg=Redo(m_LastUndone); break; //Redo actually executes the function again - no need to save afterstate!
		case FUN_ABOUT:
			QMessageBox::information(xcalc_win,"About XCalc",ABOUT+GPL);
			resetmsg();
			break;
		case FUN_WLUP: RUN(&xcalc::incWL); break;
		case FUN_WLDOWN: RUN(&xcalc::decWL); break;
		case FUN_INKEY: inkey(); break;
		case FUN_ROUND:
			// possibly following INKEY fixed in Redo
			RUN(&xcalc::Round); break;
		case FUN_CHS: RUN(&xcalc::Chs); break;
		case FUN_ENTER: lift(); break;
		case FUN_ADD: RUN(&xcalc::Add); break;
		case FUN_SUB: RUN(&xcalc::Sub); break;
		case FUN_MUL: RUN(&xcalc::Mul); break;
		case FUN_DIV: RUN(&xcalc::Div); break;
		case FUN_DIVF: RUN(&xcalc::Divf); break;
		case FUN_MOD: RUN(&xcalc::Mod); break;
		case FUN_MODF: RUN(&xcalc::Modf); break;
		case FUN_LASTX: if (m_SL) lift(); X=L; break;
		case FUN_CLX: clx(); break;
		case FUN_CLSTK: clstk(); break;
		case FUN_TOYX: TOyx(); break; //Toyx= cpx > y.re,x.re (split)
		case FUN_TODMS: X.setDMS(true); break;
		case FUN_DECIMAL: ToDecimal(); setradixtype(rtDECIMAL); break;
		case FUN_COMPLEX: ToComplex(); setradixtype(rtCOMPLEX); break;
		case FUN_HEX: ToBin(); setradixtype(rtHEX); break;
		case FUN_OCTAL: ToBin(); setradixtype(rtOCTAL); break;
		case FUN_BINARY: ToBin(); setradixtype(rtBINARY); break;
		case FUN_XY: SWAP(X,Y); break;
		case FUN_RUP: RUP(); break;
		case FUN_RDN: RDN(); break;
		case FUN_PERC: break;
		case FUN_CONV: break;
		case FUN_QCONV: break;
		case FUN_CONS: break;
		case FUN_QCONS: break;
		case FUN_TOFRAC: RUN(&xcalc::ToFrac); break;
		case FUN_TOINT: RUN(&xcalc::ToInt); break;
		case FUN_CLEANFRAC: ToggleCleanFrac(); break;
		case FUN_CART: RUN(&xcalc::ToCart); break;
		case FUN_POLAR: RUN(&xcalc::ToPolar); break;
		case FUN_EXP:RUN(&xcalc::Exp); break;
		case FUN_TEN:RUN(&xcalc::Ten); break;
		case FUN_ROOT: RUN(&xcalc::Root); break;
		case FUN_SQRT: RUN(&xcalc::Sqrt); break;
		case FUN_QROOT: RUN (&xcalc::Qroot); break;
		case FUN_LN: RUN(&xcalc::Log); break;
		case FUN_LOG: RUN(&xcalc::Log10); break;
		case FUN_POW: RUN(&xcalc::Pow); break;
		case FUN_SQR: RUN(&xcalc::Sqr); break;
		case FUN_CUBE: RUN(&xcalc::Cube); break;
		case FUN_ABS: RUN(&xcalc::Abs); break;
		case FUN_SIN: RUN(&xcalc::Sin); break;
		case FUN_COS: RUN(&xcalc::Cos); break;
		case FUN_TAN: RUN(&xcalc::Tan); break;
		case FUN_ASIN: RUN(&xcalc::ASin); break;
		case FUN_ACOS: RUN(&xcalc::ACos); break;
		case FUN_ATAN: RUN(&xcalc::ATan); break;
		case FUN_SINH: RUN(&xcalc::Sinh); break;
		case FUN_COSH: RUN(&xcalc::Cosh); break;
		case FUN_TANH: RUN(&xcalc::Tanh); break;
		case FUN_ARSINH: RUN(&xcalc::ArSinh); break;
		case FUN_ARCOSH: RUN(&xcalc::ArCosh); break;
		case FUN_ARTANH: RUN(&xcalc::ArTanh); break;
		case FUN_RCP: RUN(&xcalc::Rcp); break;
		case FUN_PI: if (m_SL) lift(); Pi(); break;
		case FUN_RAND: if (m_SL) lift(); Random(); break;
		case FUN_FACT: RUN(&xcalc::Fact); break;
		case FUN_TOIJ: RUN(&xcalc::TOij); break; //TOij= y.r,x.r > cpx (connect)
		case FUN_REIM: RUN(&xcalc::ReIm); break;
		case FUN_CONJ: RUN(&xcalc::Conj); break;
		case FUN_AND: RUN(&xcalc::And); break;
		case FUN_OR: RUN(&xcalc::Or); break;
		case FUN_XOR: RUN(&xcalc::Xor); break;
		case FUN_NOT: RUN(&xcalc::Not); break;
		case FUN_SHL: RUN(&xcalc::Shl); break;
		case FUN_SHR: RUN(&xcalc::Shr); break;
		case FUN_ASHL: RUN(&xcalc::Ashl); break;
		case FUN_ASHR: RUN(&xcalc::Ashr); break;
		case FUN_ROTL: RUN(&xcalc::Rotl); break;
		case FUN_ROTR: RUN(&xcalc::Rotr); break;
		case FUN_STO0:
		case FUN_STO1:
		case FUN_STO2:
		case FUN_STO3:
		case FUN_STO4:
		case FUN_STO5:
		case FUN_STO6:
		case FUN_STO7:
		case FUN_STO8:
		case FUN_STO9: STO(id-FUN_STO0); break;
		case FUN_RCL0:
		case FUN_RCL1:
		case FUN_RCL2:
		case FUN_RCL3:
		case FUN_RCL4:
		case FUN_RCL5:
		case FUN_RCL6:
		case FUN_RCL7:
		case FUN_RCL8:
		case FUN_RCL9: RCL(id-FUN_RCL0); break;
		case FUN_CLR0:
		case FUN_CLR1:
		case FUN_CLR2:
		case FUN_CLR3:
		case FUN_CLR4:
		case FUN_CLR5:
		case FUN_CLR6:
		case FUN_CLR7:
		case FUN_CLR8:
		case FUN_CLR9: CLR(id-FUN_CLR0); break;
		case FUN_FIX: setfixtype(ftFIX); break;
		case FUN_SCI: setfixtype(ftSCI); break;
		case FUN_ENG: setfixtype(ftENG); break;
		case FUN_FIX0: case FUN_FIX1: case FUN_FIX2: case FUN_FIX3: case FUN_FIX4:
		case FUN_FIX5: case FUN_FIX6: case FUN_FIX7: case FUN_FIX8: case FUN_FIX9: setfix(id-FUN_FIX0); break;
		case FUN_FLOAT: setfix(FLOATDISP); break;
		case FUN_FIXM: setfix(fix()-1); break;
		case FUN_FIXP: setfix(fix()+1); break;
		case FUN_DEG: setangtype(atDEGREE); break;
		case FUN_RAD: setangtype(atRADIAN); break;
		case FUN_8BIT: set8bit(); break;
		case FUN_16BIT: set16bit(); break;
		case FUN_32BIT: set32bit(); break;
		case FUN_64BIT: set64bit(); break;
		case FUN_CLRMEM: for (int m=0;m<MEMS;m++) CLR(m); break;
		case FUN_EXIT: qApp->exit(); break;
		default: ;/* do nothing */
		} // end switch id
	} // end try
	catch (...) {
		//qcout << "something thrown!" << endl;
		//m_msg = __ERROR;
	}
	// Functions don't fail any more - result is inf on nan instead.
	// Stack lift always turned on except after ENTER, CLX, CLSTK, NONE; UNDO and REDO update correctly on their own
	if (id!=FUN_UNDO && id!=FUN_REDO)
		m_SL = (id!= FUN_ENTER && id!= FUN_CLSTK && id!= FUN_NONE && id!= FUN_CLX);
	// Tell all views to update (except after redo - that's already taken care of)
	if (id!=FUN_REDO) {
		update();
		//qcout << QString("Func=%1 undoindex=%2 undoes=%3").arg(idname(id)).arg(m_UndoIndex).arg(m_Undoes) << endl;
		// debug output to examine undo stack
		//for (int i=0;i<m_UndoIndex;i++) qcout << QString("undo %1: %2").arg(i).arg(fname.value(m_UndoInfo[i].undone)) << endl;
	}
	return m_msg;
} // func()

void  xcalc::showundoes()
{
	int i = m_UndoIndex-1;
	qcout << "undolist:" << endl;
	while (i>=0) {
		qcout << i << " " << fname.value(m_UndoInfo[i].undone) << endl;
		i--;
	}
}

//--Undo function---------------------------------------------------------
// Actions:
// - If undo information exists, change back to previous state and inc m_Undoes
//------------------------------------------------------------------------

messagecode xcalc::Undo(funid &Undone)
{
	int i;

	showundoes();
	if (m_UndoIndex>0) {
		// Undoable information exists - proceed
		m_UndoIndex--;
		m_Undoes++;
		// Get what is undone
		Undone = m_UndoInfo[m_UndoIndex].undone;
		qcout << QString("Just undid %1").arg(fname.value(Undone)) << endl;
		if (Undone==FUN_INKEY) inkeynext--;
		// Always restore previous stacklift content
		m_SL = m_UndoInfo[m_UndoIndex].sl;
		// Check stack changes, and update if any
		for (i=iXreg;i<STKS;i++) {
			m_ST[i] = m_UndoInfo[m_UndoIndex].st[i];
		}
		//Register &x = m_ST[iXreg];
		if (Undone==FUN_INKEY && m_UndoIndex>=0 && m_UndoInfo[m_UndoIndex].undone==FUN_ROUND) {
			// FUN_INKEY may be proceeded by FUN_ROUND.
			// in that case, undo this as well
			funid is_round;
			Undo(is_round);
		}
		m_lreg = m_UndoInfo[m_UndoIndex].lastx;
		// Undo any memory changes
		for (i=0;i<MEMS;i++) {
			m_mem[i] = m_UndoInfo[m_UndoIndex].mem[i];
		}
		// Wordlength, radix
		Register::setWordLength(m_UndoInfo[m_UndoIndex].wl);
		setradixtype(m_UndoInfo[m_UndoIndex].radix);
		// Fix settings, cleanFrac toggle
		setfix(m_UndoInfo[m_UndoIndex].fix);
		setfixtype(m_UndoInfo[m_UndoIndex].fixtype);
		setcleanfrac(m_UndoInfo[m_UndoIndex].cleanFrac);
		// Update conversion settings
		/*
	ConvType = UndoInfo[UndoIndex].ConvType;
	From = UndoInfo[UndoIndex].From;
	To = UndoInfo[UndoIndex].To;
	memcpy(SelectFrom,UndoInfo[UndoIndex].SelectFrom,NCONVERSIONS*sizeof(short));
	memcpy(SelectTo,UndoInfo[UndoIndex].SelectTo,NCONVERSIONS*sizeof(short));
	// Update last constant
	ldConstant = UndoInfo[UndoIndex].ldConstant;
	*/
		// Maybe display has changed - doesn't hurt otherwise
		m_prnt->update();
		m_prnt->newradix();
		showundoes();
		return __UNDONE;
	}
	else {
		// No undoable information - set error
		qcout << "Nothing to undo" << endl;
		showundoes();
		return __NOUNDO;
	}
}

//--Redo function---------------------------------------------------------
// Actions:
// - Redo by actually executing function again. (must have free Undoes)
//------------------------------------------------------------------------
messagecode xcalc::Redo(funid &Redone)
{
	qcout << "Before Redo" << endl;
	showundoes();
	if (m_Undoes>0) {
		// Undo information exists - proceed
		// Redo what is undone
		Redone  = m_UndoInfo[m_UndoIndex].undone;
		qcout << QString("Redoing %1").arg(fname.value(Redone)) << endl;
		m_Undoes--;
		// if Redone==FUN_ROUND followed by FUN_INKEY, redo that as well
		if (Redone==FUN_ROUND && m_Undoes>0 && m_UndoInfo[m_UndoIndex+1].undone==FUN_INKEY) {
			funid is_inkey;
			Redo(is_inkey);
		}
		//m_UndoIndex++; done in func!
		func(Redone,true);
		//m_prnt->update();
		return __REDONE;
	}
	else {
		// No redo information - set error
		qcout << "Nothing to redo" << endl;
		return __NOREDO;
	}
	qcout << "After Redo" << endl;
	showundoes();
}

//--SaveUndo function-----------------------------------------------------
// - Save undo information on stack, if necessary push stack downwards
// 	 to make room.
//	 CommitUndo always implied - no errors to prevent it any more!
//------------------------------------------------------------------------

void xcalc::SaveUndo(funid id,bool redone)
{
	// Do not save UNDO, REDO, ABOUT, CONFIG or HELP operations
	// For debugging purposes, also do not save FIX+,FIX-
	// TODO: how to treat Round?
	// TODO: must somehow be smart and let redone ROUND trigger an INKEY, and update indices...
	if (!(id==FUN_UNDO||id==FUN_REDO||id==FUN_ABOUT||id==FUN_CONFIG||id==FUN_HELP||id==FUN_KEYHELP||id==FUN_FIXP||id==FUN_FIXM)) {
		UndoInfoStruct UndoTemp;
		UndoTemp.undone = id;
		memcpy(UndoTemp.mem,m_mem,MEMS*sizeof(Register));
		memcpy(UndoTemp.st,m_ST,STKS*sizeof(Register));
		UndoTemp.lastx = m_lreg;
		UndoTemp.sl = m_SL;
		UndoTemp.wl = Register::wordLength();
		UndoTemp.radix = radixtype();
		UndoTemp.fix = fix();
		UndoTemp.fixtype = fixtype();
		UndoTemp.cleanFrac = xcalc_cleanFrac;
		// Update conversion settings
		//UndoTemp.ConvType = ConvType;
		//UndoTemp.From = From;
		//UndoTemp.To = To;
		//memcpy(UndoTemp.SelectFrom,SelectFrom,NCONVERSIONS*sizeof(short));
		//memcpy(UndoTemp.SelectTo,SelectTo,NCONVERSIONS*sizeof(short));
		// Update last constant
		//UndoTemp.ldConstant = ldConstant;
		// was CommitUndo from here:
		if (m_UndoIndex==MAXUNDO)
			memmove(&m_UndoInfo[0],&m_UndoInfo[1],(MAXUNDO-1)*sizeof(UndoInfoStruct));
		else
			m_UndoIndex++;
		m_UndoInfo[m_UndoIndex-1] = UndoTemp;
		if (!redone) m_Undoes = 0; // clear any further redo possibilities unless this is a redo
	}
}
