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

#ifndef WORKSPACE_H
#define WORKSPACE_H

// Qt Libraries
#include <QObject>
#include <QJsonObject>


/**
 * @brief The Workspace class
 *
 * WARNING - implement mutex for class Workspace to manage multiple access
 * INFO - access m_path through the static function path()
 */

class Workspace : public QObject
{
    Q_OBJECT
public:
    explicit Workspace(QObject *parent = nullptr);

    void addPath(const QString& key, const QString& path);

    static QString path(const QString& key);

    QString colorSchemeFilename(){return m_color_scheme_filename;}

    QString appWindowTitle(const QString& filename = "", const bool& issaved = true);

    // ------------------------------------
    // Information functions
    static QJsonObject meta();

    static QString version() { return meta()["version"].toString(); }

    static QString copyright() { return meta()["copyright"].toString(); }

    static QString website() { return meta()["website"].toString(); }

    static QString donationsite() { return meta()["donationsite"].toString(); }

    static QString license() { return meta()["license"].toString(); }

    static QString licenseShort() { return meta()["licenseshort"].toString(); }

    static QString description() { return meta()["description"].toString(); }

    static QString appName() { return meta()["appname"].toString(); }

    static QString liability() { return meta()["liability"].toString(); }

    static QString aboutHTML();

    static QString licenseHTML();

signals:

public slots:

protected:


private:
    QString m_color_scheme_filename;

    /**
     * This member variable contains the hash_table of global paths
     *
     * The variable m_path is declared static to allow global access to file paths.
     * m_path is initialized in file workspace.cpp.
     */
    static QHash<QString, QString> m_path;
};

#endif // WORKSPACE_H