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

import QtQuick 2.2
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.kcoreaddons 1.0 as KCoreAddons

//FIXME
import org.kde.plasma.private.folder 0.1 as Folder

Item {
    id: main

    property int index: model.index
    property string name: model.blank ? "" : model.display
    property QtObject popupDialog: null
    property Item label: label
    property Item labelArea: textBackground
    property Item actionsOverlay: actions
    property Item hoverArea: toolTip
    property Item popupButton: null
    property bool hovered: (GridView.view.hoveredItem == main)
    property bool isDir: model.isDir
    property bool selected: model.blank ? false : model.selected
    property bool blank: model.blank
    property variant snapshot: grabber.image
    property Item snapshotSource: null

    // FIXME TODO: Replace with Qt 5.4's item-to-image API.
    Folder.ItemGrabber {
        id: grabber

        item: main.snapshotSource
    }

    onBlankChanged: {
        if (snapshotSource) {
            snapshotSource.visible = false;
        }
    }

    onSelectedChanged: {
        if (snapshotSource) {
            snapshotSource.visible = false;
            snapshotSource.destroy();
        }

        if (selected && !blank) {
            snapshotTimer.start();
        } else {
            snapshotTimer.stop();
        }
    }

    onSnapshotChanged: {
        if (!grabber.null) {
            dir.addItemDragImage(positioner.map(index), x, y, main.width, main.height, snapshot);
        }

        if (snapshotSource) {
            snapshotSource.visible = false;
            snapshotSource.destroy();
        }
    }

    onHoveredChanged: {
        if (hovered && !main.GridView.view.isRootView && model.isDir) {
            openPopupTimer.start();
        } else if (!hovered)
            openPopupTimer.stop();

            if (popupDialog != null) {
                popupDialog.destroy();
                popupDialog = null;
            }
    }

    onIsDirChanged: {
        if (isDir && main.GridView.view.isRootView && main.popupButton == null) {
            main.popupButton = popupButtonComponent.createObject(actions);
        } else if (main.popupButton) {
            main.popupButton.destroy();
            main.popupButton = null;
        }
    }

    function openPopup() {
        if (root.itemViewDialogComponent.status == Component.Ready) {
            main.popupDialog = root.itemViewDialogComponent.createObject(main);
            main.popupDialog.parentDelegate = main;
            main.popupDialog.visualParent = icon;
            main.popupDialog.url = model.url;
            main.popupDialog.visible = true;
        }
    }

    Timer {
        id: openPopupTimer

        interval: units.longDuration * 3

        onTriggered: {
            main.openPopup();
        }
    }

    Timer {
        id: snapshotTimer
        repeat: false
        interval: 0

        onTriggered: {
            dir.addItemDragImage(positioner.map(index), x, y, main.width, main.height, snapshot);
            main.snapshotSource = Qt.createQmlObject("import QtQuick 2.2; ShaderEffectSource { anchors.fill: main; live: false; sourceItem: main; }", main);
        }
    }

    PlasmaCore.ToolTipArea {
        id: toolTip

        x: frame.x + Math.min(icon.x, label.x)
        y: frame.y + icon.y

        width: Math.max(icon.width, label.width)
        height: (label.y + label.paintedHeight)

        mainItem: toolTipDelegate
        active: (popupDialog == null && !model.blank)
        interactive: false
        location: plasmoid.location

        onContainsMouseChanged:  {
            if (containsMouse && !model.blank) {
                toolTip.icon = model.decoration;
                toolTip.mainText = model.display;
                toolTip.subText = model.type + "\n" + KCoreAddons.Format.formatByteSize(model.size)
                main.GridView.view.hoveredItem = main;
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: frame

        x: units.smallSpacing * 1
        y: units.smallSpacing * 1

        width: parent.width - (2 * units.smallSpacing)
        height: (icon.height + (2 * units.smallSpacing) + (label.lineCount
            * theme.mSize(theme.defaultFont).height) + (2 * units.largeSpacing))

        visible: !model.blank
        enabled: visible

        imagePath: "widgets/viewitem"

        QIconItem {
            id: icon

            anchors {
                top: parent.top
                topMargin: units.largeSpacing
                horizontalCenter: parent.horizontalCenter
            }

            width: main.GridView.view.iconSize
            height: main.GridView.view.iconSize

            icon: model.decoration
        }

        Rectangle {
            id: textBackground

            anchors {
                left: label.left
                leftMargin: -units.smallSpacing
                top: label.top
                topMargin: -units.smallSpacing
                right: label.right
                rightMargin: -units.smallSpacing
                bottom: label.bottom
                bottomMargin: -units.smallSpacing
            }

            color: (root.isContainment && main.GridView.view.isRootView) ? theme.textColor : "transparent"
            radius: units.smallSpacing
            opacity: 0.4
        }

        PlasmaComponents.Label {
            id: label

            anchors {
                top: icon.bottom
                topMargin: 2 * units.smallSpacing
                horizontalCenter: parent.horizontalCenter
            }

            width: Math.min(paintedWidth, parent.width - units.smallSpacing * 8) // FIXME TODO: Pick a frame prefix margin to cache and use instead.
            height: undefined // Unset PlasmaComponents.Label's default.

            textFormat: Text.PlainText

            maximumLineCount: plasmoid.configuration.textLines
            wrapMode: Text.Wrap
            elide: Text.ElideRight

            horizontalAlignment: Text.AlignHCenter

            color: (root.isContainment && main.GridView.view.isRootView) ? theme.backgroundColor : theme.textColor

            text: model.blank ? "" : model.display
        }

        Column {
            id: actions

            x: units.smallSpacing * 3 // FIXME TODO: Pick a frame prefix margin to cache and use instead.
            y: units.smallSpacing * 3 // FIXME TODO: Pick a frame prefix margin to cache and use instead.

            width: implicitWidth
            height: implicitHeight

            ItemActionButton {
                width: systemSettings.singleClick ? units.iconSizes.small : 0
                height: width

                visible: systemSettings.singleClick
                opacity: (systemSettings.singleClick && main.hovered) ? 1.0 : 0.0

                element: model.selected ? "remove" : "add"

                onClicked: dir.toggleSelected(positioner.map(model.index))
            }
        }

        Component {
            id: popupButtonComponent

            ItemActionButton {
                opacity: (plasmoid.configuration.popups && main.hovered && main.popupDialog == null) ? 1.0 : 0.0

                element: "open"

                onClicked: main.openPopup()
            }
        }

        states: [
            State {
                name: "selected"
                when: model.selected

                PropertyChanges {
                    target: frame
                    prefix: "selected"
                }
            },
            State {
                name: "hover"
                when: hovered && !model.selected

                PropertyChanges {
                    target: frame
                    prefix: "hover"
                }
            },
            State {
                name: "selected+hover"
                when: hovered && model.selected

                PropertyChanges {
                    target: frame
                    prefix: "selected+hover"
                }
            }
        ]
    }
}
