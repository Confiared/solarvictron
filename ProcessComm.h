#ifndef PROCESSCOMM_H
#define PROCESSCOMM_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QTime>

class ProcessComm : public QObject
{
    Q_OBJECT
public:
    explicit ProcessComm(QObject *parent = nullptr);
    ~ProcessComm();
    static QString host;
    static QString id;
    static QString password;
    QTime ofLastStartWithoutData;

    static QString get_battery();
    static QString get_load();
    static QString get_solar();
    static QString get_voltage();
private:
    static uint64_t batterydataTime;
    static QString battery;
    static uint64_t batteryfailbackdataTime;
    static QString batteryfailback;
    static uint64_t batteryfailback2dataTime;
    static QString batteryfailback2;
    static uint64_t batteryfailback3dataTime;
    static QString batteryfailback3;
    static uint64_t batteryfailback4dataTime;
    static QString batteryfailback4;
    static uint64_t voltagedataTime;
    static QString voltage;
    static uint64_t voltagefailbackdataTime;
    static QString voltagefailback;
    static uint64_t voltagefailback2dataTime;
    static QString voltagefailback2;
    static uint64_t voltagefailback3dataTime;
    static QString voltagefailback3;
    static uint64_t voltagefailback4dataTime;
    static QString voltagefailback4;
    static QString load;
    static uint64_t solardataTime;
    static QString solar;
    static uint64_t solarfailbackdataTime;
    static QString solarfailback;
    static uint64_t solarfailback2dataTime;
    static QString solarfailback2;
    static uint64_t solarfailback3dataTime;
    static QString solarfailback3;
    static uint64_t dataTime;

    QString bufferData;
    QProcess data;
    QTimer dataTimer;
    void dataRestart();
    void data_errorOccurred(QProcess::ProcessError error);
    void data_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void data_readyReadStandardError();
    void data_readyReadStandardOutput();

    QProcess ping;
    QTimer pingTimer;
    void pingSlot();
    void ping_errorOccurred(QProcess::ProcessError error);
    void ping_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void ping_readyReadStandardError();
    void ping_readyReadStandardOutput();
signals:

};

#endif // PROCESSCOMM_H
