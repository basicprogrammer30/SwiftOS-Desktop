/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.private.kickoff 0.1 as Kickoff
Item {
    id: searchViewContainer

    anchors.fill: parent

    objectName: "SearchView"

    function decrementCurrentIndex() {
        searchView.decrementCurrentIndex();
    }

    function incrementCurrentIndex() {
        searchView.incrementCurrentIndex();
    }

    function activateCurrentIndex() {
        searchView.currentItem.activate();
    }

    function openContextMenu() {
        searchView.currentItem.openContextMenu();
    }

    PlasmaExtras.ScrollArea {
        anchors.fill: parent

        ListView {
            id: searchView

            anchors.fill: parent
            keyNavigationWraps: true
            boundsBehavior: Flickable.StopAtBounds
            delegate: KickoffItem {}
            highlight: KickoffHighlight {}
            highlightMoveDuration : 0
            model: Kickoff.KRunnerModel{}

            Connections {
                target: header

                onQueryChanged: {
                    searchView.model.setQuery(header.query)
                }
            }
        } // searchView
    } // ScrollArea
    Keys.onPressed: {
        if (event.key == Qt.Key_Menu) {
            print( "Context Menu triggered by key...");
            contextMenu.open();
        }
    }
    ContextMenu {
        id: contextMenu
    }
}
