#ifndef XCALCUTIL_H
#define XCALCUTIL_H

#include <QString>
#include "typedef.h"
#include "register.h"

bool isint32(LD v);
bool isregint(const LD &v);
bool isregintrange(const LD &v);
bool candms(const LD &v);
char *newasutf8(QString s);
long toLongRobust(QString s, bool &ok);
LD toLDRobust(QString s, bool &ok);
Register intpow(Register y,qint32 x);
LC introot(LC y,qint32 x);
char hexdigit(uint d);
QString snumber(qint64 v, WordLength wl, RadixType r); // instead of QString::number, with signed ints of specified wordlength
LD getreal(LC v);
LD getimag(LC v);
bool isreal(LC v);
QString quicknum(Register &r);
LD RADTOANG(LD a);
LD ANGTORAD(LD a);


// file/help etc.
void SetHelpDirDflt(); // gets application dir and adds html/
void SetHelpDir(QString dir); // set another dir
void ShowHelp(QString file);
void SendMail(QString rcpt,QString subject);

// missing std::functions - solved before, so I do it again... Easier this time, with proper complex arithmetic!
LC xcasin(LC x);
LC xcacos(LC x);
LC xcatan(LC x);
LC xcarsinh(LC x);
LC xcarcosh(LC x);
LC xcartanh(LC x);

// system constants

const LC j = LC(0,1);
const LC one = LC(1,0);
const LC two = LC(2,0);
extern QString g_helpdir;

#endif // XCALCUTIL_H
