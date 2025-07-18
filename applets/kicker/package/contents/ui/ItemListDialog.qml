/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
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

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.private.kicker 0.1 as Kicker

Kicker.SubMenu {
    id: itemDialog

    property alias focusParent: itemListView.focusParent
    property alias model: funnelModel.sourceModel

    visible: false
    hideOnWindowDeactivate: plasmoid.hideOnWindowDeactivate

    location: PlasmaCore.Types.Floating

    onWindowDeactivated: {
        plasmoid.expanded = false;
    }

    mainItem: ItemListView {
        id: itemListView

        height: model != undefined ? Math.min(((Math.floor((itemDialog.availableScreenRectForItem(itemListView).height
            - itemDialog.margins.top - itemDialog.margins.bottom) / itemHeight) - 1)
            * itemHeight), model.count * itemHeight) : 0

        iconsEnabled: true

        dialog: itemDialog

        model: funnelModel

        Kicker.FunnelModel {
            id: funnelModel

            Component.onCompleted: {
                kicker.reset.connect(funnelModel.reset);
            }
        }
    }

    function delayedDestroy() {
        var timer = Qt.createQmlObject('import QtQuick 2.0; Timer { onTriggered: itemDialog.destroy() }', itemDialog);
        timer.interval = 0;
        timer.start();
    }
}
