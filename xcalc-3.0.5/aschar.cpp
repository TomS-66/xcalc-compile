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

#include <qstring.h>

// NOTE: always delete[] the returned pointer whan finished...

char *newasutf8(QString q)
{
	QByteArray ba = q.toUtf8();
	char *buf = new char[ba.size()+1];
	strcpy(buf,ba.data());
	return buf;
}

char *newasascii(QString q)
{
	// ascii only, please
	QByteArray ba = q.toAscii();
	char *buf = new char[ba.size()+1];
	strcpy(buf,ba.data());
	return buf;
}

char *newaslatin1(QString q)
{
	// latin only, please
	QByteArray ba = q.toLatin1();
	char *buf = new char[ba.size()+1];
	strcpy(buf,ba.data());
	return buf;
}

wchar_t *newaswchar(QString q)
{
	wchar_t *wc=new wchar_t[q.size()+1];
	memset(wc,0,(q.size()+1)*sizeof(wchar_t));
	q.toWCharArray(wc);
	return wc;
}
