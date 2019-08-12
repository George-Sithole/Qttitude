/****************************************************************************
**
** Copyright (C) 2019 George Sithole
** Contact: http://www.geovariant.com/qttitude/
**
** This is free software distributed under the terms of the GNU General Public License, GPL v3.
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Qttitude nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

// C/C++ Libraries
#include <iostream>
#include <vector>
#include <random>
#include <math.h>
#include <functional>

// Qt Libraries
#include <QPainter>
#include <QDebug>
#include <QPaintEvent>
#include <QScrollBar>
#include <QStyleOptionSlider>
#include <QSlider>
#include <QInputDialog>
#include <QMessageBox>
#include <QVector>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

// Local Libraries
#include "dialogcolorspec.h"
#include "ui_dialogcolorspec.h"
#include "workspace.h"


using namespace  std;


DialogColorSpec::DialogColorSpec(Workspace* workspace, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogColorSpec),
    m_workspace(workspace)
{
    ui->setupUi(this);

    // setup swatches
    m_swatch_width = 16;
    m_swatch_spacing = 3;

    // update the saturation slider style sheet
    this->updateSliderHueStyleSheet();
    ui->horizontalSliderHue->update();

    this->updateSliderSaturationStyleSheet();
    ui->horizontalSliderSaturation->update();

    this->updateSliderValueStyleSheet();
    ui->horizontalSliderValue->update();

    // setup palette sort
    ui->rdbtnRandom->setChecked(true);

    // setup ranges
    ui->horizontalSliderHueRange->setMaximum( (ui->horizontalSliderHue->maximum() + 1) * 2 );
    ui->horizontalSliderHueRange->setValue(ui->horizontalSliderHueRange->maximum() / 2.0);

    ui->horizontalSliderSaturationRange->setMaximum( (ui->horizontalSliderSaturation->maximum() + 1) * 2 );
    ui->horizontalSliderSaturationRange->setValue(ui->horizontalSliderSaturationRange->maximum() / 2.0);

    ui->horizontalSliderValueRange->setMaximum( (ui->horizontalSliderValue->maximum() + 1) * 2 );
    ui->horizontalSliderValueRange->setValue(ui->horizontalSliderValueRange->maximum() / 2.0);

    // install event filter for widgetPalette
    // the event filter is installed for widgetPalette to give it control over its drawing.
    // this is necessary because widgetPalette is inside a scroll area.
    ui->widgetPalette->installEventFilter(this);
    ui->widgetPaletteFiltered->installEventFilter(this);

    // install event filter for hsv sliders
    ui->horizontalSliderHue->installEventFilter(this);
    ui->horizontalSliderSaturation->installEventFilter(this);
    ui->horizontalSliderValue->installEventFilter(this);

    // generate a color scheme
    //this->generateRandom();

    // load color schemes
    this->loadColorSchemes(m_workspace->colorSchemeFilename());
}

DialogColorSpec::~DialogColorSpec()
{
    delete ui;
}

void DialogColorSpec::paintSlider(QSlider* slider, const int& range)
{
    QPainter painter(slider);

    // get handle size ...
    // ... ensure that this is the same width (including the border) as in the slider's stylesheet
    int handle_width = 10 + 4;

    float hr = range / 2.0;
    float max_val = slider->maximum();
    float v = slider->value();

    float min_x = (v - hr) < 0 ? 0 : (v - hr);
    float max_x = (v + hr) > max_val ? max_val : v + hr;

    int sw = slider->width() - handle_width;

    int w = sw * (max_x - min_x) / max_val;

    int x = handle_width / 2.0 + sw * min_x / max_val;
    int y = 0;

    // draw the range indicator
//    painter.setPen(Qt::NoPen);
//    painter.setBrush(QBrush("greenyellow"));
//    painter.drawRect(0, 0, slider->width(), slider->height());

    painter.setPen(Qt::NoPen);
    QColor color("#999999");
    painter.setBrush(QBrush(color));
    painter.drawRect(x, y, w, slider->height());

    // draw the stops at the end of the slider
    painter.drawRect(0, 0, handle_width / 2.0, slider->height());
    painter.drawRect(slider->width() - handle_width / 2.0, 0, handle_width / 2.0, slider->height());
}

std::vector<int> DialogColorSpec::hue_range()
{
    int value = ui->horizontalSliderHue->value();
    int range = ui->horizontalSliderHueRange->value();
    int rmin = value - range / 2.0;
    int rmax = value + range / 2.0;
    rmin = rmin < 0 ? 0 : rmin;
    rmax = rmax > ui->horizontalSliderHue->maximum() ? ui->horizontalSliderHue->maximum() : rmax;

    return {rmin, rmax};
}

std::vector<int> DialogColorSpec::saturation_range()
{
    int value = ui->horizontalSliderSaturation->value();
    int range = ui->horizontalSliderSaturationRange->value();
    int rmin = value - range / 2.0;
    int rmax = value + range / 2.0;
    rmin = rmin < 0 ? 0 : rmin;
    rmax = rmax > ui->horizontalSliderSaturation->maximum() ? ui->horizontalSliderSaturation->maximum() : rmax;

    return {rmin, rmax};
}

std::vector<int> DialogColorSpec::value_range()
{
    int value = ui->horizontalSliderValue->value();
    int range = ui->horizontalSliderValueRange->value();
    int rmin = value - range / 2.0;
    int rmax = value + range / 2.0;
    rmin = rmin < 0 ? 0 : rmin;
    rmax = rmax > ui->horizontalSliderValue->maximum() ? ui->horizontalSliderValue->maximum() : rmax;

    return {rmin, rmax};
}

void DialogColorSpec::generateRandom()
{
    int num_colors = ui->spnNumColors->value();
    vector<int> hr = this->hue_range();
    vector<int> sr = this->saturation_range();
    vector<int> vr = this->value_range();

    // create a uniform random number generator
    std::random_device rd; /* Seed */
    std::default_random_engine generator(rd()); /* Random number generator */
    std::uniform_int_distribution<long long unsigned> hue_d(hr[0], hr[1]); /* Distribution on which to apply the generator */
    std::uniform_int_distribution<long long unsigned> saturation_d(sr[0], sr[1]); /* Distribution on which to apply the generator */
    std::uniform_int_distribution<long long unsigned> value_d(vr[0], vr[1]); /* Distribution on which to apply the generator */

    // clear the scheme
    if(!m_palette.isEmpty())
        m_palette.clear();

    // create the scheme ... this creates a multimap of colors
    for(int i = 0; i < num_colors; ++i)
    {
        vector<int> hsv = { int(hue_d(generator)), int(saturation_d(generator)), int(value_d(generator)) };
        m_palette.insert(i, QColor::fromHsv(hsv[0], hsv[1], hsv[2]));
    }

    if(ui->rdbtnRandom->isChecked())            this->on_rdbtnRandom_clicked();
    else if(ui->rdbtnHue->isChecked())          this->on_rdbtnHue_clicked();
    else if(ui->rdbtnSaturation->isChecked())   this->on_rdbtnSaturation_clicked();
    else if(ui->rdbtnValue->isChecked())        this->on_rdbtnValue_clicked();
}

void DialogColorSpec::on_horizontalSliderHue_sliderMoved(int position)
{
    Q_UNUSED(position)

    this->updateSliderSaturationStyleSheet();
    this->updateSliderValueStyleSheet();

    // generate palette
    this->generateRandom();
}

void DialogColorSpec::on_horizontalSliderSaturation_sliderMoved(int position)
{
    Q_UNUSED(position)

    this->updateSliderHueStyleSheet();
    this->updateSliderValueStyleSheet();

    // generate palette
    this->generateRandom();
}

void DialogColorSpec::on_horizontalSliderValue_sliderMoved(int position)
{
    Q_UNUSED(position)

    this->updateSliderHueStyleSheet();
    this->updateSliderSaturationStyleSheet();

    // generate palette
    this->generateRandom();
}

void DialogColorSpec::on_rdbtnRandom_clicked()
{
    m_palette_sort = SortRandom;

    // get the keys
    QList<int> keys = m_palette.keys();
    std::random_shuffle(keys.begin(), keys.end());

    // write palette
    QMultiMap<int, QColor> palette;
    int i = 0;
    for(int index: keys){
        palette.insert(i++, m_palette[index]);
    }

    // clear the scheme
    if(!m_palette.isEmpty()){
        m_palette.clear();
    }

    // copy map
    i = 0;
    for(QColor color: palette){
        m_palette.insert(i++, color);
    }

    // filter the palettee
    this->m_palette_filtered = this->filterPalette(this->m_palette);

    ui->widgetPalette->update();
    ui->widgetPaletteFiltered->update();
}

QMap<int, QColor> DialogColorSpec::filterPalette(const QMap<int, QColor>& palette)
{
    int nfilter = ui->spnFilter->value();
    double d = double(palette.count() - 1) / double(nfilter - 1);
    QVector<double> v(nfilter, 0.0);
    for(int i = 1; i < nfilter; ++i){
        v[i] = v[i - 1] + d;
    }

    QMap<int, QColor> fpalette;
    for(int i = 0; i < nfilter; ++i)
    {
        int idx = int(std::floor(v[i] + 0.5));
        fpalette[i] = palette[idx];
    }

    return fpalette;
}

void DialogColorSpec::on_rdbtnHue_clicked()
{
    m_palette_sort = SortHue;

    // write palette
    QMultiMap<int, QColor> palette;
    for(QColor color: m_palette)
    {
        palette.insert(color.hue(), color);
    }

    // clear the scheme
    if(!m_palette.isEmpty())
        m_palette.clear();

    // copy map
    int i = 0;
    for(QColor color: palette)
    {
        m_palette.insert(i++, color);
    }

    // filter the palettee
    this->m_palette_filtered = this->filterPalette(this->m_palette);

    ui->widgetPalette->update();
    ui->widgetPaletteFiltered->update();
}

void DialogColorSpec::on_rdbtnSaturation_clicked()
{
    m_palette_sort = SortSaturation;

    // write palette
    QMultiMap<int, QColor> palette;
    for(QColor color: m_palette)
    {
        palette.insert(color.saturation(), color);
    }

    // clear the scheme
    if(!m_palette.isEmpty())
        m_palette.clear();

    // copy map
    int i = 0;
    for(QColor color: palette)
    {
        m_palette.insert(i++, color);
    }

    // filter the palettee
    this->m_palette_filtered = this->filterPalette(this->m_palette);

    ui->widgetPalette->update();
    ui->widgetPaletteFiltered->update();
}

void DialogColorSpec::on_rdbtnValue_clicked()
{
    m_palette_sort = SortValue;

    // write palette
    QMultiMap<int, QColor> palette;
    for(QColor color: m_palette)
    {
        palette.insert(color.value(), color);
    }

    // clear the scheme
    if(!m_palette.isEmpty())
        m_palette.clear();

    // copy map
    int i = 0;
    for(QColor color: palette)
    {
        m_palette.insert(i++, color);
    }

    // filter the palettee
    this->m_palette_filtered = this->filterPalette(this->m_palette);

    ui->widgetPalette->update();
    ui->widgetPaletteFiltered->update();
}

void DialogColorSpec::on_horizontalSliderHueRange_sliderMoved(int position)
{
    Q_UNUSED(position)

    ui->horizontalSliderHue->update();

    // generate palette
    this->generateRandom();
}

void DialogColorSpec::on_horizontalSliderSaturationRange_sliderMoved(int position)
{
    Q_UNUSED(position)

    ui->horizontalSliderSaturation->update();

    // generate palette
    this->generateRandom();
}

void DialogColorSpec::on_horizontalSliderValueRange_sliderMoved(int position)
{
    Q_UNUSED(position)

    ui->horizontalSliderValue->update();

    // generate palette
    this->generateRandom();
}

bool DialogColorSpec::eventFilter(QObject *obj, QEvent *event)
{
    QWidget* widget = qobject_cast<QWidget*>(obj);

    if(widget == ui->widgetPalette)
    {
        if (event->type() == QEvent::Paint)
        {
            //QPaintEvent* paintEvent = static_cast<QPaintEvent*>(event);
            this->paintColorSwatches(ui->widgetPalette, this->m_palette);
            return true;
        }
        else
        {
            return QObject::eventFilter(obj, event); // standard event processing
        }
    }
    else if(widget == ui->widgetPaletteFiltered)
    {
        if (event->type() == QEvent::Paint)
        {
            //QPaintEvent* paintEvent = static_cast<QPaintEvent*>(event);
            this->paintColorSwatches(ui->widgetPaletteFiltered, this->m_palette_filtered);
            return true;
        }
        else
        {
            return QObject::eventFilter(obj, event); // standard event processing
        }
    }
    else if(widget == ui->horizontalSliderHue)
    {
        if (event->type() == QEvent::Paint)
            this->paintSlider(ui->horizontalSliderHue, ui->horizontalSliderHueRange->value());

        return QObject::eventFilter(obj, event); // standard event processing - continue to the sliders native paint event
    }
    else if(widget == ui->horizontalSliderSaturation)
    {
        if (event->type() == QEvent::Paint)
            this->paintSlider(ui->horizontalSliderSaturation, ui->horizontalSliderSaturationRange->value());

        return QObject::eventFilter(obj, event); // standard event processing - continue to the sliders native paint event
    }
    else if(widget == ui->horizontalSliderValue)
    {
        if (event->type() == QEvent::Paint)
            this->paintSlider(ui->horizontalSliderValue, ui->horizontalSliderValueRange->value());

        return QObject::eventFilter(obj, event); // standard event processing - continue to the sliders native paint event
    }
    else
    {
        return QObject::eventFilter(obj, event); // standard event processing
    }
}

void DialogColorSpec::resizeColorSwatches()
{
    int offsetx = 2;

    // set the size of the widget
    int num_colors = m_palette.count();
    int w = (m_swatch_width + m_swatch_spacing) * num_colors - m_swatch_spacing + 1;
    w += (2 * offsetx);
    w = w < ui->scrollArea->width() ? ui->scrollArea->width() : w;

    ui->widgetPalette->setFixedWidth(w);
    ui->widgetPaletteFiltered->setFixedWidth(w);
}

void DialogColorSpec::paintColorSwatches(QWidget* widget, QMap<int, QColor>& palette)
{
    int offsetx = 1;
    int offsety = 1;

    int cx = offsetx;
    int cy = offsety;

    // create the painter
    QPainter painter(widget);

    // paint the colors
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::white));
//    painter.drawRect(widget->rect());

    painter.setPen(Qt::black);
    QList<int> keys = palette.keys();
    for(int i = 0; i < keys.count(); ++i)
    {
        painter.setBrush(palette[i]);
        painter.drawRect(QRect(cx, cy, m_swatch_width, m_swatch_width));

        cx += (m_swatch_width + m_swatch_spacing);
        if(cx > widget->width())//ui->widgetPalette->width())
        {
            cx = offsetx; // ui->widgetPalette->x();
            cy += (m_swatch_width + m_swatch_spacing);
        }
    }
}

void DialogColorSpec::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)

    this->resizeColorSwatches();
}

void DialogColorSpec::updateSliderHueStyleSheet()
{
    QString style_sheet = "QSlider"
                          "{\n"
                               "border: 1px solid #999999;\n"
                          "}\n"
                          "QSlider::groove:horizontal\n"
                          "{\n"
                          "    border: 0px solid #999999;\n"
                          "    height: 12px;\n"
                          "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 %0, stop:0.166 %1, stop:0.333 %2, stop:0.5 %3, stop:0.666 %4, stop:0.833 %5, stop:1 %6);\n"
                          "    margin: 2px 0;\n"
                          "}\n"
                          "\n"
                          "QSlider::handle:horizontal\n"
                          "{\n"
                          //"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);\n"
                          "    border: 1px solid #5c5c5c;\n"
                          "    width: 10px;\n"
                          "    margin: -2px 0;\n"
                          "    border-radius: 3px;\n"
                          "}";

    int saturation = ui->horizontalSliderSaturation->value();
    int value = ui->horizontalSliderValue->value();

    QColor color_0 = QColor::fromHsv(0.000 * 359, saturation, value);
    QColor color_1 = QColor::fromHsv(0.166 * 359, saturation, value);
    QColor color_2 = QColor::fromHsv(0.333 * 359, saturation, value);
    QColor color_3 = QColor::fromHsv(0.500 * 359, saturation, value);
    QColor color_4 = QColor::fromHsv(0.666 * 359, saturation, value);
    QColor color_5 = QColor::fromHsv(0.833 * 359, saturation, value);
    QColor color_6 = QColor::fromHsv(1.000 * 359, saturation, value);
    style_sheet = style_sheet.arg(color_0.name()).arg(color_1.name()).arg(color_2.name()).arg(color_3.name()).arg(color_4.name()).arg(color_5.name()).arg(color_6.name());

    ui->horizontalSliderHue->setStyleSheet("");
    ui->horizontalSliderHue->setStyleSheet(style_sheet);
}

void DialogColorSpec::updateSliderSaturationStyleSheet()
{
    QString style_sheet = "QSlider"
                          "{\n"
                               "border: 1px solid #999999;\n"
                          "}\n"
                          "QSlider::groove:horizontal\n"
                          "{\n"
                          "    border: 0px solid #999999;\n"
                          "    height: 12px;\n"
                          "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 %0, stop:1 %1);\n"
                          "    margin: 2px 0;\n"
                          "}\n"
                          "\n"
                          "QSlider::handle:horizontal\n"
                          "{\n"
                          //"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);\n"
                          "    border: 1px solid #5c5c5c;\n"
                          "    width: 10px;\n"
                          "    margin: -2px 0;\n"
                          "    border-radius: 3px;\n"
                          "}";

    int hue = ui->horizontalSliderHue->value();
    int value = ui->horizontalSliderValue->value();
    QColor color_0 = QColor::fromHsv(hue, 0, value);
    QColor color_1 = QColor::fromHsv(hue, 255, value);
    style_sheet = style_sheet.arg(color_0.name()).arg(color_1.name());

    ui->horizontalSliderSaturation->setStyleSheet("");
    ui->horizontalSliderSaturation->setStyleSheet(style_sheet);
}

void DialogColorSpec::updateSliderValueStyleSheet()
{
    QString style_sheet = "QSlider"
                          "{\n"
                               "border: 1px solid #999999;\n"
                          "}\n"
                          "QSlider::groove:horizontal\n"
                          "{\n"
                          "    border: 0px solid #999999;\n"
                          "    height: 12px;\n"
                          "    background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 %0, stop:1 %1);\n"
                          "    margin: 2px 0;\n"
                          "}\n"
                          "\n"
                          "QSlider::handle:horizontal\n"
                          "{\n"
                          //"    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);\n"
                          "    border: 1px solid #5c5c5c;\n"
                          "    width: 10px;\n"
                          "    margin: -2px 0;\n"
                          "    border-radius: 3px;\n"
                          "}";

    int hue = ui->horizontalSliderHue->value();
    int saturation = ui->horizontalSliderSaturation->value();
    QColor color_0 = QColor::fromHsv(hue, saturation, 0);
    QColor color_1 = QColor::fromHsv(hue, saturation, 255);
    style_sheet = style_sheet.arg(color_0.name()).arg(color_1.name());

    ui->horizontalSliderValue->setStyleSheet("");
    ui->horizontalSliderValue->setStyleSheet(style_sheet);
}

ColorScheme DialogColorSpec::activeScheme()
{
    ColorScheme spec;
    spec.hue = ui->horizontalSliderHue->value();
    spec.saturation = ui->horizontalSliderSaturation->value();
    spec.value = ui->horizontalSliderValue->value();

    spec.hue_range = ui->horizontalSliderHueRange->value();
    spec.saturation_range = ui->horizontalSliderSaturationRange->value();
    spec.value_range = ui->horizontalSliderValueRange->value();

    QMap<int, QColor>::const_iterator iter = m_palette.constBegin();
    int i = 0;
    while (iter != m_palette.constEnd())
    {
        spec.scheme.insert(i++, iter.value().name());
        ++iter;
    }

    return spec;
}

bool DialogColorSpec::loadColorSchemes(const QString& filename)
{
    // check if the file exists
    if(!QFile::exists(filename))
    {
        cout << "Couldn't load the color scheme" << endl;
        return false;
    }

    // clear the current color schemes
    this->m_color_scheme_hash.clear();

    // read the specification file
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // read project json file
        QByteArray json = file.readAll();
        QJsonDocument json_doc = QJsonDocument::fromJson(json);
        QJsonObject root_obj = json_doc.object();

        QJsonArray schemes_arr = root_obj.value("schemes").toArray();
        foreach(QJsonValue scheme_value, schemes_arr)
        {
            QJsonObject scheme_obj = scheme_value.toObject();

            ColorScheme color_scheme;
            color_scheme.name = scheme_obj["name"].toString().trimmed();

            color_scheme.hue = scheme_obj["hue"].toInt();
            color_scheme.saturation = scheme_obj["saturation"].toInt();
            color_scheme.value = scheme_obj["value"].toInt();

            color_scheme.hue_range = scheme_obj["hue_range"].toInt();
            color_scheme.saturation_range = scheme_obj["saturation_range"].toInt();
            color_scheme.value_range = scheme_obj["value_range"].toInt();

            color_scheme.num_colors = scheme_obj["num_colors"].toInt();
            color_scheme.num_filters = scheme_obj["num_filters"].toInt();
            color_scheme.sort_mode = scheme_obj["sort_mode"].toInt();

            this->m_color_scheme_hash.insert(color_scheme.name, color_scheme);
        }
    }
    file.close();

    // add the schemes to the combo box
    ui->comboBoxSchemeName->addItems(m_color_scheme_hash.keys());

    return true;
}

bool DialogColorSpec::saveColorSchemes(const QString& filename)
{
    // read the specification file
    ColorScheme scheme;
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QJsonObject root_obj;
        QJsonArray schemes_arr;

        QHash<QString, ColorScheme>::const_iterator iter = m_color_scheme_hash.constBegin();
        while (iter != m_color_scheme_hash.constEnd())
        {
            scheme = iter.value();

            QJsonObject scheme_obj;
            scheme_obj["name"] = scheme.name;

            scheme_obj["hue"] = scheme.hue;
            scheme_obj["saturation"] = scheme.saturation;
            scheme_obj["value"] = scheme.value;

            scheme_obj["hue_range"] = scheme.hue_range;
            scheme_obj["saturation_range"] = scheme.saturation_range;
            scheme_obj["value_range"] = scheme.value_range;

            scheme_obj["num_colors"] = scheme.num_colors;
            scheme_obj["num_filters"] = scheme.num_filters;
            scheme_obj["sort_mode"] = scheme.sort_mode;

            schemes_arr.append(scheme_obj);
//            file.write(scheme.toString().toLatin1());
//            file.write("\n");
            ++iter;
        }

        root_obj["schemes"] = schemes_arr;

        QJsonDocument json_doc(root_obj);
        QByteArray json = json_doc.toJson(QJsonDocument::Indented);

        file.write(json);
    }
    file.close();

    return true;
}

void DialogColorSpec::on_btnAddNewColorScheme_clicked()
{
    bool ok;
    QString scheme_name = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                              tr("Scheme name:"), QLineEdit::Normal,
                                              "Scheme name", &ok);
    if (ok && !scheme_name.isEmpty())
    {
        // check for duplicate
        if(ui->comboBoxSchemeName->findText(scheme_name) > -1) // duplicate found
        {
            QMessageBox msgBox;
            msgBox.setText("This scheme name already exists. Do you want to replace it?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::No);
            int result = msgBox.exec();
            if(result == QMessageBox::No)
            {
                cout << "Do not replace" << endl;
                return;
            }
        }

        // get the active color scheme
        ColorScheme scheme = this->activeScheme();
        scheme.name = scheme_name;
        scheme.num_colors = ui->spnNumColors->value();
        scheme.num_filters = ui->spnFilter->value();
        scheme.sort_mode = (ui->rdbtnRandom->isChecked() << 3) |
                           (ui->rdbtnHue->isChecked() << 2) |
                           (ui->rdbtnSaturation->isChecked() << 1) |
                           (ui->rdbtnValue->isChecked());

        m_color_scheme_hash.insert(scheme_name, scheme);

        ui->comboBoxSchemeName->addItem(scheme_name);

        // save color schemes
        this->saveColorSchemes(m_workspace->colorSchemeFilename());
    }
}

void DialogColorSpec::on_comboBoxSchemeName_currentIndexChanged(const QString& scheme_name)
{
    // get the color scheme
    ColorScheme scheme = m_color_scheme_hash.value(scheme_name, ColorScheme());

    // set current scheme
    this->setCurrentScheme(scheme);
}

void DialogColorSpec::setCurrentScheme(const ColorScheme& scheme)
{
    // set the widgets
    ui->horizontalSliderHue->setValue(scheme.hue);
    ui->horizontalSliderHueRange->setValue(scheme.hue_range);

    ui->horizontalSliderSaturation->setValue(scheme.saturation);
    ui->horizontalSliderSaturationRange->setValue(scheme.saturation_range);

    ui->horizontalSliderValue->setValue(scheme.value);
    ui->horizontalSliderValueRange->setValue(scheme.value_range);

    ui->spnNumColors->setValue(scheme.num_colors);

    switch (scheme.sort_mode) {
    case ColorScheme::SortRandom: ui->rdbtnRandom->setChecked(true); break;
    case ColorScheme::SortByHue: ui->rdbtnHue->setChecked(true); break;
    case ColorScheme::SortBySaturation: ui->rdbtnSaturation->setChecked(true); break;
    case ColorScheme::SortByValue: ui->rdbtnValue->setChecked(true); break;
    }
    ui->spnFilter->setValue(scheme.num_filters);

    // generate palette
    this->generateRandom();

    // filter the palettee
    this->m_palette_filtered = this->filterPalette(this->m_palette);

    ui->widgetPalette->update();
    ui->widgetPaletteFiltered->update();

    this->update();
}

void DialogColorSpec::on_btnRemove_clicked()
{
    QString scheme_name = ui->comboBoxSchemeName->currentText();

    // remove from the hash table
    m_color_scheme_hash.remove(scheme_name);

    // remove from the combo box
    ui->comboBoxSchemeName->removeItem(ui->comboBoxSchemeName->currentIndex());

    // update the current color swatch
    scheme_name = ui->comboBoxSchemeName->currentText();
    ColorScheme scheme = m_color_scheme_hash.value(scheme_name, ColorScheme());

    // set current scheme
    this->setCurrentScheme(scheme);

    // save color schemes
    this->saveColorSchemes(m_workspace->colorSchemeFilename());
}


void DialogColorSpec::on_spnFilter_valueChanged(const QString &arg1)
{
    Q_UNUSED(arg1)

    // filter the palettee
    this->m_palette_filtered = this->filterPalette(this->m_palette);

    ui->widgetPaletteFiltered->update();
}

void DialogColorSpec::on_spnNumColors_valueChanged(const QString &arg1)
{
    Q_UNUSED(arg1)

    // update the size of the widgetPalette and widgetPaletteFiltered
    this->resizeColorSwatches();

    // generate palette
    this->generateRandom();
}
