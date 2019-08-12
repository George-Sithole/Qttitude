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

#ifndef GLOBALS_H
#define GLOBALS_H

// Qt Libraries
#include <QList>
#include <QPair>
#include <QList>
#include <QColor>
#include <QMap>


struct ImageFormatInfo
{
    QString filename;
    float x = 0;
    float y = 0;
    float width = 0;
    float height = 0;
};

typedef QList<ImageFormatInfo> ImageFormatInfoList;


// -------------------------------------------------
//                 Reference
// -------------------------------------------------
typedef QPair<QString, QString> ReferenceItem;
typedef QList<ReferenceItem> ReferenceItems;




// -------------------------------------------------
//            Color Scheme
// -------------------------------------------------
struct ColorScheme
{
    enum SortMode {SortRandom = 0x1000,
                   SortByHue = 0x0100,
                   SortBySaturation = 0x0010,
                   SortByValue = 0x0001};

    QString name;

    int hue;
    int saturation;
    int value;

    int hue_range;
    int saturation_range;
    int value_range;

    int num_colors;
    int num_filters;
    int sort_mode;

    QMap<int, QColor> scheme;

    QString toString();
};


#endif // GLOBALS_H
