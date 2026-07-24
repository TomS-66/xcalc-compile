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

#ifndef QENGINE_H
#define QENGINE_H

#include <QObject>
#include "xcalc.h"

class Register;

class QEngine : public QObject
{
	Q_OBJECT
public:
	explicit QEngine(QObject *parent = 0);

signals:
	void changed();
	void radixchanged();
public slots:
	void docommand(funid k); // sends c to xcalc and calls update
public:
	xcalc *m_xengine;
	void update(); // emits changed signal
	void newradix(); // emits radixchanged signal
};

#endif // QENGINE_H
