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

#ifndef PROJECT_H
#define PROJECT_H

// Qt Libraries
#include <QObject>
#include <QString>


class Project : public QObject
{
public:
    Project(QObject *parent = nullptr);

    /** This member function returns the project filename.
     */
    QString projectFilename(){return this->m_proj_filename;}

    /** This member function sets the project filename.
     */
    void setProjectFilename(const QString filename){this->m_proj_filename = filename;}

    /** This member function returns the stylesheet filename.
     */
    QString qssFilename(){return this->m_qss_filename;}

    /** This member function sets the stylesheet filename.
     */
    void setQssFilename(const QString filename){this->m_qss_filename = filename;}

    /** This member function returns the project working directory.
     */
    QString workingDir();

    /** This member function returns the save state of the project.
     */
    bool isSaved(){return this->m_issaved;}

    /** This member function sets the project filename.
     */
    void setIsSaved(const bool state){this->m_issaved = state;}

    /** This member function resets the project.
     */
    void reset();

private:
    /** This member variable is the project filename.
     */
    QString m_proj_filename;

    /** This member variable is the stylesheet filename.
     */
    QString m_qss_filename;

    /** This member variable contains the save state of the variable.
     */
    bool m_issaved;
};

#endif // PROJECT_H
