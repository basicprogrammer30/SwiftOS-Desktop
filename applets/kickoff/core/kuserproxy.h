/***************************************************************************
 *   Copyright 2014 Sebastian K�gler <sebas@kde.org>                       *
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

#ifndef KUSERPROXY_H
#define KUSERPROXY_H

#include <QObject>

#include <KDirWatch>
#include <KUser>

class KUserProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString fullName READ fullName NOTIFY nameChanged)
    Q_PROPERTY(QString loginName READ loginName NOTIFY nameChanged)
    Q_PROPERTY(QString faceIconPath READ faceIconPath NOTIFY faceIconPathChanged)
    Q_PROPERTY(QString os READ os CONSTANT)
    Q_PROPERTY(QString host READ host CONSTANT)

public:
    KUserProxy(QObject *parent = 0);
    ~KUserProxy();

    QString fullName() const;
    QString loginName() const;
    QString faceIconPath() const;
    QString os();
    QString host() const;

Q_SIGNALS:
    void nameChanged();
    void faceIconPathChanged();

private:
    void update(const QString &path);
    KDirWatch m_dirWatch;
    KUser m_user;
    QString m_os;
    bool m_temporaryEmptyFaceIconPath;
};

#endif //KUSERPROXY_H

