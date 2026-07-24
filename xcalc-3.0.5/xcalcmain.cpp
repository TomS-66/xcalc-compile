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

#include <QtGui/QApplication>
#include "xcalcwindow.h"
#include "xcalc.h"
#include "xcalcrc.h"
#include "qengine.h"
#include <QMessageBox>

/*
// This doesn't work at all under Linux.

extern "C" void fpsigfunc(int sig)
{
		//qcout << "FP signal received:" << sig << endl;
		QString msg = QString("FP signal received: %1").arg(sig);
		QMessageBox::warning(0,"FP signal",msg);
}
*/

int main(int argc,char *argv[])
{
	//Doesn't work under Linux
	//struct sigaction act,oact;
	//act.sa_handler=fpsigfunc;
	//sigaction(SIGFPE,&act,&oact);
	qcout << "Starting xcalc " << XCALCVERSION << endl;
	XCalcApp a(argc, argv);

	xcalc_win = new XCALCWindow();
	xcalc_win->show();
	// does a second window work?
	//XCALCWindow *w2 = new XCALCWindow();
	//w2->show();

	a.m_qengine->update();
	return a.exec();
}

