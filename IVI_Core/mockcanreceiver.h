#ifndef MOCKCANRECEIVER_H
#define MOCKCANRECEIVER_H

#include <QObject>
#include <QTimer>


class MockCanReceiver : public QObject {
    Q_OBJECT
public:
    explicit MockCanReceiver(QObject *parent = nullptr);
    void start();

signals:
    void speedReceived(double speed);
    void rpmReceived(double rpm);
    void leftSignalReceived(bool on);
    void rightSignalReceived(bool on);
    void hazardReceived(bool on);

private slots:
    void simulateTick();
    void simulateTurnSignal();

private:
    QTimer *m_dataTimer;
    QTimer *m_turnTimer;
    double  m_simSpeed;
    bool    m_speedIncreasing;
};

#endif // MOCKCANRECEIVER_H
