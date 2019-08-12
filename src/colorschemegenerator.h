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

#ifndef COLORSCHEMEGENERATOR_H
#define COLORSCHEMEGENERATOR_H

// Qt Libraries
#include <QObject>
#include <QColor>


class ColorSchemeGenerator : public QObject
{
    Q_OBJECT
public:
    explicit ColorSchemeGenerator(QObject *parent = nullptr);

    enum SchemeOrder{Count, HSV, HVS, SVH, SHV, VHS, VSH};

    /** This member function generates a color scheme from an image. The function returns num_colors number of colors.
     */
    static QMap<int, QColor> generate(const QString& filename, const int& num_colors = 10,
                                      const int& num_samples = 1000, const int& color_ordering = HSV);

    static QMap<int, QColor> generateRandom(const int& num_colors,
                                            const int& min_hue = 0, const int& max_hue = 255,
                                            const int& min_sat = 0, const int& max_sat = 255,
                                            const int& min_val = 0, const int& max_val = 255);

signals:

public slots:

};

#endif // COLORSCHEMEGENERATOR_H
