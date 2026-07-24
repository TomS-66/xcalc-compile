#include "xcalcconfig.h"
#include "ui_xcalcconfig.h"
#include "util.h"

XCalcConfig::XCalcConfig(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::XCalcConfig)
{
	ui->setupUi(this);
}

XCalcConfig::~XCalcConfig()
{
	delete ui;
}
