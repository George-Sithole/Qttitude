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

#ifndef DIALOGCOLORSPEC_H
#define DIALOGCOLORSPEC_H

// C/C++ Libraries
#include <vector>

// Qt Libraries
#include <QDialog>
#include <QMap>

// Local Libraries
#include "globals.h"


namespace Ui {
class DialogColorSpec;
}


class QPainter;
class QSlider;

class Workspace;


class DialogColorSpec : public QDialog
{
    Q_OBJECT

public:
    explicit DialogColorSpec(Workspace* workspace, QWidget *parent = nullptr);
    ~DialogColorSpec();

    enum PaletteSorting{SortHue, SortSaturation, SortValue, SortRandom};

    std::vector<int> hue_range();
    std::vector<int> saturation_range();
    std::vector<int> value_range();

    QMap<int, QColor> colorScheme(){return m_palette_filtered;}

    void generateRandom();

    ColorScheme activeScheme();

    QHash<QString, ColorScheme>& colorSpecifications(){return m_color_scheme_hash;}

protected:
    void paintSlider(QSlider* slider, const int& half_range);

    void updateSliderHueStyleSheet();

    void updateSliderSaturationStyleSheet();

    void updateSliderValueStyleSheet();

    bool eventFilter(QObject *obj, QEvent *event);

    void paintColorSwatches(QWidget* widget, QMap<int, QColor>& palette);

    void resizeColorSwatches();

    void resizeEvent(QResizeEvent* event);

    bool loadColorSchemes(const QString& filename);

    bool saveColorSchemes(const QString& filename);

    void setCurrentScheme(const ColorScheme& scheme);

    QMap<int, QColor> filterPalette(const QMap<int, QColor>& palette);

private slots:
    void on_horizontalSliderHue_sliderMoved(int position);

    void on_horizontalSliderSaturation_sliderMoved(int position);

    void on_horizontalSliderValue_sliderMoved(int position);

    void on_rdbtnRandom_clicked();

    void on_rdbtnHue_clicked();

    void on_rdbtnSaturation_clicked();

    void on_rdbtnValue_clicked();

    void on_horizontalSliderHueRange_sliderMoved(int position);

    void on_horizontalSliderSaturationRange_sliderMoved(int position);

    void on_horizontalSliderValueRange_sliderMoved(int position);

    void on_btnAddNewColorScheme_clicked();

    void on_comboBoxSchemeName_currentIndexChanged(const QString& scheme_name);

    void on_btnRemove_clicked();

    void on_spnFilter_valueChanged(const QString &arg1);

    void on_spnNumColors_valueChanged(const QString &arg1);

private:
    Ui::DialogColorSpec *ui;

    QMap<int, QColor> m_palette;
    QMap<int, QColor> m_palette_filtered;

    int m_palette_sort;

    int m_swatch_width;
    int m_swatch_spacing;

    Workspace* m_workspace;

    QHash<QString, ColorScheme> m_color_scheme_hash;
};

#endif // DIALOGCOLORSPEC_H
