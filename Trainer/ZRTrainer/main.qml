import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Universal 2.12

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 480
    height: 160
    maximumWidth: mainWindow.width
    minimumWidth: mainWindow.width
    maximumHeight: mainWindow.height
    minimumHeight: mainWindow.height
    title: qsTr("Zombie's Retreat - Beta 0.14.4 Trainer by Jim00000")

    Universal.theme: Universal.Dark
    Universal.accent: Universal.Violet

    Column {
        topPadding: 10
        leftPadding: 10
        spacing: 8

        Button {
            id: findProcess
            text: qsTr("Find process")

            property bool is_process_ready: false

            onClicked: {
                is_process_ready = binder.findProcess()
                if(is_process_ready) {
                    findProcess.destroy()
                    ammoHerb99.checkable = true
                    heldItem90.checkable = true
                    allItems90.checkable = true
                    binder.showProcessIsReady()
                }
            }

        }

        Switch {
            id: ammoHerb99
            checkable: false
            text: qsTr("Ammo and herb x99 (press Q to take effect)")
            onToggled: {
                if(checked)
                    binder.enableAmmoHerbCheat()
                else
                    binder.disableAmmoHerbCheat()
            }
        }

        Switch {
            id: heldItem90
            checkable: false
            text: qsTr("Held Item x90")
            onToggled: {
                if(checked) {
                    if(allItems90.checked) {
                        binder.disableAllItem90Cheat()
                        allItems90.checked = false
                    }
                    binder.enableHeldItem90Cheat()
                } else
                    binder.disableHeldItem90Cheat()
            }
        }

        Switch {
            id: allItems90
            checkable: false
            text: qsTr("All Items x90")
            onToggled: {
                if(checked) {
                    if(heldItem90.checked) {
                        binder.disableHeldItem90Cheat()
                        heldItem90.checked = false
                    }
                    binder.enableAllItem90Cheat()
                } else
                    binder.disableAllItem90Cheat()
            }
        }

    }
}
