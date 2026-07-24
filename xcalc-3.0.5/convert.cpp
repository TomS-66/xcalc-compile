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
//------------------------------------------------------------------------
// Conversion procedure
// Actions:
//	- opens conversion dialogue or quick converts
//	- sets ST[Xreg] to new value based on conversion
//------------------------------------------------------------------------

#include "xcalc.h"

// These from dialogs.c
extern LD ConvertedVal;
extern LD ConvertVal;

bool Convert(bool Quick)
{
	/*
	if (Quick) {
		ConvertVal = getreal(&ST[Xreg]);
		if (DoConversion()) {
			ST[Xreg].real = ConvertedVal;
			ST[Xreg].imag = 0;
			null(&ST[Xreg]);
			return FALSE;
		}
		else {
			Error = __RANGE;
			return TRUE;
		}
	}
	else {
		if (DialogBox(hInst,"XCALCCONVERT",hwndXCALC,ConvertDlgProc)) // true="OK"
		{
			ST[Xreg].real = ConvertedVal;
			ST[Xreg].imag = 0;
			null(&ST[Xreg]);
			return FALSE;
		}
		else
			return TRUE;
	}
	*/
	return Quick;
}
