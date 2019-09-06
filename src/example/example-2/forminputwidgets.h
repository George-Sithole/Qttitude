#ifndef FORMINPUTWIDGETS_H
#define FORMINPUTWIDGETS_H

#include <QWidget>

namespace Ui {
class FormInputWidgets;
}

class FormInputWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit FormInputWidgets(QWidget *parent = nullptr);
    ~FormInputWidgets();

private:
    Ui::FormInputWidgets *ui;
};

#endif // FORMINPUTWIDGETS_H
