/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef IMAGEPICKER_H
#define IMAGEPICKER_H

#include <QObject>
#include <QUrl>
#include <QDialog>
#include <QFileDialog>

class ImagePicker : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)

    public:
        ImagePicker(QObject *parent = 0);
        ~ImagePicker();

        QUrl url() const;

        Q_INVOKABLE void open();

    Q_SIGNALS:
        void urlChanged() const;

    private Q_SLOTS:
        void dialogAccepted();

    private:
        QFileDialog *m_dialog;
        QUrl m_url;
};

#endif
