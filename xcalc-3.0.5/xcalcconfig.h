#ifndef XCALCCONFIG_H
#define XCALCCONFIG_H

#include <QDialog>

namespace Ui {
	class XCalcConfig;
}

class XCalcConfig : public QDialog
{
	Q_OBJECT

public:
	explicit XCalcConfig(QWidget *parent = 0);
	~XCalcConfig();

private:
	Ui::XCalcConfig *ui;
};

#endif // XCALCCONFIG_H
