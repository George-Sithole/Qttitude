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

// Qt Libraries
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QByteArray>
#include <QDebug>

// Local Libraries
#include "workspace.h"


using namespace std;


Workspace::Workspace(QObject *parent) : QObject(parent)
{
    // load color specifications
    m_color_scheme_filename = "color_schemes.col";
}

QJsonObject Workspace::meta()
{
    QFile file(":/info/meta.json");
    QByteArray json;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        json = file.readAll();
    } else {
        std::cout << "Error opening file :/info/meta.json" << std::endl;
    }
    file.close();

    QJsonParseError* error = new QJsonParseError();
    QJsonDocument json_doc = QJsonDocument::fromJson(json, error);
    return json_doc.object();
}

QString Workspace::aboutHTML()
{
    QString html = "<html>"
                   "<head/>"
                   "<body>"
                   "<p><b>" + Workspace::appName() + " " + Workspace::version() + "</b></p>"
                   "<p>" + Workspace::copyright() + "</p>"
                   "<p>" + Workspace::licenseShort() + "</p>"
                   "<p>For more information about this application visit our <a href=https://" + Workspace::website().trimmed() + ">website</a></p>"
                   "<p>Want to help? Show some love by <a href=http://" + Workspace::donationsite().trimmed() + ">making a donation</a></p>"
                   "</body>"
                   "</html>";
    return html;
}

QString Workspace::licenseHTML()
{
    QString license = "<html>"
                      "<head/>"
                      "<body>"
//                      "<p>" + this->description() + "</p>"
                      "<p>" + Workspace::copyright() + "</p>"
                      "<p>" + Workspace::licenseShort() + "</p>"
                      "<p>" + Workspace::liability() + "</p>"
                      "<p>For more information about this application visit our <a href=https://" + Workspace::website().trimmed() + ">website</a></P>"
                      "</body>"
                      "</html>";
    return license;
}

QString Workspace::appWindowTitle(const QString& filename, const bool& issaved)
{
    if (issaved)
    {
        if (filename.trimmed().isEmpty()) {
            return this->appName();
        }
        else {
            return QFileInfo(filename).baseName() + " - " + this->appName();
        }
    }
    else {
        return "* " + QFileInfo(filename).baseName() + " - " + this->appName();
    }
}
