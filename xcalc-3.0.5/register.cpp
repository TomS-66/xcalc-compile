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

#include "register.h"
#include "xcalc.h"
#include "util.h"
#include "xcalcutil.h"
#include <stdlib.h>

const QString ctName(ContentType ct){
	if (ct==ctINTEGER) return "integer";
	else if (ct==ctFRACTION) return "fraction";
	else return "real/complex";
}

const QString ctnm(ContentType ct){
	if (ct==ctINTEGER) return "int";
	else if (ct==ctFRACTION) return "frac";
	else return "real";
}

Register::Register() {
	clear();
	reduce();
}

Register::Register(const Register &r) {
	if (r.isregint()) {
		m_ct = r.m_ct;
		m_Real = r.m_Real;
		m_Imag = r.m_Imag;
		m_i = r.m_i;
		m_num = r.m_num;
		m_den = r.m_den;
		m_DMS = r.m_DMS;
	} else if (r.isproperfrac()) {
		m_ct = r.m_ct;
		m_num = r.m_num;
		m_den = r.m_den;
		m_i = 0;
	} else {
		m_ct = ctCPX;
		m_Real = r.m_Real;
		m_Imag = r.m_Imag;
		m_i = 0;
		m_num = 0;
		m_den = 1;
		m_DMS = r.m_DMS;
	}
	reduce();
}

Register &Register::operator =(const Register &src){
	if (src.isregint()) {
		m_ct = src.m_ct;
		m_Real = src.m_Real;
		m_Imag = src.m_Imag;
		m_i = src.m_i;
		m_num = src.m_num;
		m_den = src.m_den;
		m_DMS = src.m_DMS;
	} else if (src.isproperfrac()) {
		m_ct = src.m_ct;
		m_num = src.m_num;
		m_den = src.m_den;
		m_i = 0;
	} else {
		m_ct = ctCPX;
		m_Real = src.m_Real;
		m_Imag = src.m_Imag;
		m_i = 0;
		m_num = 0;
		m_den = 1;
		m_DMS = src.m_DMS;
	}
	reduce();
	return *this;
}

Register::Register(LD v1, LD v2 ,bool DMS) {
	m_ct = ctCPX;
	m_Real = v1;
	m_Imag = v2;
	m_i = 0;
	m_num = 0;
	m_den = 1;
	m_DMS = DMS;
	reduce();
}

Register::Register(LC v, bool DMS) {
	m_ct = ctCPX;
	m_Real = v.real();
	m_Imag = v.imag();
	m_i = 0;
	m_num = 0;
	m_den = 1;
	m_DMS = DMS;
	reduce();
}

Register::Register(qint64 l) {
	m_ct = ctINTEGER;
	m_Real = 0;
	m_Imag = 0;
	m_i = l;
	m_num = 0;
	m_den = 1;
	m_DMS = false;
	reduce();
}

Register::Register(qint32 n, qint32 d, int f) {
	UNUSED(f);
	m_ct = ctFRACTION;
	m_Real = 0;
	m_Imag = 0;
	m_i = 0;
	m_num = n;
	m_den = d?d:1;
	m_DMS = false;
	reduce();
}

void Register::clear() {
	m_ct = ctINTEGER;
	m_Real = 0;
	m_Imag = 0;
	m_i = 0;
	m_num = 0;
	m_den = 1;
	m_DMS = false;
}

Register::operator LD() {
	return asfloat();
}

Register::operator LC() {
	return ascomplex();
}

LD Register::asfloat() {
	switch (m_ct) {
	case ctINTEGER: return (LD)(qint64)m_i;
	case ctFRACTION: return (LD)m_num/(LD)m_den;
	case ctCPX: return m_Real;
	default: return 42;
	}
}

LC Register::ascomplex() {
	switch (m_ct) {
	case ctINTEGER: return (LD)(qint64)m_i;
	case ctFRACTION: return (LD)m_num/(LD)m_den;
	case ctCPX: return LC(m_Real,m_Imag);
	default: return 42;
	}
}

LD Register::asreal() {
	switch (m_ct) {
	case ctINTEGER: return (qint64)m_i;
	case ctFRACTION: return (LD)m_num/(LD)m_den;
	case ctCPX: return m_Real;
	default: return 42;
	}
}

LD Register::asimag() {
	switch (m_ct) {
	case ctINTEGER: return 0;
	case ctFRACTION: return 0;
	case ctCPX: return m_Imag;
	default: return 42;
	}
}

qint64 Register::asint() {
	switch (m_ct) {
	case ctINTEGER: return (qint64)m_i;
	case ctFRACTION: return m_num/m_den;
	case ctCPX: return m_Real;
	default: return 42;
	}
}

qint32 Register::asnum() {
	if (isproperfrac()) return m_num;
	else return (qint32)asint();
}

qint32 Register::asden() {
	if (isproperfrac()) return m_den;
	else return 1;
}

void Register::epstest() {
	// check relative size of real/imag parts, zeroing out a too small one
	LD r = fabsl(m_Real);
	LD i  = fabsl(m_Imag);
	if (i<r && i/r<64*LDBL_EPSILON) m_Imag = 0;
	if (r<i && r/i<64*LDBL_EPSILON) m_Real = 0;
}

void Register::nantest() {
	if (!std::isfinite(m_Real) && std::isnan(m_Imag)) m_Imag=0; // inf,nan -> inf,0
}

qint64 Register::minregint(){
	switch (xcalc_wordLength) {
	case wl8BIT: return SCHAR_MIN;
	case wl16BIT: return SHRT_MIN;
	case wl32BIT: return INT_MIN;
	default: return LONG_LONG_MIN;
	}
}

qint64 Register::maxregint(){
	switch (xcalc_wordLength) {
	case wl8BIT: return SCHAR_MAX;
	case wl16BIT: return SHRT_MAX;
	case wl32BIT: return INT_MAX;
	default: return LONG_LONG_MAX;
	}
}

#ifdef __linux__
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif

void Register::reduce() {
	// Find correct number representation:
	// - Promote from fraction or real to int if possible (32 bit int value)
	// - Reduce if fraction,
	// - Choose first nonzero value using
	//   INTEGER, FRACTION, CPX
	//   in that order. (INTEGER if zero)
	// - Check real/imag relative sizes, zeroing a too small value
	// - Zero imag part if NaN (pow does that if real part->inf)
	// - Keep m_DMS if present.
	// - Let complex part always contain shadow copy of INTEGER or FRAC value.
	// Called automatically from constructor.

	if ((qint64)m_i!=0) m_ct = ctINTEGER;
	else if (m_num!=0) m_ct = ctFRACTION;
	else m_ct = ctCPX;

	if (m_ct==ctCPX && imagval()==0 && ::isint32(realval())) {
		m_i=(qint64)realval();
		m_ct=ctINTEGER;
	}
	if (m_ct==ctFRACTION) {
		qint32 n, d, r;

		// We could end up here with a number such as "1/-2", so we check
		if (m_den < 0) {
			m_den = -m_den;
			m_num = -m_num;
		}

		n = m_num;
		d = m_den;

		// Only come here with proper (nonzero) fractions.
		if (n == 0) { // then m_i is 0 as well. Number is INT
			m_Real=0;
			m_Imag=0;
			m_den = 1;
			m_ct = ctINTEGER;
			return; // No fraction. Should never happen
		}
		if (n<0) n = -n;

		while (d>0) {
			r = n%d;
			n = d;
			d = r;
		}

		m_num /= n;
		m_den /= n;

		if (m_den == 1) {
			m_i = m_num;
			m_Real = 0;
			m_Imag = 0;
			m_num = 0;
			m_ct = ctINTEGER;
		}
	}
	// Make shadow copy of ctINTEGER and ctFRACTION values (don't think it is needed)
	if (m_ct==ctINTEGER) m_Real = m_i;
	if (m_ct==ctFRACTION) m_Real = (LD)m_num/(LD)m_den;
	if (m_ct==ctCPX) { // eps check
		epstest(); // test relative real/imag size, zeroing out negligible value
		nantest(); // zero nan imag part
	}
} // reduce

bool Register::isproperint() const // zero considered int here (same as isint)
{
	return m_ct==ctINTEGER;
}

bool Register::isregint() const
{
	return isint() && isregintrange();
}

bool Register::isregintrange() const
{
	Register *t = const_cast<Register*>(this);
	return (qint64)t->m_i>=Register::minregint() && (qint64)t->m_i<=Register::maxregint();
}

bool Register::isint() const // zero considered int here
{
	return m_ct==ctINTEGER;
}

bool Register::isproperfrac() const
{
	return m_ct==ctFRACTION;
}

bool Register::isfrac() const // zero and int considered fractions here
{
	return isproperfrac() || isint();
}

bool Register::iseven() const
{
	return isint() && ((qint64)realval()&0x01)==0;
}

bool Register::iscomplex() const // zero considered complex here
{
	return m_ct==ctCPX || iszero();
}

bool Register::ispropercomplex() const // imag part !=0
{
	return m_ct==ctCPX && m_Imag!=0;
}

bool Register::iszero() const
{
	Register *t = const_cast<Register*>(this);
	return (t->m_ct==ctINTEGER&&(qint64)t->m_i==0);
}

bool Register::isnegative() const // and nonzero
{
	Register *t = const_cast<Register*>(this);
	if (t->m_ct==ctCPX) return t->imagval()==0.0 && t->realval()<0.0;
	else if (t->m_ct==ctFRACTION) return t->m_num<0;
	else return (qint64)t->m_i<0;
}

bool Register::ispositive() const // and nonzero
{
	Register *t = const_cast<Register*>(this);
	if (t->m_ct==ctCPX) return t->imagval()==0.0 && t->realval()>0.0;
	else if (t->m_ct==ctFRACTION) return t->m_num>0;
	else return (qint64)t->m_i>0;
}

bool Register::candms() const
{
	Register *t = const_cast<Register*>(this);
	LD a = (LD)*t;
	return a<=(LD)DMSMAXINT;
}

// Maths operations - here is the actual work!
// Note that there are no errors any more - invalid results are Inf or NaN.
// Binary results just saturate/wrap.

Register &Register::operator+=(Register &o) {
	if (((isproperfrac()&&o.isfrac())||(isfrac()&&o.isproperfrac()))
			&&::isint32((LD)asnum()*o.asden()+(LD)o.asnum()*asden())
			&&::isint32((LD)asden()*o.asden()))
		setfrac(asnum()*o.asden()+o.asnum()*asden(),asden()*o.asden());
	else
		setcpx(ascomplex()+o.ascomplex(),m_DMS||o.m_DMS);
	//set(asreal()+o.asreal(),asimag()+o.asimag(),m_DMS||o.m_DMS);
	return *this;
}

Register &Register::operator-=(Register &o) {
	if (((isproperfrac()&&o.isfrac())||(isfrac()&&o.isproperfrac()))
			&&::isint32((LD)asnum()*o.asden()-(LD)o.asnum()*asden())
			&&::isint32((LD)asden()*o.asden()))
		setfrac(asnum()*o.asden()-o.asnum()*asden(),asden()*o.asden());
	else
		setcpx(ascomplex()-o.ascomplex(),m_DMS||o.m_DMS);
	//set(asreal()-o.asreal(),asimag()-o.asimag(),m_DMS||o.m_DMS);
	return *this;
}

Register &Register::operator*=(Register &o) {
	if (((isproperfrac()&&o.isfrac())||(isfrac()&&o.isproperfrac()))
			&&::isint32((LD)asnum()*o.asnum())
			&&::isint32((LD)asden()*o.asden()))
		setfrac(asnum()*o.asnum(),asden()*o.asden());
	else
		setcpx(ascomplex()*o.ascomplex(),m_DMS);
	return *this;
}

Register &Register::operator/=(Register &o) {
	if (((isproperfrac()&&o.isfrac())||(isfrac()&&o.isproperfrac()))
			&&::isint32((LD)asnum()*o.asden())
			&&::isint32((LD)asden()*o.asnum()))
		setfrac(asnum()*o.asden(),asden()*o.asnum());
	else {
		setcpx(ascomplex()/o.ascomplex(),m_DMS);
	}
	return *this;
}

Register &Register::operator%=(Register &o) {
	// This makes little sense for complex numbers. (returns same as division)
	// TODO: Fractions could be done though.
	if (((isproperfrac()&&o.isfrac())||(isfrac()&&o.isproperfrac()))
			&&::isint32((LD)asnum()*o.asden())
			&&::isint32((LD)asden()*o.asnum()))
		setfrac(asnum()*o.asden(),asden()*o.asnum());
	else {
		setcpx(ascomplex()/o.ascomplex(),m_DMS);
	}
	return *this;
}

const Register Register::operator-() {
	Register r(*this);
	if (r.isproperfrac()&&(-r.asnum()==-(LD)r.asnum()))
		r.setfrac(-asnum(),asden());
	else if (r.isint())
		r.setlonglong(-r.asint());
	else
		r.setcpx(-ascomplex(),m_DMS);
	return r;
}

const Register Register::sqrt() {
	if (isproperfrac()) {
		LD newnum=asnum(); newnum=std::sqrt(newnum);
		LD newden=asden(); newden=std::sqrt(newden);
		if (::isint32(newnum) && ::isint32(newden)) {
			qint32 n=newnum;
			qint32 d=newden;
			return Register(n,d,1);
		}
	}
	LC t(ascomplex());
	return std::sqrt(t);
}

const Register Register::log() {
	LC t(ascomplex());
	return std::log(t);
}

const Register Register::exp() {
	LC t(ascomplex());
	return std::exp(t);
}

const Register Register::log10() {
	LC t(ascomplex());
	return std::log10(t);
}

const Register Register::ten() {
	/*
	LC t(10,0);
	LC x(ascomplex());
	return std::pow(t,x);
	*/
	return Register(10,0,1).pow(*this);
}

const Register Register::pow(Register &o) {
	Register y(*this);
	if (o.isint()) {
		qint32 x=o.asint();
		return intpow(y,x);
	} else {
		LC x(o.ascomplex());
		return std::pow(y.ascomplex(),x);
	}
}

const Register Register::root(Register &o) {
	LC y(ascomplex());
	if (o.isint()) {
		qint32 x=o.asint();
		return introot(y,x);
	} else {
		LC one(1,0);
		LC x(o.ascomplex());
		return std::pow(y,one/x);
	}
}

const Register Register::And(Register &o) {
	return asint()&o.asint();
}

const Register Register::Or(Register &o) {
	return asint()|o.asint();
}

const Register Register::Xor(Register &o) {
	return asint()^o.asint();
}

const Register Register::Not() {
	return ~asint();
}

const Register Register::sin() {
	return std::sin(ascomplex());
}

const Register Register::cos() {
	return std::cos(ascomplex());
}

const Register Register::tan() {
	return std::tan(ascomplex());
}

const Register Register::asin() {
	return xcasin(ascomplex());
}

const Register Register::acos() {
	return xcacos(ascomplex());
}

const Register Register::atan() {
	return xcatan(ascomplex());
}

const Register Register::sinh() {
	return std::sinh(ascomplex());
}

const Register Register::cosh() {
	return std::cosh(ascomplex());
}

const Register Register::tanh() {
	return std::tanh(ascomplex());
}

const Register Register::arsinh() {
	return xcarsinh(ascomplex());
}

const Register Register::arcosh() {
	return xcarcosh(ascomplex());
}

const Register Register::artanh() {
	return xcartanh(ascomplex());
}

const Register Register::abs(){
	if (ispropercomplex()) return std::abs(ascomplex());
	else if (isnegative()) return -*this;
	return *this;
}

const Register Register::rcp(){
	return Register(1,0,1)/(*this);
}

const Register Register::fact(){
	qint64 i=asint();
	LD m = 1.0;
	if (i<=FACT_MAX) while (i) m*=i--;
	return Register(m,0L,false);
}
const Register Register::random(){
	m_ct = ctCPX;
	m_Real = ((LD)rand())/((LD)RAND_MAX+1);
	m_Imag = 0;
	return *this;
}

const Register Register::shl(){
	Register ret(*this);
	switch (xcalc_wordLength) {
	case wl8BIT: ret.m_i = (quint8)((quint8)m_i.val<<1); break;
	case wl16BIT: ret.m_i = (quint16)((quint16)m_i.val<<1); break;
	case wl32BIT: ret.m_i = (quint32)((quint32)m_i.val<<1); break;
	case wl64BIT: ret.m_i = (quint64)((quint64)m_i.val<<1); break;
	}
	return ret;
}

const Register Register::shr(){
	Register ret(*this);
	switch (xcalc_wordLength) {
	case wl8BIT: ret.m_i = (quint8)((quint8)m_i.val>>1); break;
	case wl16BIT: ret.m_i = (quint16)((quint16)m_i.val>>1); break;
	case wl32BIT: ret.m_i = (quint32)((quint32)m_i.val>>1); break;
	case wl64BIT: ret.m_i = (quint64)((quint64)m_i.val>>1); break;
	}
	return ret;
}

const Register Register::ashl(){
	return shl();
}

const Register Register::ashr(){
	Register ret(*this);
	switch (xcalc_wordLength) {
	case wl8BIT: ret.m_i = (qint8)((qint8)m_i.val>>1); break;
	case wl16BIT: ret.m_i = (qint16)((qint16)m_i.val>>1); break;
	case wl32BIT: ret.m_i = (qint32)((qint32)m_i.val>>1); break;
	case wl64BIT: ret.m_i = (qint64)((qint64)m_i.val>>1); break;
	}
	return ret;
}

const Register Register::rotl(){
	Register ret(*this);
	switch (xcalc_wordLength) {
	case wl8BIT: ret.m_i = (quint8)(((quint8)m_i.val<<1)|((quint8)m_i.val>>7)); break;
	case wl16BIT: ret.m_i = (quint16)(((quint16)m_i.val<<1)|((quint16)m_i.val>>15)); break;
	case wl32BIT: ret.m_i = (quint32)(((quint32)m_i.val<<1)|((quint32)m_i.val>>31)); break;
	case wl64BIT: ret.m_i = (quint64)(((quint64)m_i.val<<1)|((quint64)m_i.val>>63)); break;
	}
	return ret;
}

const Register Register::rotr(){
	Register ret(*this);
	switch (xcalc_wordLength) {
	case wl8BIT: ret.m_i = (quint8)(((quint8)m_i.val>>1)|((quint8)m_i.val<<7)); break;
	case wl16BIT: ret.m_i = (quint16)(((quint16)m_i.val>>1)|((quint16)m_i.val<<15)); break;
	case wl32BIT: ret.m_i = (quint32)(((quint32)m_i.val>>1)|((quint32)m_i.val<<31)); break;
	case wl64BIT: ret.m_i = (quint64)(((quint64)m_i.val>>1)|((quint64)m_i.val<<63)); break;
	}
	return ret;
}

