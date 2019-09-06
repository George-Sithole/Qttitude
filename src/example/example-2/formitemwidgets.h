#ifndef FORMITEMWIDGETS_H
#define FORMITEMWIDGETS_H

#include <QWidget>

namespace Ui {
class FormItemWidgets;
}

class FormItemWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit FormItemWidgets(QWidget *parent = nullptr);
    ~FormItemWidgets();

private:
    Ui::FormItemWidgets *ui;
};

#endif // FORMITEMWIDGETS_H
