#include "profile.h"
#include "aschar.h"
#include "util.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

extern QTextStream qcout;
using namespace std;

bool exists(const char * filename)
{
	if (FILE* file = fopen(filename, "r")) {
		fclose(file);
		return true;
	}
	return false;
}

Profile::Profile(QString appnm, QWidget *w) : m_pw(w)
{
	char *cwd = getcwd(0,0);
	QString wd = cwd;
	QString appname = appnm;
	free(cwd);
	if (!wd.endsWith(PATHSEP)) wd+=PATHSEP;
	QString pathapp = wd+nodir(appname);
	m_inifile = chext(pathapp,"ini");
	m_SSSfile = chext(pathapp,"$$$");
	m_bakfile = chext(pathapp,"bak");
	m_ininame=newasutf8(m_inifile);
	m_bakname=newasutf8(m_bakfile);
	m_SSSname=newasutf8(m_SSSfile);
	m_changed = FALSE;
	// Populate with file contents
	FILE *ifil = fopen(m_ininame,"r");
	QString f;
	if (ifil) {
		f.sprintf("(Profile::Profile) using inifile %s",m_ininame);
		qcout << "(Profile::Profile) opening for read " << m_ininame << endl;
		QTextStream ini(ifil);
		ini.setCodec("UTF-8");
		while (!ini.atEnd()) {
			QString kv = ini.readLine(Kb); // now this can read utf-8 data
			int eq = kv.indexOf('=');
			if (eq>0) {
				QString k = kv.left(eq);
				QString v = kv.mid(eq+1);
				k = k.simplified();
				v = v.simplified();
				m_strings[k] = v;
				m_changedvalue[k] = false;
			}
		}
		ini.setDevice(0);
		fclose(ifil);
	} else {
		f.sprintf("(Profile::Profile) NEW inifile %s",m_ininame);
		qcout << "(Profile::Profile) new file " << m_ininame << endl;
		m_changed = true;
	}
	//QMessageBox::information(m_pw,"Wordup info",f,0,0,0);
}

Profile::~Profile()
{
	if (m_changed) {
		qcout << "(Profile::~Profile) changed inifile:\n";
		QList<QString> allkeys = m_strings.keys();
		for (int i = 0; i < allkeys.size(); ++i) {
			QString key = allkeys[i];
			if (!m_changedvalue.contains(key) || m_changedvalue.value(key))
				qcout << "\t" << key.toAscii().constData() << endl;
		 }
		// Write changed content to tmpfile
		FILE *tfil = fopen(m_SSSname,"w");
		if (tfil) {
			qcout << "(Profile::~Profile) writing " << m_SSSname << endl;
			QTextStream tmp(tfil);
			tmp.setCodec("UTF-8");
			QHashIterator<QString,QString> i(m_strings);
			while (i.hasNext()) {
				i.next();
				QString namevalue = i.key()+"="+i.value();
				tmp << namevalue << endl;
			}
			tmp.flush();
			tmp.setDevice(0);
			fclose(tfil);
		} else {
			QString f;
			f.sprintf("(Profile::~Profile) Weird. Cannot open %s for writing!",m_SSSname);
			QMessageBox::information(m_pw,"Error",f,0,0,0);
		}
		// involve m_bakfile as well
		if (exists(m_bakname)) {
			remove(m_bakname);
			qcout << "old " << m_bakname << " removed\n";
		}
		if (exists(m_ininame)) {
			rename(m_ininame,m_bakname);
			qcout << "renamed " << m_ininame << "->" << m_bakname << endl;
		}
		rename(m_SSSname,m_ininame);
		qcout << "renamed " << m_SSSname << "->" << m_ininame << endl;
	} else {
		qcout << "(Profile::~Profile) unchanged inifile\n";
	}
	m_strings.clear();
	delete[] m_ininame;
	delete[] m_bakname;
	delete[] m_SSSname;
}

void Profile::read(QString name, QString &s, QString def)
{
	readstring(name,s,def);
}

void Profile::read(QString name, int &i, int def)
{
	QString sval;
	QString sdef = QString("%1").arg(def);
	readstring(name,sval,sdef);
	i = sval.toInt();
}

void Profile::read(QString name, bool &b, bool def)
{
	QString sval;
	QString sdef = QString("%1").arg((int)def);
	readstring(name,sval,sdef);
	b = (bool)sval.toInt();
}

QString Profile::readString(QString name, QString def)
{
	QString s;
	read(name,s,def);
	return s;
}

int Profile::readInt(QString name,int def)
{
	int i;
	read(name,i,def);
	return i;
}

bool Profile::readBool(QString name,bool def)
{
	bool b;
	read(name,b,def);
	return b;
}

void Profile::write(QString name, QString s)
{
	writestring(name,s);
}

void Profile::write(QString name, int i)
{
	writestring(name,QString("%1").arg(i));
}

void Profile::write(QString name, bool b)
{
	writestring(name,QString("%1").arg((int)b));
}

//private:

bool Profile::readstring(QString name,QString &value,QString def)
{
	name = name.simplified();
	if (m_strings.contains(name)) {
		value = m_strings.value(name);
		return true;
	}
	 // new item added to application, not yet in ini file (or new ini file)
	value = def;
	m_strings[name] = value;
	m_changedvalue[name] = true;
	m_changed = true;
	return false;
}

void Profile::writestring(QString name,QString value)
{
	name = name.simplified();
	value = value.simplified();
	if (m_strings.contains(name)) {
		m_changed |= value != m_strings.value(name);
		m_changedvalue[name] |= value != m_strings.value(name); // |= since name may be new (added in read)
	} else {
		m_changed = true;
		m_changedvalue[name] = true;
	}
	m_strings[name] = value;
}
