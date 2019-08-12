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
#include <QImage>
#include <QMap>
#include <QDebug>
#include <QColor>

// Local Libraries
#include "colorschemegenerator.h"
#include "third_party/dkm/include/dkm.hpp"


using namespace std;


int hsv(QColor& c){return ((c.hue() & 0xffu) << 16) | ((c.saturation() & 0xffu) << 8) | (c.value() & 0xffu);}
int hvs(QColor& c){return ((c.hue() & 0xffu) << 16) | ((c.value() & 0xffu) << 8) | (c.saturation() & 0xffu);}
int shv(QColor& c){return ((c.saturation() & 0xffu) << 16) | ((c.hue() & 0xffu) << 8) | (c.value() & 0xffu);}
int svh(QColor& c){return ((c.saturation() & 0xffu) << 16) | ((c.value() & 0xffu) << 8) | (c.hue() & 0xffu);}
int vhs(QColor& c){return ((c.value() & 0xffu) << 16) | ((c.hue() & 0xffu) << 8) | (c.saturation() & 0xffu);}
int vsh(QColor& c){return ((c.value() & 0xffu) << 16) | ((c.saturation() & 0xffu) << 8) | (c.hue() & 0xffu);}


ColorSchemeGenerator::ColorSchemeGenerator(QObject *parent) : QObject(parent)
{

}

QMap<int, QColor> ColorSchemeGenerator::generate(const QString& filename, const int& num_colors,
                                                 const int& num_samples, const int& color_ordering)
{
    // open the image
    QImage image(filename);

    float scale = sqrt( image.width() * image.height() / float(num_samples) );
    image = image.scaled(image.size() / scale, Qt::KeepAspectRatio);

    // sample
    std::vector<std::array<float, 3>> data;
    // std::vector<std::array<float, 3>> data;//{{1.f, 1.f, 1.f}, {2.f, 2.f, 2.f}, {1200.f, 1200.f, 1200.f}, {2.f, 2.f, 2.f}};
    // data.push_back({1.f, 1.f, 1.f});

    bool do_random_sampling = (image.width() * image.height()) < num_samples ? false : true;
    do_random_sampling = false;
    if(do_random_sampling)
    {
        // ... create a uniform random number generator
        std::random_device rd; /* Seed */
        std::default_random_engine generator(rd()); /* Random number generator */
        std::uniform_int_distribution<long long unsigned> xdistribution(0, image.width() - 1); /* Distribution on which to apply the generator */
        std::uniform_int_distribution<long long unsigned> ydistribution(0, image.height() - 1); /* Distribution on which to apply the generator */

        for(int i = 0; i < num_samples; ++i)
        {
            int x = xdistribution(generator);
            int y = ydistribution(generator);
            QRgb rgb = image.pixel(x, y);
            data.push_back({float(qRed(rgb)), float(qGreen(rgb)), float(qBlue(rgb))});
        }
    }
    else
    {
        for(int x = 0; x < image.width(); ++x)
        {
            for(int y = 0; y < image.height(); ++y)
            {
                QRgb rgb = image.pixel(x, y);
                data.push_back({float(qRed(rgb)), float(qGreen(rgb)), float(qBlue(rgb))});
            }
        }
    }
    //cout << "Image size: " << image.width() << " / " << image.height() << endl;
    //cout << "Data count: " << data.size() << endl;

    // perform a k-means clustering using lloyd's method
    auto means_clusters = dkm::kmeans_lloyd(data, num_colors);
    auto means = std::get<0>(means_clusters);
    auto clusters = std::get<1>(means_clusters);

    //cout << typeid(means_clusters).name() << endl;
    //cout << typeid(means).name() << endl;
    //cout << typeid(clusters).name() << endl;


    int c_id = 0;
    QMap<int, QColor> scheme;
    if(color_ordering == ColorSchemeGenerator::Count)
    {
        for_each(means.begin(), means.end(), [&](auto m)
        {
            // cout << typeid(m).name() << endl;
            int count = std::count(clusters.cbegin(), clusters.cend(), c_id);
            QColor color(m[0], m[1], m[2]);
            scheme[count] = color;
            ++c_id;
        });
    }
    else
    {
        std::function<int(QColor&)> order_func;
        switch(color_ordering)
        {
        case ColorSchemeGenerator::HSV: order_func = hsv; break;
        case ColorSchemeGenerator::HVS: order_func = hvs; break;
        case ColorSchemeGenerator::SVH: order_func = svh; break;
        case ColorSchemeGenerator::SHV: order_func = shv; break;
        case ColorSchemeGenerator::VHS: order_func = vhs; break;
        case ColorSchemeGenerator::VSH: order_func = vsh; break;
        }

        for_each(means.begin(), means.end(), [&](auto m)
        {
            // cout << typeid(m).name() << endl;
            QColor color(m[0], m[1], m[2]);
            int order = order_func(color);
            scheme[order] = color;
            ++c_id;
        });
    }

    return scheme;
}

QMap<int, QColor> ColorSchemeGenerator::generateRandom(const int& num_colors,
                                                       const int& min_hue, const int& max_hue,
                                                       const int& min_sat, const int& max_sat,
                                                       const int& min_val, const int& max_val)
{
    // create a uniform random number generator
    std::random_device rd; /* Seed */
    std::default_random_engine generator(rd()); /* Random number generator */
    std::uniform_int_distribution<long long unsigned> hue_d(min_hue, max_hue); /* Distribution on which to apply the generator */
    std::uniform_int_distribution<long long unsigned> saturation_d(min_sat, max_sat); /* Distribution on which to apply the generator */
    std::uniform_int_distribution<long long unsigned> value_d(min_val, max_val); /* Distribution on which to apply the generator */

    QMap<int, QColor> scheme;
    for(int i = 0; i < num_colors; ++i)
    {
        int h = hue_d(generator);
        int s = saturation_d(generator);
        int v = value_d(generator);
        scheme[i] = QColor::fromHsv(h, s, v);
    }

    return scheme;
}
