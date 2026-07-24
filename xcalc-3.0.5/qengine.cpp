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
#include "aschar.h"

QEngine::QEngine(QObject *parent) :
	QObject(parent)
{
	m_xengine = new xcalc(this);
}

void QEngine::docommand(funid k)
{
	m_xengine->func(k);
	update(); // not needed - done at end of xcalc::func
	if (k==FUN_DECIMAL || k==FUN_COMPLEX || k==FUN_HEX ||  k==FUN_OCTAL ||  k==FUN_BINARY) {
		// this is needed for redo, which is triggered in xcalcwindow. xengine knows nothing about views
		newradix();
	}
}

void QEngine::update() {
	emit changed();
}

void QEngine::newradix() {
	emit radixchanged();
}
