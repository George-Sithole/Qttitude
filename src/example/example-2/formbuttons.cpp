#include "formbuttons.h"
#include "ui_formbuttons.h"

FormButtons::FormButtons(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormButtons)
{
    ui->setupUi(this);
}

FormButtons::~FormButtons()
{
    delete ui;
}
