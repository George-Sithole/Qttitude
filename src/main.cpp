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

#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QThread>
#include <QTimer>
#include <QDate>
#include "workspace.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString message = "<br>"
                      "<p style=\"color:#444444; font-size: 40px;\"><b>" + Workspace::appName() + "</b></p>"
                      "<p style=\"color:#666666;\">Version: " + Workspace::version() + "</p>"
                      "<p style=\"color:#666666;\">" + Workspace::website() + "</p>"
                      "<br><br><br><br>"
                      "<br><br><br><br>"
                      "<p style=\"color:#666666;\"><b>Support " + Workspace::appName() + "</b></p>"
                      "<p style=\"color:#666666;\">" + Workspace::donationsite() + "<p>"
                      "<br><br>"
                      "<p style=\"color:#666666;\">" + Workspace::copyright() + "</p>"
                      "<p style=\"color:#666666;\">Date: " + QDate::currentDate().toString() + "</p>";

    QPixmap pixmap(":/splash/logo_turquoise.png");
    QSplashScreen splash(pixmap.copy(0, 0, 600, pixmap.height()), Qt::WindowStaysOnTopHint );
    splash.showMessage(message);
    splash.show();

    // close the splash screen after 5 seconds
//    QTimer::singleShot(5000, &splash, &QWidget::close);

    MainWindow w;
    w.resize(800, 800);
    w.show();

    return a.exec();
}


