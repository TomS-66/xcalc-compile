#ifndef UTIL_H
#define UTIL_H

#include <QString>

QString chext(QString name, QString ext);
QString appName();
QString nodir(QString name);
QString justdir(QString name);
QString appPath();

#if defined __WIN32__
const QString PATHSEP = "\\";
const QString PATHSEP2 = "/";
const QString EXTSEP = ".";
extern QString g_appPath;
#else
const QString PATHSEP = "/";
const QString PATHSEP2 = "/";
const QString EXTSEP = ".";
#endif

#endif // UTIL_H
