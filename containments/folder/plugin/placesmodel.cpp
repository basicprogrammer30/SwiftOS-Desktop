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

#include "placesmodel.h"

#include <KFilePlacesModel>

PlacesModel::PlacesModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    m_sourceModel = new KFilePlacesModel(this);

    setSourceModel(m_sourceModel);

    QHash<int, QByteArray> roleNames = m_sourceModel->roleNames();
    roleNames[Qt::DisplayRole] = "display";
    roleNames[Qt::DecorationRole] = "decoration";
    setRoleNames(roleNames);

    setDynamicSortFilter(true);
}

PlacesModel::~PlacesModel()
{
}

QString PlacesModel::urlForIndex(int idx) const
{
    return m_sourceModel->url(mapToSource(index(idx, 0))).toString();
}


int PlacesModel::indexForUrl(const QString& url) const
{
    QUrl _url(url);
    QModelIndex idx;

    for (int i = 0; i < rowCount(); i++) {
        if (_url == m_sourceModel->url(mapToSource(index(i, 0)))) {
            idx = index(i, 0);
            break;
        }
    }

    if (idx.isValid()) {
        return idx.row();
    }

    return -1;
}

bool PlacesModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = m_sourceModel->index(sourceRow, 0, sourceParent);

    return !m_sourceModel->isHidden(index);
}
