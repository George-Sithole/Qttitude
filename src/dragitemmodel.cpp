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
#include <QMimeData>
#include <QModelIndex>
#include <QList>
#include <QDebug>

// Local Libraries
#include "dragitemmodel.h"


using namespace std;


DragItemModel::DragItemModel(QObject* parent) : QStandardItemModel(parent)
{

}

QStringList DragItemModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

QMimeData* DragItemModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    QStringList text_list;
    foreach (const QModelIndex& index, indexes)
    {
        if (index.isValid())
        {
            if (index.column() == 0){ // take care that the indexes in the other columns are not included in the parsing
                QModelIndex index_col_0 = index.sibling(index.row(), 0);
                QString widget_name = index_col_0.data().toString();

                QStringList crumbs = this->breadCrumb(index_col_0);
                if(crumbs.count() == 0){
                    qDebug() << "Error: DragItemModel::mimeData - the bread crumb is empty";
                } else if(crumbs.count() == 1) {
                    text_list << crumbs.first();
                } else {
                    text_list << QString("%1 %2").arg(crumbs.first()).arg(crumbs.last());
                }
            }
        }
    }
    stream << text_list;

    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}

QStringList DragItemModel::breadCrumb(const QModelIndex& index) const
{
    if(index.isValid()) {
        QString widget_name = index.sibling(index.row(), 0).data(Qt::DisplayRole).toString();
        QString widget_class = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();
        QStringList widget_definition(widget_class + "#" + widget_name);

        return this->breadCrumb(index.parent()) + widget_definition;
    } else {
        return QStringList();
    }
}


