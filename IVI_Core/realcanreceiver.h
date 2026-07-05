#ifndef REALCANRECEIVER_H
#define REALCANRECEIVER_H

#include <QObject>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QTimer>

class RealCanReceiver : public QObject {
    Q_OBJECT
public:
    explicit RealCanReceiver(QObject *parent = nullptr);
    ~RealCanReceiver();
    void start();

signals:
    void speedReceived(double speed);
    void rpmReceived(double rpm);
    void leftSignalReceived(bool on);
    void rightSignalReceived(bool on);
    void hazardReceived(bool on);

private slots:
    void processReceivedFrames();
    void tryReconnect();

private:
    void connectCanDevice();

    QCanBusDevice *m_canDevice;
    QTimer        *m_reconnectTimer;
};

#endif // REALCANRECEIVER_H
