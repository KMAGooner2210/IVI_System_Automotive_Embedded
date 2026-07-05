#include "realcanreceiver.h"
#include <QDebug>

RealCanReceiver::RealCanReceiver(QObject *parent)
    : QObject{parent}, m_canDevice(nullptr)
{
    /* Hẹn giờ thử lại mỗi 3 giây khi mất kết nối */
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(3000);
    connect(m_reconnectTimer, &QTimer::timeout,
            this, &RealCanReceiver::tryReconnect);
}


RealCanReceiver::~RealCanReceiver() {
    if (m_canDevice) {
        m_canDevice->disconnectDevice();
        delete m_canDevice;
        m_canDevice = nullptr;
    }
}

void RealCanReceiver::start() {
    connectCanDevice();
}

void RealCanReceiver::connectCanDevice() {
    if (m_canDevice) {
        m_canDevice->disconnectDevice();
        delete m_canDevice;
        m_canDevice = nullptr;
    }

    QString errorString;
    m_canDevice = QCanBus::instance()->createDevice(
        QStringLiteral("socketcan"),
        QStringLiteral("can0"), 
        &errorString
    );

    if (!m_canDevice) {
        qWarning() << "[CAN] Device creation failed:" << errorString;
        m_reconnectTimer->start();
        return;
    }

    connect(m_canDevice, &QCanBusDevice::framesReceived,
            this, &RealCanReceiver::processReceivedFrames);

   
    connect(m_canDevice, &QCanBusDevice::errorOccurred,
            this, [this](QCanBusDevice::CanBusError error) {
                qWarning() << "[CAN] Bus error occurred:" << error
                           << m_canDevice->errorString();
                m_reconnectTimer->start();
            });

    if (!m_canDevice->connectDevice()) {
        qWarning() << "[CAN] connectDevice failed:"
                   << m_canDevice->errorString();
        m_reconnectTimer->start();
    } else {
        m_reconnectTimer->stop();
        qInfo() << "[CAN] Connected successfully on can0 @ 500kbps";
    }
}

void RealCanReceiver::tryReconnect() {
    qInfo() << "[CAN] Auto-reconnecting...";
    connectCanDevice();
}

void RealCanReceiver::processReceivedFrames() {
    if (!m_canDevice) return;

    while (m_canDevice->framesAvailable()) {
        const QCanBusFrame frame = m_canDevice->readFrame();

        
        if (frame.frameId() == 0x123 && frame.payload().size() >= 1) {
           
            emit speedReceived(static_cast<uint8_t>(frame.payload()[0]));
        }
        else if (frame.frameId() == 0x124 && frame.payload().size() >= 2) {
            
            uint16_t rpm = (static_cast<uint8_t>(frame.payload()[0]) << 8)
                         |  static_cast<uint8_t>(frame.payload()[1]);
            emit rpmReceived(static_cast<double>(rpm));
        }
        else if (frame.frameId() == 0x125 && frame.payload().size() >= 2) {
            bool left  = static_cast<uint8_t>(frame.payload()[0]) != 0;
            bool right = static_cast<uint8_t>(frame.payload()[1]) != 0;
            emit leftSignalReceived(left);
            emit rightSignalReceived(right);
            /* Cả hai cùng bật -> hazard */
            emit hazardReceived(left && right);
        }
    }
}
