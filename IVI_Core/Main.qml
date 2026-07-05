import QtQuick
import QtQuick.Controls
import QtPositioning
import QtLocation

Window {
    id: rootWindow
    width: 1024
    height: 600
    visible: true
    // visibility: Window.FullScreen
    title: "MT Dashboard"
    color: "#13151a"

    property real smoothSpeed: vehicleData.speed
    property real smoothRpm:   vehicleData.rpm
    property string currentGear: smoothSpeed < 1 ? "P" :
                                  smoothSpeed < 30 ? "1" :
                                  smoothSpeed < 60 ? "2" :
                                  smoothSpeed < 90 ? "3" :
                                  smoothSpeed < 130 ? "4" :
                                  smoothSpeed < 180 ? "5" : "6"

    Behavior on smoothSpeed { NumberAnimation { duration: 50; easing.type: Easing.OutCubic } }
    Behavior on smoothRpm   { NumberAnimation { duration: 50; easing.type: Easing.OutCubic } }


    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter { name: "osm.mapping.cache.directory"; value: "/tmp/osm_map_cache" }
        PluginParameter { name: "osm.mapping.providersrepository.disabled"; value: "true" }
    }

    RouteQuery {
        id: routeQuery
    }

    RouteModel {
        id: routeModel
        plugin: mapPlugin
        query: routeQuery
        autoUpdate: false


        onStatusChanged: {
            if (status === RouteModel.Ready) {
                console.log("[Route] Tính toán đường đi thành công!")
                var route = routeModel.get(0)
                var distKm = (route.distance / 1000).toFixed(1)

                // Tính giờ đến (ETA) = Giờ hiện tại + Thời gian di chuyển ( travelTime tính bằng giây )
                var durationMin = Math.round(route.travelTime / 60)
                var now = new Date()
                now.setMinutes(now.getMinutes() + durationMin)
                var etaHrsStr = now.getHours().toString().padStart(2, '0')
                var etaMinStr = now.getMinutes().toString().padStart(2, '0')

                mainMapView.etaTimeText = "ETA " + etaHrsStr + ":" + etaMinStr
                mainMapView.etaDetailText = distKm + " km · " + formatTime(route.travelTime)
            } else if (status === RouteModel.Error) {
                console.log("[Route] Lỗi định tuyến:", errorString)
            }
        }


        function formatTime(sec) {
            var mins = Math.round(sec / 60)
            if (mins < 60) return mins + " min"
            var hours = Math.floor(mins / 60)
            var remMins = mins % 60
            return hours + "h " + remMins + "m"
        }
    }


    GeocodeModel {
        id: geocodeModel
        plugin: mapPlugin

        onStatusChanged: {
            if (status === GeocodeModel.Ready) {
                if (count > 0) {
                    var location = get(0)
                    mainMapView.destLatitude = location.coordinate.latitude
                    mainMapView.destLongitude = location.coordinate.longitude
                    mainMapView.destName = location.address.text.split(',')[0]
                    mainMapView.calculateRoute()
                    mainMapView.autoCenter = true
                    console.log("[Search] Đã tìm thấy vị trí: " + location.address.text)
                } else {
                    console.log("[Search] Không tìm thấy địa điểm này!")
                }
            } else if (status === GeocodeModel.Error) {
                console.log("[Search] Lỗi tìm kiếm địa điểm:", errorString)
            }
        }
    }


    Item {
        id: topBar
        width: parent.width
        height: 48
        anchors.top: parent.top

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.verticalCenter: parent.verticalCenter
            spacing: 12

            Text {
                text: Qt.formatTime(new Date(), "hh:mm")
                font.pixelSize: 20
                font.weight: Font.Bold
                color: "#ffffff"

                Timer {
                    interval: 1000
                    running: true
                    repeat: true
                    onTriggered: parent.text = Qt.formatTime(new Date(), "hh:mm")
                }
            }
            Text {
                text: Qt.formatDate(new Date(), "dddd, MMM d")
                font.pixelSize: 13
                color: "#A0AEC0"
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Text {
            anchors.centerIn: parent
            text: "KMAGOONER"
            font.pixelSize: 16
            font.weight: Font.Bold
            font.letterSpacing: 8
            color: "#1e3a5a"
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: 24
            anchors.verticalCenter: parent.verticalCenter
            spacing: 20

            Text {
                text: "BT"
                font.pixelSize: 13
                color: "#A0AEC0"
                font.weight: Font.Bold
            }
            Text {
                text: "LTE"
                font.pixelSize: 13
                color: "#3a7bd5"
                font.weight: Font.Bold
            }
            Row {
                spacing: 6
                anchors.verticalCenter: parent.verticalCenter
                Rectangle {
                    width: 28
                    height: 14
                    radius: 3
                    color: "transparent"
                    border.color: "#2ecc71"
                    border.width: 1
                    anchors.verticalCenter: parent.verticalCenter
                    Rectangle {
                        anchors.left: parent.left
                        anchors.leftMargin: 2
                        anchors.verticalCenter: parent.verticalCenter
                        width: 20
                        height: 10
                        radius: 2
                        color: "#2ecc71"
                    }
                }
                Text {
                    text: "85%"
                    font.pixelSize: 13
                    color: "#2ecc71"
                    font.weight: Font.Bold
                }
            }
        }

        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "#1a1e26"
        }
    }


    Item {
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 16
        anchors.topMargin: 12

        Column {
            id: leftCol
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 250
            spacing: 12

            Rectangle {
                width: parent.width
                height: 240
                color: "#1a1e28"
                radius: 16
                border.color: "#22283a"
                border.width: 1

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 42
                    width: 150
                    height: 16
                    radius: 100
                    color: "#3a7bd5"
                    opacity: 0.12
                }

                Image {
                    anchors.top: parent.top
                    anchors.topMargin: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 40
                    height: 130
                    fillMode: Image.PreserveAspectFit
                    source: "assets/VF8.png"
                    antialiasing: true
                    smooth: true

                    SequentialAnimation on anchors.topMargin {
                        loops: Animation.Infinite
                        NumberAnimation { to: 18; duration: 2200; easing.type: Easing.InOutSine }
                        NumberAnimation { to: 15; duration: 2200; easing.type: Easing.InOutSine }
                    }
                }

                Row {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 12

                    Column {
                        spacing: 2
                        Text { text: "340 km"; color: "#ffffff"; font.pixelSize: 13; font.weight: Font.Bold }
                        Text { text: "Range"; color: "#A0AEC0"; font.pixelSize: 9; font.weight: Font.Medium }
                    }

                    Rectangle { width: 1; height: 24; color: "#22283a" }
                    Column {
                        spacing: 2
                        Text { text: "15.2"; color: "#3a7bd5"; font.pixelSize: 13; font.weight: Font.Bold }
                        Text { text: "kWh/100"; color: "#A0AEC0"; font.pixelSize: 9; font.weight: Font.Medium }
                    }
                    Rectangle { width: 1; height: 24; color: "#22283a" }
                    Column {
                        spacing: 2
                        Text { text: "85%"; color: "#2ecc71"; font.pixelSize: 13; font.weight: Font.Bold }
                        Text { text: "Battery"; color: "#A0AEC0"; font.pixelSize: 9; font.weight: Font.Medium }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: parent.height - 240 - 12
                color: "#1a1e28"
                radius: 16
                border.color: "#22283a"
                border.width: 1

                Row {
                    anchors.top: parent.top
                    anchors.topMargin: 14
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    spacing: 0

                    Text {
                        text: "SPEED"
                        font.pixelSize: 10
                        font.letterSpacing: 2
                        color: "#A0AEC0"
                    }

                    Item { width: parent.width - 50 - 80; height: 1 }

                    Row {
                        spacing: 10
                        Canvas {
                            id: leftSignalCanvas
                            width: 16; height: 12
                            opacity: vehicleData.leftOn ? 1.0 : 0.25
                            onPaint: {
                                var c = getContext("2d"); c.clearRect(0,0,width,height);
                                c.fillStyle = vehicleData.leftOn ? "#2ecc71" : "#4A5568"
                                c.beginPath(); c.moveTo(18,4); c.lineTo(7,4); c.lineTo(7,0);
                                c.lineTo(0,5); c.lineTo(7,12); c.lineTo(7,8); c.lineTo(16,8);
                                c.closePath(); c.fill();
                            }
                            SequentialAnimation on opacity {
                                running: vehicleData.leftOn; loops: Animation.Infinite
                                NumberAnimation { to: 0.15; duration: 380 }
                                NumberAnimation { to: 1.0; duration: 380 }
                            }
                            Connections { target: vehicleData
                                function onLeftOnChanged() { leftSignalCanvas.requestPaint() } }
                        }

                        Text {
                            text: "⚠"
                            font.pixelSize: 13
                            color: vehicleData.hazard ? "#e67e22" : "#4A5568"
                            SequentialAnimation on opacity {
                                running: vehicleData.hazard; loops: Animation.Infinite
                                NumberAnimation { to: 0.15; duration: 300 }
                                NumberAnimation { to: 1.0; duration: 300 }
                            }
                        }

                        Canvas {
                            id: rightSignalCanvas
                            width: 16; height: 12
                            opacity: vehicleData.rightOn ? 1.0 : 0.25
                            onPaint: {
                                var c = getContext("2d"); c.clearRect(0,0,width,height);
                                c.fillStyle = vehicleData.rightOn ? "#2ecc71" : "#4A5568"
                                c.beginPath(); c.moveTo(0,4); c.lineTo(9,4); c.lineTo(9,0);
                                c.lineTo(16,5); c.lineTo(9,12); c.lineTo(9,8); c.lineTo(0,8);
                                c.closePath(); c.fill();
                            }
                            SequentialAnimation on opacity {
                                running: vehicleData.rightOn; loops: Animation.Infinite
                                NumberAnimation { to: 0.15; duration: 380 }
                                NumberAnimation { to: 1.0; duration: 380 }
                            }
                            Connections { target: vehicleData
                                function onRightOnChanged() { rightSignalCanvas.requestPaint() } }
                        }
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 24
                    text: Math.round(smoothSpeed)
                    font.pixelSize: 72; font.weight: Font.Thin
                    color: "#ffffff"
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 104
                    text: "km/h"
                    font.pixelSize: 11; font.letterSpacing: 2
                    color: "#A0AEC0"
                }

                Item {
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.leftMargin: 16; anchors.rightMargin: 16
                    anchors.top: parent.top; anchors.topMargin: 128
                    height: 24

                    Rectangle {
                        id: rpmBg
                        width: parent.width; height: 3; radius: 2
                        color: "#0e1420"
                        Rectangle {
                            width: (smoothRpm / 8000) * parent.width
                            height: 3; radius: 2
                            color: smoothRpm > 6500 ? "#e74c3c" :
                                   smoothRpm > 5000 ? "#e67e22" : "#3a7bd5"
                            Behavior on width { NumberAnimation { duration: 100 } }
                            Behavior on color { ColorAnimation { duration: 200 } }
                        }
                    }
                    Text {
                        anchors.right: parent.right
                        anchors.top: rpmBg.bottom; anchors.topMargin: 4
                        text: Math.round(smoothRpm) + " rpm"
                        font.pixelSize: 10; color: "#A0AEC0"
                        font.weight: Font.Medium
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom; anchors.bottomMargin: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 200; height: 32; radius: 16
                    color: "#13151a"
                    Row {
                        anchors.centerIn: parent; spacing: 2
                        Repeater {
                            model: ["P", "R", "N", "D"]
                            Rectangle {
                                width: 44; height: 24; radius: 12
                                color: currentGear === modelData ? "#3a7bd5" : "transparent"
                                Behavior on color { ColorAnimation { duration: 200 } }
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData
                                    font.pixelSize: 12; font.weight: Font.Bold
                                    color: currentGear === modelData ? "#ffffff" : "#4A5568"
                                }
                            }
                        }
                    }
                }
            }
        }


        Column {
            anchors.left: leftCol.right; anchors.leftMargin: 12
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            spacing: parent.height * 0.02


            Rectangle {
                width: parent.width
                height: parent.height * 0.58
                color: "#1a1e28"
                radius: 16
                border.color: "#22283a"
                border.width: 1
                clip: true

                MapView {
                    id: mainMapView
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: -24

                    map.plugin: mapPlugin
                    map.tilt: 60
                    map.bearing: 335
                    map.center: QtPositioning.coordinate(vehicleData.latitude, vehicleData.longitude)
                    map.zoomLevel: 15

                    property bool autoCenter: true
                    property double destLatitude: 20.983250
                    property double destLongitude: 105.798800
                    property string destName: "Học viện Mật mã"

                    property string etaTimeText: "ETA --:--"
                    property string etaDetailText: "-- km · -- min"

                    function calculateRoute() {
                        routeQuery.clearWaypoints()
                        routeQuery.addWaypoint(QtPositioning.coordinate(vehicleData.latitude, vehicleData.longitude))
                        routeQuery.addWaypoint(QtPositioning.coordinate(destLatitude, destLongitude))
                        routeModel.update()
                    }

                    Component.onCompleted: {
                        calculateRoute()
                    }

                    Connections {
                        target: mainMapView.map
                        function onSupportedMapTypesChanged() {
                            var types = mainMapView.map.supportedMapTypes;
                            for (var i = 0; i < types.length; ++i) {
                                if (types[i].style === MapType.StreetMap) {
                                    mainMapView.map.activeMapType = types[i]
                                    break
                                }
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        propagateComposedEvents: true
                        onPressed: (mouse) => {
                            mainMapView.autoCenter = false
                            recenterTimer.restart()
                            mouse.accepted = false
                        }
                        onDoubleClicked: (mouse) => {
                            var clickedCoord = mainMapView.map.toCoordinate(Qt.point(mouse.x, mouse.y))
                            mainMapView.destLatitude = clickedCoord.latitude
                            mainMapView.destLongitude = clickedCoord.longitude
                            mainMapView.destName = "Custom Route"
                            mainMapView.calculateRoute()
                        }
                    }

                    map.data: [
                        MapItemView {
                            model: routeModel
                            delegate: MapRoute {
                                route: routeData
                                line.width: 8
                                line.color: "#733a7bd5"
                            }
                        },
                        MapItemView {
                            model: routeModel
                            delegate: MapRoute {
                                route: routeData
                                line.width: 4
                                line.color: "#3a7bd5"
                            }
                        },
                        MapQuickItem {
                            coordinate: QtPositioning.coordinate(vehicleData.latitude, vehicleData.longitude)
                            anchorPoint.x: 16
                            anchorPoint.y: 16

                            sourceItem: Item {
                                width: 32; height: 32
                                Canvas {
                                    id: navArrow
                                    anchors.fill: parent
                                    rotation: 30
                                    onPaint: {
                                        var ctx = getContext("2d")
                                        ctx.clearRect(0, 0, width, height)
                                        ctx.shadowColor = "rgba(0, 0, 0, 0.35)"
                                        ctx.shadowBlur = 6
                                        ctx.shadowOffsetY = 3
                                        ctx.fillStyle = "#33ccff"
                                        ctx.beginPath()
                                        ctx.moveTo(16, 2); ctx.lineTo(6, 26); ctx.lineTo(16, 18); ctx.closePath(); ctx.fill()
                                        ctx.shadowColor = "transparent"; ctx.shadowBlur = 0
                                        ctx.fillStyle = "#0066ff"
                                        ctx.beginPath()
                                        ctx.moveTo(16, 2); ctx.lineTo(26, 26); ctx.lineTo(16, 18); ctx.closePath(); ctx.fill()
                                    }
                                }
                            }
                        },
                        MapQuickItem {
                            coordinate: QtPositioning.coordinate(mainMapView.destLatitude, mainMapView.destLongitude)
                            anchorPoint.x: 15
                            anchorPoint.y: 35

                            sourceItem: Item {
                                width: 30; height: 35
                                Canvas {
                                    anchors.fill: parent
                                    onPaint: {
                                        var ctx = getContext("2d")
                                        ctx.clearRect(0, 0, width, height)
                                        ctx.shadowColor = "rgba(0, 0, 0, 0.4)"
                                        ctx.shadowBlur = 4
                                        ctx.shadowOffsetY = 4
                                        ctx.fillStyle = "#e74c3c"
                                        ctx.beginPath()
                                        ctx.arc(15, 12, 10, 0, 2 * Math.PI)
                                        ctx.fill()
                                        ctx.beginPath()
                                        ctx.moveTo(5, 12)
                                        ctx.lineTo(25, 12)
                                        ctx.lineTo(15, 32)
                                        ctx.closePath()
                                        ctx.fill()
                                        ctx.shadowColor = "transparent"
                                        ctx.shadowBlur = 0
                                        ctx.fillStyle = "#ffffff"
                                        ctx.beginPath()
                                        ctx.arc(15, 12, 4, 0, 2 * Math.PI)
                                        ctx.fill()
                                    }
                                }
                            }
                        }
                    ]
                } // MapView

                Connections {
                    target: vehicleData
                    function onLatitudeChanged() {
                        if (mainMapView.autoCenter) {
                            mainMapView.map.center = QtPositioning.coordinate(vehicleData.latitude, vehicleData.longitude)
                        }
                        mainMapView.calculateRoute()
                    }
                    function onLongitudeChanged() {
                        if (mainMapView.autoCenter) {
                            mainMapView.map.center = QtPositioning.coordinate(vehicleData.latitude, vehicleData.longitude)
                        }
                        mainMapView.calculateRoute()
                    }
                }

                Timer {
                    id: recenterTimer
                    interval: 10000
                    running: !mainMapView.autoCenter
                    repeat: false
                    onTriggered: {
                        mainMapView.autoCenter = true
                        mainMapView.map.center = QtPositioning.coordinate(vehicleData.latitude, vehicleData.longitude)
                    }
                }

                Rectangle {
                    id: compassWidget
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.top: parent.top
                    anchors.topMargin: 16
                    width: 36; height: 36; radius: 18
                    color: "#111318"; border.color: "#222830"; border.width: 1; opacity: 0.90

                    Canvas {
                        id: compassNeedle
                        anchors.fill: parent
                        rotation: -mainMapView.map.bearing
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.clearRect(0, 0, width, height)
                            ctx.fillStyle = "#e74c3c"
                            ctx.beginPath(); ctx.moveTo(18, 18); ctx.lineTo(14, 18); ctx.lineTo(18, 6); ctx.closePath(); ctx.fill()
                            ctx.fillStyle = "#c0392b"
                            ctx.beginPath(); ctx.moveTo(18, 18); ctx.lineTo(22, 18); ctx.lineTo(18, 6); ctx.closePath(); ctx.fill()
                            ctx.fillStyle = "#ecf0f1"
                            ctx.beginPath(); ctx.moveTo(18, 18); ctx.lineTo(14, 18); ctx.lineTo(18, 30); ctx.closePath(); ctx.fill()
                            ctx.fillStyle = "#bdc3c7"
                            ctx.beginPath(); ctx.moveTo(18, 18); ctx.lineTo(22, 18); ctx.lineTo(18, 30); ctx.closePath(); ctx.fill()
                            ctx.fillStyle = "#2c3e50"
                            ctx.beginPath(); ctx.arc(18, 18, 3, 0, 2 * Math.PI); ctx.fill()
                        }
                    }
                }

                Column {
                    anchors.left: parent.left; anchors.leftMargin: 16
                    anchors.bottom: parent.bottom; anchors.bottomMargin: 16; spacing: 8

                    Rectangle {
                        width: 32; height: 32; radius: 8; color: "#111318"; border.color: "#222830"; border.width: 1; opacity: 0.90
                        Text { text: "+"; color: "#ffffff"; font.pixelSize: 18; font.weight: Font.Bold; anchors.centerIn: parent }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (mainMapView.map.zoomLevel < 20) mainMapView.map.zoomLevel += 1
                            }
                        }
                    }
                    Rectangle {
                        width: 32; height: 32; radius: 8; color: "#111318"; border.color: "#222830"; border.width: 1; opacity: 0.90
                        Text { text: "-"; color: "#ffffff"; font.pixelSize: 22; font.weight: Font.Bold; anchors.centerIn: parent }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (mainMapView.map.zoomLevel > 1) mainMapView.map.zoomLevel -= 1
                            }
                        }
                    }
                }

                Rectangle {
                    id: recenterBtn
                    visible: !mainMapView.autoCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 42
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 110; height: 28; radius: 14
                    color: "#111318"; border.color: "#222830"; border.width: 1; opacity: 0.92

                    Row {
                        anchors.centerIn: parent
                        spacing: 6
                        Text { text: "🎯"; font.pixelSize: 11; color: "#2ecc71" }
                        Text { text: "Re-center"; font.pixelSize: 11; font.weight: Font.Bold; color: "#ffffff" }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            mainMapView.autoCenter = true
                            mainMapView.map.center = QtPositioning.coordinate(vehicleData.latitude, vehicleData.longitude)
                        }
                    }
                }

                Rectangle {
                    id: searchBarWidget
                    anchors.top: parent.top
                    anchors.topMargin: 16
                    anchors.left: compassWidget.right
                    anchors.leftMargin: 12
                    width: 260
                    height: 36
                    color: "#111318"
                    radius: 10
                    border.color: "#222830"
                    border.width: 1
                    opacity: 0.95

                    TextField {
                        id: searchInput
                        anchors.left: parent.left
                        anchors.right: dropDownArrow.left
                        anchors.leftMargin: 12
                        anchors.rightMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        placeholderText: "🔍  Search destination..."
                        placeholderTextColor: "#a0aec0"
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        background: null

                        onAccepted: {
                            if (text.trim() !== "") {
                                geocodeModel.query = text.trim()
                                geocodeModel.update()
                                focus = false
                            }
                        }
                    }

                    Text {
                        id: dropDownArrow
                        anchors.right: parent.right
                        anchors.rightMargin: 12
                        anchors.verticalCenter: parent.verticalCenter
                        text: "▼"
                        color: "#a0aec0"
                        font.pixelSize: 10

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                destinationMenu.visible = !destinationMenu.visible
                            }
                        }
                    }

                    Rectangle {
                        id: destinationMenu
                        visible: false
                        anchors.top: parent.bottom
                        anchors.topMargin: 6
                        anchors.left: parent.left
                        width: parent.width
                        height: 110
                        color: "#111318"
                        radius: 8
                        border.color: "#222830"
                        border.width: 1

                        Column {
                            anchors.fill: parent
                            anchors.margins: 4
                            spacing: 2

                            Rectangle {
                                width: parent.width - 8; height: 30; radius: 4
                                color: "transparent"
                                Text { anchors.centerIn: parent; text: "📍 Học viện Mật Mã"; color: "#2ecc71"; font.pixelSize: 12; font.weight: Font.Medium }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        mainMapView.destLatitude = 20.983250
                                        mainMapView.destLongitude = 105.798800
                                        mainMapView.destName = "Học viện Mật Mã"
                                        searchInput.text = "Học viện Mật Mã"
                                        mainMapView.calculateRoute()
                                        destinationMenu.visible = false
                                        mainMapView.autoCenter = true
                                    }
                                }
                            }

                            Rectangle {
                                width: parent.width - 8; height: 30; radius: 4
                                color: "transparent"
                                Text { anchors.centerIn: parent; text: "📍 Hồ Hoàn Kiếm"; color: "#e2e8f0"; font.pixelSize: 12; font.weight: Font.Medium }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        mainMapView.destLatitude = 21.028500
                                        mainMapView.destLongitude = 105.854200
                                        mainMapView.destName = "Hồ Hoàn Kiếm"
                                        searchInput.text = "Hồ Hoàn Kiếm"
                                        mainMapView.calculateRoute()
                                        destinationMenu.visible = false
                                        mainMapView.autoCenter = true
                                    }
                                }
                            }

                            Rectangle {
                                width: parent.width - 8; height: 30; radius: 4
                                color: "transparent"
                                Text { anchors.centerIn: parent; text: "📍 Aeon Mall Hà Đông"; color: "#e2e8f0"; font.pixelSize: 12; font.weight: Font.Medium }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        mainMapView.destLatitude = 20.978800
                                        mainMapView.destLongitude = 105.761800
                                        mainMapView.destName = "Aeon Mall Hà Đông"
                                        searchInput.text = "Aeon Mall Hà Đông"
                                        mainMapView.calculateRoute()
                                        destinationMenu.visible = false
                                        mainMapView.autoCenter = true
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    id: etaWidget
                    anchors.top: parent.top; anchors.topMargin: 16
                    anchors.right: parent.right; anchors.rightMargin: 16
                    width: 175; height: 68; color: "#111318"; radius: 12; border.color: "#222830"; border.width: 1; opacity: 0.93

                    Column {
                        anchors.centerIn: parent; spacing: 4
                        Text { text: mainMapView.etaTimeText; color: "#ffffff"; font.pixelSize: 14; font.weight: Font.Bold; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: mainMapView.etaDetailText; color: "#a0aec0"; font.pixelSize: 11; font.weight: Font.Medium; anchors.horizontalCenter: parent.horizontalCenter }
                    }
                }
            }


            Row {
                width: parent.width
                height: parent.height * 0.40
                spacing: 12


                Rectangle {
                    width: (parent.width - 12) * 0.55
                    height: parent.height
                    color: "#1a1e28"
                    radius: 16
                    border.color: "#22283a"
                    border.width: 1

                    Row {
                        anchors.top: parent.top
                        anchors.topMargin: parent.height * 0.07
                        anchors.left: parent.left
                        anchors.leftMargin: 16
                        spacing: 12

                        Rectangle {
                            width: 44; height: 44; radius: 8
                            color: "#0e1420"
                            border.color: "#22283a"; border.width: 1

                            Rectangle {
                                anchors.centerIn: parent
                                width: 16; height: 16; radius: 8
                                color: "#2ecc71"
                                opacity: vehicleData.mediaPlaying ? 0.4 : 0.1
                            }
                            Text {
                                anchors.centerIn: parent
                                text: "♪"
                                font.pixelSize: 16
                                color: vehicleData.mediaPlaying ? "#2ecc71" : "#405060"
                            }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text { text: vehicleData.mediaTitle; color: "#2ecc71"; font.pixelSize: 13; font.weight: Font.Bold }
                            Text { text: vehicleData.mediaArtist; color: "#A0AEC0"; font.pixelSize: 11; font.weight: Font.Medium }
                        }
                    }

                    Item {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 16
                        anchors.rightMargin: 16
                        anchors.top: parent.top
                        anchors.topMargin: parent.height * 0.42 // Tự động co giãn vị trí thanh tiến trình
                        height: 16

                        Rectangle {
                            width: parent.width; height: 2; radius: 1; color: "#0e1420"
                            Rectangle {
                                width: parent.width * vehicleData.mediaPosition
                                height: 2; radius: 1; color: "#2ecc71"
                            }
                        }

                        Text {
                            anchors.left: parent.left
                            anchors.bottom: parent.top
                            text: vehicleData.mediaPosText
                            font.pixelSize: 9
                            color: "#2ecc71"
                            font.weight: Font.Bold
                        }
                        Text {
                            anchors.right: parent.right
                            anchors.bottom: parent.top
                            text: vehicleData.mediaDurText
                            font.pixelSize: 9
                            color: "#A0AEC0"
                            font.weight: Font.Bold
                        }
                    }

                    Row {
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: parent.height * 0.06 // Co giãn phím bấm theo đáy
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 14

                        Text {
                            text: "⏮"
                            color: "#A0AEC0"
                            font.pixelSize: 20
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                onClicked: vehicleData.mediaPrevious()
                            }
                        }

                        Rectangle {
                            width: 32; height: 32; radius: 16; color: "#2ecc71"
                            Text {
                                anchors.centerIn: parent
                                text: vehicleData.mediaPlaying ? "⏸" : "▶"
                                color: "#ffffff"; font.pixelSize: 13
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    vehicleData.mediaPlayPause()
                                }
                            }
                        }

                        Text {
                            text: "⏭"
                            color: "#A0AEC0"
                            font.pixelSize: 20
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                onClicked: vehicleData.mediaNext()
                            }
                        }

                        Rectangle {
                            width: 1
                            height: 16
                            color: "#22283a"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Canvas {
                            id: volDownBtn
                            width: 18; height: 18
                            anchors.verticalCenter: parent.verticalCenter
                            opacity: 0.7
                            onPaint: {
                                var ctx = getContext("2d"); ctx.clearRect(0,0,width,height);
                                ctx.fillStyle = "#A0AEC0"; ctx.beginPath();
                                ctx.rect(1, 5, 4, 8);
                                ctx.moveTo(5, 5); ctx.lineTo(10, 1); ctx.lineTo(10, 17); ctx.lineTo(5, 13);
                                ctx.closePath(); ctx.fill();
                                ctx.strokeStyle = "#A0AEC0"; ctx.lineWidth = 1.2;
                                ctx.beginPath(); ctx.moveTo(13, 9); ctx.lineTo(17, 9); ctx.stroke();
                            }
                            MouseArea { anchors.fill: parent; onClicked: vehicleData.volumeDown() }
                        }

                        Item {
                            width: 50; height: 16
                            anchors.verticalCenter: parent.verticalCenter
                            Rectangle {
                                id: volBg
                                width: parent.width; height: 2; radius: 1; color: "#0e1420"
                                anchors.verticalCenter: parent.verticalCenter
                                Rectangle {
                                    width: parent.width * vehicleData.volume
                                    height: parent.height; radius: 1; color: "#2ecc71"
                                }
                            }
                        }

                        Canvas {
                            id: volUpBtn
                            width: 18; height: 18
                            anchors.verticalCenter: parent.verticalCenter
                            opacity: 0.7
                            onPaint: {
                                var ctx = getContext("2d"); ctx.clearRect(0,0,width,height);
                                ctx.fillStyle = "#A0AEC0"; ctx.beginPath();
                                ctx.rect(1, 5, 4, 8);
                                ctx.moveTo(5, 5); ctx.lineTo(10, 1); ctx.lineTo(10, 17); ctx.lineTo(5, 13);
                                ctx.closePath(); ctx.fill();
                                ctx.strokeStyle = "#A0AEC0"; ctx.lineWidth = 1.2;
                                ctx.beginPath();
                                ctx.moveTo(13, 9); ctx.lineTo(17, 9);
                                ctx.moveTo(15, 7);  ctx.lineTo(15, 11);
                                ctx.stroke();
                            }
                            MouseArea { anchors.fill: parent; onClicked: vehicleData.volumeUp() }
                        }
                    }
                }


                Rectangle {
                    width: (parent.width - 12) * 0.45; height: parent.height
                    radius: 16; clip: true
                    color: "#1e3a5a"
                    border.color: "#22283a"; border.width: 1

                    Item {
                        id: topWeatherSection
                        anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right
                        anchors.topMargin: parent.height * 0.07; anchors.leftMargin: 16; anchors.rightMargin: 16; height: 60

                        Column {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text { text: vehicleData.weatherCity; color: "#e2e8f0"; font.pixelSize: 11; font.weight: Font.Medium }
                            Text { text: vehicleData.weatherDesc; color: "#ffffff"; font.pixelSize: 13; font.weight: Font.Bold }
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            text: vehicleData.weatherTemp
                            color: "#ffffff"; font.pixelSize: 38; font.weight: Font.Thin
                        }
                    }

                    Row {
                        id: bottomWeatherRow
                        anchors.bottom: parent.bottom; anchors.bottomMargin: parent.height * 0.08
                        anchors.left: parent.left; anchors.leftMargin: 16; width: parent.width - 32; spacing: 28

                        Column {
                            spacing: 1
                            Text { text: "Humidity"; color: "#92a6c0"; font.pixelSize: 9; font.weight: Font.Bold }
                            Text { text: vehicleData.weatherHumid; color: "#ffffff"; font.pixelSize: 12; font.weight: Font.Bold }
                        }
                        Column {
                            spacing: 1
                            Text { text: "Wind"; color: "#92a6c0"; font.pixelSize: 9; font.weight: Font.Bold }
                            Text { text: vehicleData.weatherWind; color: "#ffffff"; font.pixelSize: 12; font.weight: Font.Bold }
                        }
                    }

                    Rectangle { anchors.bottom: bottomWeatherRow.top; anchors.bottomMargin: parent.height * 0.05; anchors.left: parent.left; anchors.right: parent.right; anchors.leftMargin: 16; anchors.rightMargin: 16; height: 1; color: "#ffffff"; opacity: 0.1 }
                }
            }
        }
    }
}
