#ifndef VEHICLEDATACONTROLLER_H
#define VEHICLEDATACONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUdpSocket>
#include <QTimer>
#include <QMediaPlayer>
#include <QAudioOutput>

struct SongData { QString title; QString artist; QString mp3Url; };

class VehicleDataController : public QObject {
    Q_OBJECT
    Q_PROPERTY(double speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(double rpm READ rpm WRITE setRpm NOTIFY rpmChanged)
    Q_PROPERTY(bool leftOn READ leftOn WRITE setLeftOn NOTIFY leftOnChanged)
    Q_PROPERTY(bool rightOn READ rightOn WRITE setRightOn NOTIFY rightOnChanged)
    Q_PROPERTY(bool hazard READ hazard WRITE setHazard NOTIFY hazardChanged)
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)

    Q_PROPERTY(QString weatherTemp READ weatherTemp NOTIFY weatherChanged)
    Q_PROPERTY(QString weatherHumid READ weatherHumid NOTIFY weatherChanged)
    Q_PROPERTY(QString weatherWind READ weatherWind NOTIFY weatherChanged)
    Q_PROPERTY(QString weatherDesc READ weatherDesc NOTIFY weatherChanged)
    Q_PROPERTY(QString weatherCity READ weatherCity NOTIFY weatherChanged)

    Q_PROPERTY(QString mediaTitle READ mediaTitle NOTIFY mediaChanged)
    Q_PROPERTY(QString mediaArtist READ mediaArtist NOTIFY mediaChanged)
    Q_PROPERTY(double mediaPosition READ mediaPosition NOTIFY mediaChanged)
    Q_PROPERTY(QString mediaPosText READ mediaPosText NOTIFY mediaChanged)
    Q_PROPERTY(QString mediaDurText READ mediaDurText NOTIFY mediaChanged)
    Q_PROPERTY(bool mediaPlaying READ mediaPlaying NOTIFY mediaChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)

public:
    explicit VehicleDataController(QObject *parent = nullptr);
    double speed() const { return m_speed; } double rpm() const { return m_rpm; }
    bool leftOn() const { return m_leftOn; } bool rightOn() const { return m_rightOn; }
    bool hazard() const { return m_hazard; } double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    QString weatherTemp() const { return m_weatherTemp; } QString weatherHumid() const { return m_weatherHumid; }
    QString weatherWind() const { return m_weatherWind; } QString weatherDesc() const { return m_weatherDesc; }
    QString weatherCity() const { return m_weatherCity; }
    QString mediaTitle() const { return m_mediaTitle; } QString mediaArtist() const { return m_mediaArtist; }
    double mediaPosition() const { return m_mediaPosition; } QString mediaPosText() const { return m_mediaPosText; }
    QString mediaDurText() const { return m_mediaDurText; } bool mediaPlaying() const { return m_mediaPlaying; }
    double volume() const { return m_volume; }

public slots:
    void setSpeed(double v); void setRpm(double v); void setLeftOn(bool v); void setRightOn(bool v);
    void setHazard(bool v); void setLatitude(double lat); void setLongitude(double lng);
    void setVolume(double vol); void setEsp32Ip(const QString &ip);
    void fetchWeatherData(); void mediaPlayPause(); void mediaNext(); void mediaPrevious();
    void volumeUp(); void volumeDown();

signals:
    void speedChanged(); void rpmChanged(); void leftOnChanged(); void rightOnChanged();
    void hazardChanged(); void latitudeChanged(); void longitudeChanged();
    void weatherChanged(); void mediaChanged(); void volumeChanged();

private slots:
    void onWeatherReplyFinished(QNetworkReply *reply);

private:
    double m_speed; double m_rpm; bool m_leftOn; bool m_rightOn; bool m_hazard; double m_latitude; double m_longitude;
    QString m_weatherTemp; QString m_weatherHumid; QString m_weatherWind; QString m_weatherDesc; QString m_weatherCity;
    QNetworkAccessManager *m_networkManager;
    QUdpSocket *m_udpAudioSocket; QString m_esp32Ip; QList<SongData> m_playlist; int m_currentSongIndex;


    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;

    int m_totalDurationSeconds;
    QString m_mediaTitle; QString m_mediaArtist; double m_mediaPosition; QString m_mediaPosText; QString m_mediaDurText;
    bool m_mediaPlaying; double m_volume;

    void sendAudioCommand(const QString &cmd); void loadCurrentSongInfo(); QString formatTime(qint64 milliseconds);
};
#endif
