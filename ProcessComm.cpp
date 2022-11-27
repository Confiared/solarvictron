#include "ProcessComm.h"
#include <QDateTime>
#include <QDebug>

#define TIMEOUTDATA 5*60

QString ProcessComm::host="";
QString ProcessComm::id="";
QString ProcessComm::password="";

uint64_t ProcessComm::batterydataTime;
QString ProcessComm::battery;
uint64_t ProcessComm::batteryfailbackdataTime;
QString ProcessComm::batteryfailback;
uint64_t ProcessComm::batteryfailback2dataTime;
QString ProcessComm::batteryfailback2;
uint64_t ProcessComm::batteryfailback3dataTime;
QString ProcessComm::batteryfailback3;
uint64_t ProcessComm::batteryfailback4dataTime;
QString ProcessComm::batteryfailback4;
uint64_t ProcessComm::voltagedataTime;
QString ProcessComm::voltage;
QString ProcessComm::voltagefailback;
QString ProcessComm::voltagefailback2;
QString ProcessComm::voltagefailback3;
QString ProcessComm::voltagefailback4;
QString ProcessComm::load;
uint64_t ProcessComm::solardataTime;
QString ProcessComm::solar;
uint64_t ProcessComm::solarfailbackdataTime;
QString ProcessComm::solarfailback;
uint64_t ProcessComm::solarfailback2dataTime;
QString ProcessComm::solarfailback2;
uint64_t ProcessComm::solarfailback3dataTime;
QString ProcessComm::solarfailback3;
uint64_t ProcessComm::dataTime=0;

ProcessComm::ProcessComm(QObject *parent) : QObject(parent)
{
    pingTimer.start(5000);
    pingTimer.setSingleShot(false);
    if(!connect(&pingTimer,&QTimer::timeout,this,&ProcessComm::pingSlot,Qt::QueuedConnection))
        abort();
    if(!connect(&ping,&QProcess::errorOccurred,this,&ProcessComm::ping_errorOccurred,Qt::QueuedConnection))
        abort();
    if(!connect(&ping,static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),this,&ProcessComm::ping_finished,Qt::QueuedConnection))
        abort();
    if(!connect(&ping,&QProcess::readyReadStandardError,this,&ProcessComm::ping_readyReadStandardError,Qt::QueuedConnection))
        abort();
    if(!connect(&ping,&QProcess::readyReadStandardOutput,this,&ProcessComm::ping_readyReadStandardOutput,Qt::QueuedConnection))
        abort();

    dataTimer.start(1000);
    dataTimer.setSingleShot(true);
    if(!connect(&dataTimer,&QTimer::timeout,this,&ProcessComm::dataRestart,Qt::QueuedConnection))
        abort();
    if(!connect(&data,&QProcess::errorOccurred,this,&ProcessComm::data_errorOccurred,Qt::QueuedConnection))
        abort();
    if(!connect(&data,static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished),this,&ProcessComm::data_finished,Qt::QueuedConnection))
        abort();
    if(!connect(&data,&QProcess::readyReadStandardError,this,&ProcessComm::data_readyReadStandardError,Qt::QueuedConnection))
        abort();
    if(!connect(&data,&QProcess::readyReadStandardOutput,this,&ProcessComm::data_readyReadStandardOutput,Qt::QueuedConnection))
        abort();

    pingSlot();
    ofLastStartWithoutData.restart();
}

ProcessComm::~ProcessComm()
{
    ping.terminate();
}

void ProcessComm::pingSlot()
{
    //if elapsed 60s without data, quit, systemd will restart it
    if(ofLastStartWithoutData.elapsed()>60*1000)
    {
        qDebug() << "elapsed 60s without data, quit, systemd will restart it";
        qDebug() << "pingSlot: /usr/bin/mosquitto_pub -m '' -h "+ProcessComm::host+" -t R/"+ProcessComm::id+"/system/0/Serial -u contact@confiared.com -P "+ProcessComm::password;
        qDebug() << "dataRestart: /usr/bin/mosquitto_sub -I clrgcc_ -h "+ProcessComm::host+" -u contact@confiared.com -P "+ProcessComm::password+" -t N/# -v";
        exit(0);
    }

    if(ProcessComm::host.isEmpty())
        return;
    if(ProcessComm::id.isEmpty())
        return;
    if(ProcessComm::password.isEmpty())
        return;
    //qDebug() << "pingSlot: /usr/bin/mosquitto_pub -m '' -h "+ProcessComm::host+" -t R/"+ProcessComm::id+"/system/0/Serial -u contact@confiared.com -P "+ProcessComm::password;
    ping.start("/usr/bin/mosquitto_pub",QStringList() << "-m" << "" << "-h" << ProcessComm::host << "-t" << "R/"+ProcessComm::id+"/system/0/Serial" << "-u" << "contact@confiared.com" << "-P" << ProcessComm::password);
}

void ProcessComm::ping_errorOccurred(QProcess::ProcessError error)
{
    qDebug() << "ping" << error;
}

void ProcessComm::ping_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    (void)exitCode;
    (void)exitStatus;
    //qDebug() << "ping_finished" << exitCode << exitStatus;
    ping.terminate();
}

void ProcessComm::ping_readyReadStandardError()
{
    qDebug() << "ping err" << QString::fromLocal8Bit(ping.readAllStandardError());
}

void ProcessComm::ping_readyReadStandardOutput()
{
    qDebug() << "ping out" << QString::fromLocal8Bit(ping.readAllStandardOutput());
}

void ProcessComm::dataRestart()
{
    if(ProcessComm::host.isEmpty())
        return;
    if(ProcessComm::password.isEmpty())
        return;
    //qDebug() << "dataRestart: /usr/bin/mosquitto_sub -I clrgcc_ -h "+ProcessComm::host+" -u contact@confiared.com -P "+ProcessComm::password+" -t N/# -v";
    data.terminate();
    data.kill();
    data.start("/usr/bin/mosquitto_sub",QStringList() << "-I" << "clrgcc_" << "-h" << ProcessComm::host << "-u" << "contact@confiared.com" <<
               "-P" << ProcessComm::password << "-t" << "N/#" << "-v"
    );
    ofLastStartWithoutData.restart();
}

void ProcessComm::data_errorOccurred(QProcess::ProcessError error)
{
    qDebug() << "data" << error;
}

void ProcessComm::data_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    dataTimer.start(1000);
    data.terminate();
    qDebug() << "data_finished";
    qDebug() << "data" << exitCode << exitStatus;
}

void ProcessComm::data_readyReadStandardError()
{
    qDebug() << "data err" << QString::fromLocal8Bit(data.readAllStandardError());
}

void ProcessComm::data_readyReadStandardOutput()
{
    bufferData+=QString::fromLocal8Bit(data.readAllStandardOutput());

    QString base="N/"+ProcessComm::id+"/";
    QString base2="vebus/";
    QString base2failback1="vebus/";
    QString base2failback2="system/";
    QString base2failback3="solarcharger/";
    QString base2failback1end="Dc/0/Voltage {\"value\": ";
    QString base2failback2end="Dc/Battery/Voltage {\"value\": ";
    QString base2failback3end="Dc/0/Voltage {\"value\": ";
    QString base2b="system/0/Dc/";
    QString base3="Dc/0/Voltage {\"value\": ";
    QString base3b="Vebus/Power {\"value\": ";
    QString base3c="Pv/Power {\"value\": ";
    double minBat=45.8;
    double maxBat=52.0;
    int pos=bufferData.indexOf("\n");
    while(pos>=0)
    {
        QString line=bufferData.mid(0,pos+1);
        const int removeSize=line.size();
        //qDebug() << "ProcessComm::data_readyReadStandardOutput()" << line;
        bufferData=bufferData.mid(removeSize);
        //qDebug() << "data out remain" << bufferData;
        QString linetemp=line;
        ProcessComm::dataTime=QDateTime::currentSecsSinceEpoch();

        if(linetemp.startsWith(base))
        {
            /*
            Battery:
            N/d4363917a2ce/vebus/257/Dc/0/Voltage {\"value\": 49.380001068115234}\n
            Failback:
            N/d4363917a2ce/battery/258/Dc/0/Voltage {\"value\": 47.240001678466797}\n
            N/d4363917a2ce/system/0/Dc/Battery/Voltage {\"value\": 47.240001678466797}\n
            N/d4363917a2ce/solarcharger/1/Dc/0/Voltage {\"value\": 47.569999694824219}\n
            N/d4363917a2ce/solarcharger/4/Dc/0/Voltage {\"value\": 47.569999694824219}\n

            Load:
            N/d4363917a2ce/system/0/Dc/Vebus/Power {\"value\": -805}\n"

            Solar:
            N/d4363917a2ce/system/0/Dc/Pv/Power {\"value\": 1074.6460169353486}\n
            Failback:
            N/d4363917a2ce/system/0/Dc/Pv/Power {\"value\": 834.61999195098906}\n
            N/d4363917a2ce/battery/258/Dc/0/Power {\"value\": 844.0250244140625}\n
            N/d4363917a2ce/system/0/Dc/Battery/Power {\"value\": 844.0250244140625}\n
            */
            bool havebatterypercent=false;
            linetemp=linetemp.mid(base.size());
            if(!havebatterypercent && linetemp.startsWith(base2))
            {
                linetemp=linetemp.mid(base2.size());
                const int pos2=linetemp.indexOf("/");
                if(pos2>=0)
                {
                    linetemp=linetemp.mid(pos2+1);
                    if(linetemp.startsWith(base3))
                    {
                        linetemp=linetemp.mid(base3.size());
                        const int pos3=linetemp.indexOf("}");
                        if(pos3>=0)
                        {
                            linetemp=linetemp.mid(0,pos3);
                            bool ok=false;
                            const double voltage=linetemp.toDouble(&ok);
                            if(ok)
                            {
                                //qDebug() << "Voltage:" << voltage;
                                int percent=0;
                                if(voltage<minBat)
                                    percent=0;
                                else if(voltage>maxBat)
                                    percent=100;
                                else
                                    percent=(voltage-minBat)*100/(maxBat-minBat);
                                ProcessComm::voltage=QString::number(voltage);
                                ProcessComm::battery=QString::number(percent);//double convertion to have only number
                                ProcessComm::batterydataTime=QDateTime::currentSecsSinceEpoch();
                                ofLastStartWithoutData.restart();
                                //qDebug() << "Battery percent:" << percent << "%";
                                havebatterypercent=true;
                            }
                        }
                        //qDebug() << "data out line ok:" << linetemp << removeSize;
                    }
                }
            }
            if(!havebatterypercent && linetemp.startsWith(base2failback1))
            {
                linetemp=linetemp.mid(base2failback1.size());
                const int pos2=linetemp.indexOf("/");
                if(pos2>=0)
                {
                    linetemp=linetemp.mid(pos2+1);
                    if(linetemp.startsWith(base2failback1end))
                    {
                        linetemp=linetemp.mid(base2failback1end.size());
                        const int pos3=linetemp.indexOf("}");
                        if(pos3>=0)
                        {
                            linetemp=linetemp.mid(0,pos3);
                            bool ok=false;
                            const double voltage=linetemp.toDouble(&ok);
                            if(ok)
                            {
                                //qDebug() << "failback 1) Voltage:" << voltage;
                                int percent=0;
                                if(voltage<minBat)
                                    percent=0;
                                else if(voltage>maxBat)
                                    percent=100;
                                else
                                    percent=(voltage-minBat)*100/(maxBat-minBat);
                                ProcessComm::voltagefailback=QString::number(voltage);
                                ProcessComm::batteryfailback=QString::number(percent);//double convertion to have only number
                                ProcessComm::batteryfailbackdataTime=QDateTime::currentSecsSinceEpoch();
                                ofLastStartWithoutData.restart();
                                //qDebug() << "failback 1) Battery percent:" << percent << "%";
                                havebatterypercent=true;
                            }
                        }
                        //qDebug() << "data out line ok:" << linetemp << removeSize;
                    }
                }
            }
            if(!havebatterypercent && linetemp.startsWith(base2failback2))
            {
                linetemp=linetemp.mid(base2failback2.size());
                const int pos2=linetemp.indexOf("/");
                if(pos2>=0)
                {
                    linetemp=linetemp.mid(pos2+1);
                    if(linetemp.startsWith(base2failback2end))
                    {
                        linetemp=linetemp.mid(base2failback2end.size());
                        const int pos3=linetemp.indexOf("}");
                        if(pos3>=0)
                        {
                            linetemp=linetemp.mid(0,pos3);
                            bool ok=false;
                            const double voltage=linetemp.toDouble(&ok);
                            if(ok)
                            {
                                //qDebug() << "failback 2) Voltage:" << voltage;
                                int percent=0;
                                if(voltage<minBat)
                                    percent=0;
                                else if(voltage>maxBat)
                                    percent=100;
                                else
                                    percent=(voltage-minBat)*100/(maxBat-minBat);
                                ProcessComm::voltagefailback2=QString::number(voltage);
                                ProcessComm::batteryfailback2=QString::number(percent);//double convertion to have only number
                                ProcessComm::batteryfailback2dataTime=QDateTime::currentSecsSinceEpoch();
                                ofLastStartWithoutData.restart();
                                //qDebug() << "failback 2) Battery percent:" << percent << "%";
                                havebatterypercent=true;
                            }
                        }
                        //qDebug() << "data out line ok:" << linetemp << removeSize;
                    }
                }
            }
            if(!havebatterypercent && linetemp.startsWith(base2failback3))
            {
                linetemp=linetemp.mid(base2failback3.size());
                const int pos2=linetemp.indexOf("/");
                if(pos2>=0)
                {
                    linetemp=linetemp.mid(pos2+1);
                    if(linetemp.startsWith(base2failback3end))
                    {
                        linetemp=linetemp.mid(base2failback3end.size());
                        const int pos3=linetemp.indexOf("}");
                        if(pos3>=0)
                        {
                            linetemp=linetemp.mid(0,pos3);
                            bool ok=false;
                            const double voltage=linetemp.toDouble(&ok);
                            if(ok)
                            {
                                //qDebug() << "failback 3) Voltage:" << voltage;
                                int percent=0;
                                if(voltage<minBat)
                                    percent=0;
                                else if(voltage>maxBat)
                                    percent=100;
                                else
                                    percent=(voltage-minBat)*100/(maxBat-minBat);
                                ProcessComm::voltagefailback3=QString::number(voltage);
                                ProcessComm::batteryfailback3=QString::number(percent);//double convertion to have only number
                                ProcessComm::batteryfailback3dataTime=QDateTime::currentSecsSinceEpoch();
                                ofLastStartWithoutData.restart();
                                //qDebug() << "failback 3) Battery percent:" << percent << "%";
                                havebatterypercent=true;
                            }
                        }
                        //qDebug() << "data out line ok:" << linetemp << removeSize;
                    }
                }
            }

            if(linetemp.startsWith(base2b))
            {
                linetemp=linetemp.mid(base2b.size());
                if(linetemp.startsWith(base3b))
                {
                    linetemp=linetemp.mid(base3b.size());
                    const int pos3=linetemp.indexOf("}");
                    if(pos3>=0)
                    {
                        linetemp=linetemp.mid(0,pos3);
                        bool ok=false;
                        const double load=-linetemp.toDouble(&ok);
                        if(ok)
                        {
                            //qDebug() << "Load:" << load;
                            ProcessComm::load=QString::number(load);//double convertion to have only number
                        }
                    }
                    //qDebug() << "data out line ok:" << linetemp << removeSize;
                }
                if(linetemp.startsWith(base3c))
                {
                    linetemp=linetemp.mid(base3c.size());
                    const int pos3=linetemp.indexOf("}");
                    if(pos3>=0)
                    {
                        linetemp=linetemp.mid(0,pos3);
                        bool ok=false;
                        const double solar=linetemp.toDouble(&ok);
                        if(ok)
                        {
                            //qDebug() << "Solar:" << solar;
                            ProcessComm::solar=QString::number(solar);//double convertion to have only number
                        }
                    }
                    //qDebug() << "data out line ok:" << linetemp << removeSize;
                }
            }
        }

        pos=bufferData.indexOf("\n");
    }
}

QString ProcessComm::get_battery()
{
    const uint64_t t=QDateTime::currentSecsSinceEpoch();
    if(t<=TIMEOUTDATA)
        return QString("No data, wrong time");
    if(dataTime>t)
        dataTime=0;
    if(dataTime<(t-TIMEOUTDATA))
        return QString("No data, timeout");

    if(ProcessComm::batterydataTime>t)
        ProcessComm::batterydataTime=0;
    if(ProcessComm::batteryfailbackdataTime>t)
        ProcessComm::batteryfailbackdataTime=0;
    if(ProcessComm::batteryfailback2dataTime>t)
        ProcessComm::batteryfailback2dataTime=0;
    if(ProcessComm::batteryfailback3dataTime>t)
        ProcessComm::batteryfailback3dataTime=0;

    if(ProcessComm::batterydataTime>(t-TIMEOUTDATA) && !ProcessComm::battery.isEmpty())
        return ProcessComm::battery;
    if(ProcessComm::batteryfailbackdataTime>(t-TIMEOUTDATA) && !ProcessComm::batteryfailback.isEmpty())
        return ProcessComm::batteryfailback;
    if(ProcessComm::batteryfailback2dataTime>(t-TIMEOUTDATA) && !ProcessComm::batteryfailback2.isEmpty())
        return ProcessComm::batteryfailback2;
    if(ProcessComm::batteryfailback3dataTime>(t-TIMEOUTDATA) && !ProcessComm::batteryfailback3.isEmpty())
        return ProcessComm::batteryfailback3;

    return QString("No data");
}

QString ProcessComm::get_load()
{
    const uint64_t t=QDateTime::currentSecsSinceEpoch();
    if(t<=TIMEOUTDATA)
        return QString("No data, wrong time");
    if(dataTime<(t-TIMEOUTDATA))
        return QString("No data, timeout");
    if(ProcessComm::load.isEmpty())
        return QString("No data");
    return ProcessComm::load;
}

QString ProcessComm::get_solar()
{
    const uint64_t t=QDateTime::currentSecsSinceEpoch();
    if(t<=TIMEOUTDATA)
        return QString("No data, wrong time");
    if(dataTime<(t-TIMEOUTDATA))
        return QString("No data, timeout");
    if(ProcessComm::solar.isEmpty())
        return QString("No data");
    return ProcessComm::solar;
}

QString ProcessComm::get_voltage()
{
    const uint64_t t=QDateTime::currentSecsSinceEpoch();
    if(t<=TIMEOUTDATA)
        return QString("No data, wrong time");
    if(dataTime>t)
        dataTime=0;
    if(dataTime<(t-TIMEOUTDATA))
        return QString("No data, timeout");

    if(ProcessComm::batterydataTime>t)
        ProcessComm::batterydataTime=0;
    if(ProcessComm::batteryfailbackdataTime>t)
        ProcessComm::batteryfailbackdataTime=0;
    if(ProcessComm::batteryfailback2dataTime>t)
        ProcessComm::batteryfailback2dataTime=0;
    if(ProcessComm::batteryfailback3dataTime>t)
        ProcessComm::batteryfailback3dataTime=0;

    if(ProcessComm::batterydataTime>(t-TIMEOUTDATA) && !ProcessComm::voltage.isEmpty())
        return ProcessComm::voltage;
    if(ProcessComm::batteryfailbackdataTime>(t-TIMEOUTDATA) && !ProcessComm::voltagefailback.isEmpty())
        return ProcessComm::voltagefailback;
    if(ProcessComm::batteryfailback2dataTime>(t-TIMEOUTDATA) && !ProcessComm::voltagefailback2.isEmpty())
        return ProcessComm::voltagefailback2;
    if(ProcessComm::batteryfailback3dataTime>(t-TIMEOUTDATA) && !ProcessComm::voltagefailback3.isEmpty())
        return ProcessComm::voltagefailback3;

    return QString("No data");
}
