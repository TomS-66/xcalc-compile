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

//------------------------------------------------------------------------
// File for project XCALC
//-----------------------------------------------------------------------
// Various functions
//-----------------------------------------------------------------------

#include "util.h"
#include "xcalcutil.h"
#include "xcalc.h"
#include "aschar.h"
#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <complex>

// Convenience declarations
// Note: BC5.0 does not make proper LD values from decimal digits -
// therefore, we use bit patterns from BC3.1... (similar to M_PI_L)

#ifdef __HAVELD__
//const char radsindeg[] = {0xae,0xc8,0xe9,0x94,0x12,0x35,0xfa,0x8e,0xf9,0x3f};
//const char degsinrad[] = {0xc3,0xbd,0x0f,0x1e,0xd3,0xe0,0x2e,0xe5,0x04,0x40};
//const char m_pi[] = {0x35,0xc2,0x68,0x21,0xa2,0xda,0x0f,0xc9,0x00,0x40};
//const LD * const M_PI_L_PTR = (LD*)m_pi;
#endif

//-----------------------------------------------------------------------
// Various functions for xcalc
//-----------------------------------------------------------------------
LD getreal(LC v) {
	return v.real();
}

LD getimag(LC v) {
	return v.imag();
}

bool isreal(LC v) {
	return v.imag()==0;
}

bool isregint(const LD &v)
{
	return isregintrange(v) && v == (LD)(qint64)v;
}

bool isregintrange(const LD &v)
{
	return v>=Register::minregint() && v<=Register::maxregint();
}

bool isint32(LD v)
{
	return v==(qint32)v;
}

bool candms(const LD &v)
{
	LD a = floorl(fabsl(v));
	if (a>(LD)DMSMAXINT) return FALSE;
	return TRUE;
}

LD _hypot(const LD &x,const LD &y)
{
	return sqrt(x*x+y*y);
}

LD _hypotsq(const LD &x,const LD &y)
{
	return x*x+y*y;
}

long toLongRobust(QString s, bool &ok) // ok if any characters are used, or empty input (res=0)
{
	char *u = newasutf8(s);
	char *stop;
	long l = std::strtol(u,&stop,10);
	ok = s.isEmpty() || stop>u;
	delete[] u;
	return l;
}

LD toLDRobust(QString s, bool &ok) // ok if any characters are used, or empty input (res=0)
{
	if (s==".") { ok=true; return 0; }
	char *u = newasutf8(s);
	char *stop;
	LD ld = std::strtold(u,&stop);
	ok = s.isEmpty() || stop>u;
	delete[] u;
	return ld;
}

//-----------------------------------------------------------------------
// Update 25/3/97: accept large negative power.
// Update 14/4/12: using register, not LC (should make fractions work)
//-----------------------------------------------------------------------
Register intpow(Register y,qint32 x)
{
	if (y.iszero() && x==0) {
		return Register(NAN,NAN,false);
	}
	Register prod = y.iszero()?Register(0,1,0):Register(1,0,0);	// initialise result to one unless y==0
	Register temp = y;				// temp for squaring where possible
	bool neg = (x<0);				// negative integer power
	x = abs(x);

	while (x) {
		// if odd power, multiply product with temp and decrement power by one
		if (x&1) {
			prod*=temp; // no range checking yet
			x--;
		}
		if (!x) break;		// done
		// now power is even - square temp and half power
		temp*=temp; // still no range checking
		x>>=1;
	}
	// result in prod. invert if negative power
	if (neg) {
		Register o = Register(1,0);
		return o/prod;
	}
	else {
		return prod;
	}
}

LC introot(LC y,qint32 x)
{
	//-----------------------------------------------------------------------
	// does not work with fractions
	//-----------------------------------------------------------------------
	Register tmp(0L,0L,false);
	LD radius,angle;

	if (x==0)
		return NAN; // zero'th root is singularity

	if (y==LC(0,0)) {
		if (x<=0) return NAN; // neg root of zero is sing
		return 0; // pos root of zero is zero
	}

	if (isreal(y)) {
		// y is real: find correct length of y's radius, and set angle
		// depending on if x is odd (same angle) or even (divide angle by x)
		// therefore, odd roots never deliver complex results with real input.
		radius=abs(getreal(y));
		radius=pow(radius,1.0L/x);
		angle=atan2(getimag(y),getreal(y));
		if (!(x&1)) {
			tmp.setv1v2DMS(radius * cos(angle/x),radius * sin(angle/x),false);
		}
		else {
			tmp.setv1v2DMS(radius * cos(angle),radius * sin(angle),false);
		}
	}
	else {
		// y is complex: use std's routine
		tmp = std::pow(y,1.0L/x);
	}

	return tmp;
}

char hexdigit(uint d) {
	return d>9?d-10+'a':d+'0';
}

QString snumber(qint64 v, WordLength wl, RadixType r) // instead of QString::number, with unsigned ints of specified wordlength
{
	QString s;
	char b65[65],*p=b65+64;
	memset(b65,0,65);
	quint8 i8=v&MASK8;
	quint16 i16=v&MASK16;
	quint32 i32=v&MASK32;
	quint64 i64=v;
	switch(wl) {
	case wl8BIT:
		if (r==rtHEX) {
			if (!i8) *--p='0';
			else while (i8) {
				*--p=hexdigit(i8&0xf);
				i8>>=4;
			}
		} else if (r==rtOCTAL) {
			if (!i8) *--p='0';
			else while (i8) {
				*--p=hexdigit(i8&0x7);
				i8>>=3;
			}
		} else if (r==rtBINARY) {
			if (!i8) *--p='0';
			else while (i8) {
				*--p=hexdigit(i8&1);
				i8>>=1;
			}
		}
		break;
	case wl16BIT:
		if (r==rtHEX) {
			if (!i16) *--p='0';
			else while (i16) {
				*--p=hexdigit(i16&0xf);
				i16>>=4;
			}
		} else if (r==rtOCTAL) {
			if (!i16) *--p='0';
			else while (i16) {
				*--p=hexdigit(i16&0x7);
				i16>>=3;
			}
		} else if (r==rtBINARY) {
			if (!i16) *--p='0';
			else while (i16) {
				*--p=hexdigit(i16&1);
				i16>>=1;
			}
		}
		break;
	case wl32BIT:
		if (r==rtHEX) {
			if (!i32) *--p='0';
			else while (i32) {
				*--p=hexdigit(i32&0xf);
				i32>>=4;
			}
		} else if (r==rtOCTAL) {
			if (!i32) *--p='0';
			else while (i32) {
				*--p=hexdigit(i32&0x7);
				i32>>=3;
			}
		} else if (r==rtBINARY) {
			if (!i32) *--p='0';
			else while (i32) {
				*--p=hexdigit(i32&1);
				i32>>=1;
			}
		}
		break;
	case wl64BIT:
		if (r==rtHEX) {
			if (!i64) *--p='0';
			else while (i64) {
				*--p=hexdigit(i64&0xf);
				i64>>=4;
			}
		} else if (r==rtOCTAL) {
			if (!i64) *--p='0';
			else while (i64) {
				*--p=hexdigit(i64&0x7);
				i64>>=3;
			}
		} else if (r==rtBINARY) {
			if (!i64) *--p='0';
			else while (i64) {
				*--p=hexdigit(i64&1);
				i64>>=1;
			}
		}
	}
	s = p;
	return s;
}

void SetHelpDirDflt() {
	// set help directory as html relative to application
	xcalc_helpdir = justdir(appPath());
	if (!xcalc_helpdir.endsWith(PATHSEP))
		xcalc_helpdir += PATHSEP;
	xcalc_helpdir += "html"+PATHSEP;
}

void SetHelpDir(QString dir) // set another dir
{
	xcalc_helpdir = dir;
}

void ShowHelp(QString file)
{
	QString path = xcalc_helpdir + file;
#if defined __linux
	QString HELP = "xdg-open";

	pid_t pid;
	char *args[3];

	char *cmd = new char[HELP.toLatin1().length()+1];
	strcpy(cmd,HELP.toLatin1().constData());
	args[0] = cmd;
	args[1] = new char[path.toLatin1().length()+1];
	strcpy(args[1],path.toLatin1().constData());

	args[2]=0;
	pid=fork();
	if(!pid)
		execvp(cmd,args); // really weird API - duplicate cmd and args[0], or am I mistaken?
	delete cmd;
	delete args[1];
#elif defined __WIN32__
	// we're running windows - shell execute should do it
	ShellExecute(NULL,L"open",path.toStdWString().c_str(),NULL,NULL,SW_SHOWNORMAL);
#endif
}

void SendMail(QString rcpt,QString subject)
{
#if defined __linux
	// Only works with thunderbird as client!
	// If thunderbird is there, this will work:
	// thunderbird -compose "to='<rcpt>',subject='<subject>'"
	// (double quotes needed for shell)
	QString MAIL = "thunderbird";
	QString arg1 = "-compose";
	QString arg2 = QString("to='%1',subject='%2'").arg(rcpt,subject);

	pid_t pid;
	char *args[4];

	char *cmd = new char[MAIL.toLatin1().length()+1];
	strcpy(cmd,MAIL.toLatin1().constData());
	args[0] = cmd;
	args[1] = new char[arg1.toLatin1().length()+1];
	args[2] = new char[arg2.toLatin1().length()+1];
	strcpy(args[1],arg1.toLatin1().constData());
	strcpy(args[2],arg2.toLatin1().constData());

	args[3]=0;
	pid=fork();
	if(!pid)
		execvp(cmd,args); // really weird API - duplicate cmd and args[0], or am I mistaken?
	delete cmd;
	delete args[1];
	delete args[2];
#elif defined __WIN32__
	// we're running windows - shell execute should do it
	QString command = "mailto:";
	ShellExecute(NULL,command.toStdWString().c_str(),rcpt.toStdWString().c_str(),NULL,NULL,SW_SHOWNORMAL);
#endif
}

QString quicknum(Register &r) {
	if (r.isproperfrac()) return QString("%1/%2").arg(r.numval()).arg(r.denval());
	else if (r.isint()) return QString::number(r.asint());
	else if (r.ispropercomplex()) return QString("%1 +%2i").arg((double)r.asreal()).arg((double)r.asimag());
	else return QString::number((double)r.asreal());
}

LC xcasin(LC z) {
	return -j*std::log(j*z+sqrt(one-z*z));
}

LC xcacos(LC z) {
	return -j*std::log(z+sqrt(z*z-one));
}

LC xcatan(LC z) {
	return j/two*(std::log(one-j*z)-(std::log(one+j*z)));
}

LC xcarsinh(LC x) {
	return std::log(x+sqrt(x*x+one));
}

LC xcarcosh(LC x) {
	return std::log(x+sqrt(x+one)*sqrt(x-one));
}

LC xcartanh(LC x) {
	return (log(one+x)-log(one-x))/two;
}

LD ANGTORAD(LD a) {
	if (xcalc_radix==rtDECIMAL && xcalc_ang==atDEGREE) return a*180/M_PIl;
	return a;
}

LD RADTOANG(LD a) {
	if (xcalc_radix==rtDECIMAL && xcalc_ang==atDEGREE) return a*M_PIl/180;
	return a;
}
