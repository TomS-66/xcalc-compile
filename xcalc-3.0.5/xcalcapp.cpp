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
#include "qengine.h"

XCalcApp::XCalcApp(int argc, char *argv[]):QApplication(argc,argv) {
	m_qengine = new QEngine(this);
	m_profile = new Profile(appName(),0);
	ReadProfile();

}

XCalcApp::~XCalcApp() {
	SaveProfile();
	delete m_profile;
}

void XCalcApp::ReadProfile() {
	xcalc_wordLength = (WordLength)m_profile->readInt("WordLength",xcalc_wordLength);
	xcalc_radix = (RadixType)m_profile->readInt("Radix",xcalc_radix);
	xcalc_cleanFrac = m_profile->readBool("CleanFrac",xcalc_cleanFrac);
	xcalc_invTrigAsDMS = m_profile->readBool("InvTrigAsDMS",xcalc_invTrigAsDMS);
	xcalc_fixtype =  (FixType)m_profile->readInt("FixType",xcalc_fixtype);
	xcalc_fix = m_profile->readInt("Fix",xcalc_fix);
	xcalc_copytop = m_profile->readBool("CopyTop",xcalc_copytop);
	xcalc_showtype = m_profile->readBool("ShowType",xcalc_showtype);
}

void XCalcApp::SaveProfile() {
	m_profile->write("WordLength",(int)xcalc_wordLength);
	m_profile->write("Radix",(int)xcalc_radix);
	m_profile->write("CleanFrac",xcalc_cleanFrac);
	m_profile->write("InvTrigAsDMS",xcalc_invTrigAsDMS);
	m_profile->write("FixType",(int)xcalc_fixtype);
	m_profile->write("Fix",xcalc_fix);
	m_profile->write("CopyTop",xcalc_copytop);
	m_profile->write("ShowType",xcalc_showtype);
}
