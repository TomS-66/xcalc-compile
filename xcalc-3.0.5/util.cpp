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
#include "aschar.h"
#include "typedef.h"
#include <time.h>
#include <cstdlib>
#include <cstdio>
#include <complex>

// Convenience declarations
// Note: BC5.0 does not make proper LD values from decimal digits -
// therefore, we use bit patterns from BC3.1... (similar to M_PI_L)

//-----------------------------------------------------------------------
// Various functions for xcalc
//-----------------------------------------------------------------------
QString chext(QString name, QString ext)
{
	int dot = name.lastIndexOf(EXTSEP);
	int sla = name.lastIndexOf(PATHSEP);
	if (sla<0) sla = name.lastIndexOf(PATHSEP2);
	if (dot>=0 && dot>sla)
		name=name.left(dot);
	return name+EXTSEP+ext;
}

QString nodir(QString name)
{
	int sla = name.lastIndexOf(PATHSEP);
	if (sla<0) sla = name.lastIndexOf(PATHSEP2);
	if (sla<0) return name;
	else return name.mid(sla+1);
}

QString appName()
{
	return nodir(appPath());
}

QString justdir(QString name)
{
	int sla = name.lastIndexOf(PATHSEP);
	if (sla<0) sla = name.lastIndexOf(PATHSEP2);
	if (sla<0) return name;
	else return name.left(sla+1);
}

QString appPath()
{
	// if linux, look up the /proc/<PID>/exe link.
#if defined __linux__
	QString link;
	char spath[128];
	memset(spath,0,128);
	link.sprintf("/proc/%d/exe",getpid());
	if (readlink(link.toUtf8().constData(),spath,128)>0) return spath;
	return "";
#elif defined __WIN32__
	return g_appPath;
#endif
}
