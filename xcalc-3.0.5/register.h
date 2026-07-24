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

#ifndef REGISTER_H
#define REGISTER_H

#include "typedef.h"
#include <complex>

extern RadixType xcalc_radix;
extern WordLength xcalc_wordLength;
extern bool xcalc_cleanFrac;
extern bool xcalc_invTrigAsDMS;
extern QString xcalc_helpdir;
extern FixType xcalc_fixtype;
extern int xcalc_fix;
extern AngType xcalc_ang;
extern bool xcalc_copytop;
extern bool xcalc_showtype;

const qint64 MASK8 = 0xff;
const qint64 MASK16 = 0xffff;
const qint64 MASK32 = 0xffffffff;

enum ContentType {
	// preferred order (first nonzero counts) is ctINTEGER, ctFRACTION, ctCPX,
	// INTEGER if zero
	ctINTEGER,
	ctFRACTION,
	ctCPX
};

const QString ctName(ContentType ct);
const QString ctnm(ContentType ct);

#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif

struct intType {
	qint64 val;
public:
	intType(const intType &i) {
		switch(xcalc_wordLength) {
		case wl64BIT: val = i.val; break;
		case wl32BIT: val = (qint32)(i.val&MASK32); break;
		case wl16BIT: val = (qint16)(i.val&MASK16); break;
		case wl8BIT: val = (qint8)(i.val&MASK8); break;
		}
	}
	intType &operator=(const intType &src) {
		switch(xcalc_wordLength) {
		case wl64BIT: val = src.val; break;
		case wl32BIT: val = (qint32)(src.val&MASK32); break;
		case wl16BIT: val = (qint16)(src.val&MASK16); break;
		case wl8BIT: val = (qint8)(src.val&MASK8); break;
		}
		return *this;
	}

	void clr() {
		val = 0;
	}

	intType(qint64 v=0) {
		switch(xcalc_wordLength) {
		case wl64BIT: val = v; break;
		case wl32BIT: val = (qint32)(v&MASK32); break;
		case wl16BIT: val = (qint16)(v&MASK16); break;
		case wl8BIT: val = (qint8)(v&MASK8); break;
		}
	}
	// Problem here: it is always difficult to mix signed/unsigned -
	// I want rotates and resizing to word size to be unsigned,
	// and conversion to/from decimal numbers signed. Solved now? Check it.
	//
	operator qint64() {
		/*
		switch (xcalc_wordLength) {
		case wl8BIT: return qint8(val&MASK8);
		case wl16BIT: return qint16(val&MASK16);
		case wl32BIT: return qint32(val&MASK32);
		default: return val;
		}
		*/
		return val;
	}
	operator quint64() {
		switch (xcalc_wordLength) {
		case wl8BIT: return quint8(val&MASK8);
		case wl16BIT: return quint16(val&MASK16);
		case wl32BIT: return quint32(val&MASK32);
		default: return quint64(val);
		}
	}
	operator LD() { return (LD)(qint64)*this; }
	operator LC() { return (LC)(qint64)*this; }
};

#ifdef __linux__
#pragma GCC diagnostic pop
#endif

class Register {
public:
	Register();
	Register(const Register&);
	Register &operator =(const Register &src);
	Register(LD v1, LD v2=0,bool DMS=false);
	Register(qint64 l);
	Register(qint32 n, qint32 d,int f);
	Register(LC v,bool DMS=false);
	Register &setv1v2DMS(LD v1, LD v2=0, bool DMS=false) {Register r(v1,v2,DMS); return *this = r; }
	Register &setlonglong(qint64 l) {Register r(l); return *this = r; }
	Register &setfrac(qint32 n,qint32 d) {Register r(n,d,1); return *this = r; }
	Register &setcpx(LC v, bool DMS=false) {Register r(v,DMS); return *this = r;}
	void clear();
	operator LD();
	operator LC();
	LD realval() const { return m_Real; }
	LD imagval() const { return m_Imag; }
	qint64 ival() const { return m_i.val; }
	qint64 numval() const { return m_num; }
	qint64 denval() const { return m_den; }
	void reduce();
	void epstest();
	void nantest();
	bool isint() const;
	bool isregint() const;
	bool isregintrange() const;
	bool isfrac() const;
	bool isproperfrac() const;
	bool isproperint() const;
	bool iseven() const;
	bool iscomplex() const;
	bool ispropercomplex() const;
	bool iszero() const;
	bool isnegative() const;
	bool ispositive() const;
	bool isdms() const { return m_DMS && candms(); }
	bool candms() const;
	LC ascomplex() ;
	LD asfloat();
	LD asreal();
	LD asimag();
	qint64 asint();
	qint32 asnum();
	qint32 asden();
	ContentType ct() { return m_ct; }
	void setDMS(bool v) { m_DMS=v; }
	// Maths functions (slowly, but they'll all be there!)
	Register &operator+=(Register &o);
	const Register operator+(Register &o) { return Register(*this)+=o; }
	Register &operator-=(Register &o);
	const Register operator-(Register &o) { return Register(*this)-=o; }
	Register &operator*=(Register &o);
	const Register operator*(Register &o) { return Register(*this)*=o; }
	Register &operator/=(Register &o);
	const Register operator/(Register &o) { return Register(*this)/=o; }
	Register &operator%=(Register &o);
	const Register operator%(Register &o) { return Register(*this)%=o; }
	const Register operator-();
	const Register sqrt();
	const Register log();
	const Register exp();
	const Register log10();
	const Register ten();
	const Register sin();
	const Register cos();
	const Register tan();
	const Register asin();
	const Register acos();
	const Register atan();
	const Register sinh();
	const Register cosh();
	const Register tanh();
	const Register arsinh();
	const Register arcosh();
	const Register artanh();
	const Register pow(Register &o);
	const Register root(Register &o);
	const Register And(Register &o);
	const Register Or(Register &o);
	const Register Xor(Register &o);
	const Register Not();
	const Register abs();
	const Register rcp();
	const Register fact();
	const Register random();
	const Register shl();
	const Register shr();
	const Register ashl();
	const Register ashr();
	const Register rotl();
	const Register rotr();
	static WordLength wordLength() { return xcalc_wordLength; }
	static void setWordLength(WordLength wl) { xcalc_wordLength=wl; }
	static bool cleanFrac() { return xcalc_cleanFrac; }
	static void setCleanFrac(bool cf) { xcalc_cleanFrac=cf; }
	static qint64 minregint();
	static qint64 maxregint();
private:
	ContentType m_ct;
	LD m_Real, m_Imag;
	intType m_i;
	qint32 m_num,m_den;
	bool m_DMS; // for display
};

#endif // REGISTER_H
