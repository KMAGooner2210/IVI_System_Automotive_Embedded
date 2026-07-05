#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUdpSocket>
#include <QDebug>
#include "vehicledatacontroller.h"

#ifdef Q_OS_LINUX
    #include "realcanreceiver.h"
#else
    #include "mockcanreceiver.h"
#endif

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    VehicleDataController controller;

#ifdef Q_OS_LINUX
    RealCanReceiver receiver;
    QObject::connect(&receiver, &RealCanReceiver::speedReceived, &controller, &VehicleDataController::setSpeed);
    QObject::connect(&receiver, &RealCanReceiver::rpmReceived, &controller, &VehicleDataController::setRpm);
    QObject::connect(&receiver, &RealCanReceiver::leftSignalReceived, &controller, &VehicleDataController::setLeftOn);
    QObject::connect(&receiver, &RealCanReceiver::rightSignalReceived, &controller, &VehicleDataController::setRightOn);
    QObject::connect(&receiver, &RealCanReceiver::hazardReceived, &controller, &VehicleDataController::setHazard);
#else
    MockCanReceiver receiver;
    QObject::connect(&receiver, &MockCanReceiver::speedReceived, &controller, &VehicleDataController::setSpeed);
    QObject::connect(&receiver, &MockCanReceiver::rpmReceived, &controller, &VehicleDataController::setRpm);
    QObject::connect(&receiver, &MockCanReceiver::leftSignalReceived, &controller, &VehicleDataController::setLeftOn);
    QObject::connect(&receiver, &MockCanReceiver::rightSignalReceived, &controller, &VehicleDataController::setRightOn);
    QObject::connect(&receiver, &MockCanReceiver::hazardReceived, &controller, &VehicleDataController::setHazard);
#endif

    QUdpSocket udpSocket;
    udpSocket.bind(5555, QUdpSocket::ShareAddress);
    QObject::connect(&udpSocket, &QUdpSocket::readyRead, [&]() {
        while (udpSocket.hasPendingDatagrams()) {
            QByteArray datagram; datagram.resize(udpSocket.pendingDatagramSize());
            QHostAddress senderIp; udpSocket.readDatagram(datagram.data(), datagram.size(), &senderIp);
            controller.setEsp32Ip(senderIp.toString()); 
            QString strLine = QString::fromUtf8(datagram).trimmed();
            if (strLine.startsWith("$GPS")) {
                QStringList parts = strLine.split(',');
                if (parts.size() >= 5) {
                    double lat = parts[1].toDouble(); double lng = parts[2].toDouble();
                    if (lat != 0.0 && lng != 0.0) { controller.setLatitude(lat); controller.setLongitude(lng); }
                }
            }
        }
    });

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("vehicleData", &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ClusterClient/Main.qml")));
    receiver.start();
    return app.exec();
}
