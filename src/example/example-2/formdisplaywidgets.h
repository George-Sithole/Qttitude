#ifndef FORMDISPLAYWIDGETS_H
#define FORMDISPLAYWIDGETS_H

#include <QWidget>

namespace Ui {
class FormDisplayWidgets;
}

class FormDisplayWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit FormDisplayWidgets(QWidget *parent = nullptr);
    ~FormDisplayWidgets();

private:
    Ui::FormDisplayWidgets *ui;
};

#endif // FORMDISPLAYWIDGETS_H
