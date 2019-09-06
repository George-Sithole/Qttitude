#ifndef FORMBUTTONS_H
#define FORMBUTTONS_H

#include <QWidget>

namespace Ui {
class FormButtons;
}

class FormButtons : public QWidget
{
    Q_OBJECT

public:
    explicit FormButtons(QWidget *parent = nullptr);
    ~FormButtons();

private:
    Ui::FormButtons *ui;
};

#endif // FORMBUTTONS_H
