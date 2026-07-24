#ifndef PROFILE_H
#define PROFILE_H

#include <QString>
#include <QHash>
#include "util.h"

#define Kb 1024

class Profile
{
public:
	Profile(QString appnm, QWidget *);
	~Profile();
	void read(QString name, QString &s, QString def = "");
	void read(QString name, int &i, int def = 0);
	void read(QString name, bool &b, bool def = false);
	QString readString(QString name, QString def = "");
	int readInt(QString name, int def = 0);
	bool readBool(QString name, bool def = false);
	void write(QString name,QString s);
	void write(QString name,int i);
	void write(QString name,bool b);
private:
	QWidget *m_pw;
	QString m_inifile;
	QString m_SSSfile;
	QString m_bakfile;
	char *m_ininame;
	char *m_SSSname;
	char *m_bakname;
	bool m_changed;
	QHash<QString,QString> m_strings;
	QHash<QString,bool> m_changedvalue;
	bool readstring(QString name, QString &value, QString def);
	void writestring(QString name,QString s);
};

#endif // PROFILE_H
