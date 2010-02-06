#include "HashProgress.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/HashManager.h"
#include "dcpp/TimerManager.h"

using namespace dcpp;

HashProgress::HashProgress(QWidget *parent):
        QDialog(parent),
        autoClose(true),
        startBytes(0),
        startFiles(0),
        startTime(0)
{
    setupUi(this);

    HashManager::getInstance()->setPriority(Thread::NORMAL);

    timer = new QTimer();
    timer->setInterval(800);
    timer->setSingleShot(true);

    connect(timer, SIGNAL(timeout()), this, SLOT(timerTick()));

    timer->start();
}

HashProgress::~HashProgress(){
    timer->stop();//really need?

    delete timer;
}

void HashProgress::timerTick(){
    string path;
    int64_t bytes = 0;
    size_t files = 0;
    uint32_t tick = GET_TICK();

    HashManager::getInstance()->getStats(path, bytes, files);

    if(bytes > startBytes)
        startBytes = bytes;

    if(files > startFiles)
        startFiles = files;

    if(autoClose && files == 0) {
        accept();

        return;;
    }

    double diff = tick - startTime;

    if(diff < 1000 || files == 0 || bytes == 0) {
        stat->setText(QString(tr("-.-- files/h, %1 files left")).arg((uint32_t)files));
        speed->setText(QString(tr("-.-- B/s, %1 left").arg(QString::fromStdString(Text::toT(Util::formatBytes(bytes))))));
        left->setText(tr("-:--:-- left"));
        progress->setValue(0);
    }
    else {
        double filestat = (((double)(startFiles - files)) * 60 * 60 * 1000) / diff;
        double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

        stat->setText(QString(tr("%1 files/h, %2 files left").arg(filestat).arg((uint32_t)files)));
        speed->setText(QString(tr("%1/s, %2 left").arg(QString::fromStdString(Text::toT(Util::formatBytes((int64_t)speedStat))))
                                                  .arg(QString::fromStdString(Text::toT(Util::formatBytes(bytes))))));

        if(filestat == 0 || speedStat == 0) {
            left->setText(tr("-:--:-- left"));
        }
        else {
            double fs = files * 60 * 60 / filestat;
            double ss = bytes / speedStat;

            left->setText(QString(tr("%1 left").arg(QString::fromStdString(Text::toT(Util::formatSeconds((int64_t)(fs + ss) / 2))))));
        }
    }

    if(files == 0) {
        file->setText(tr("Done"));
    }
    else {
        file->setText(QString::fromStdString(Text::toT(path)));
    }

    if(startFiles == 0 || startBytes == 0) {
        progress->setValue(0);
    }
    else {
        progress->setValue((int)(10000 * ((0.5 * (startFiles - files)/startFiles) + 0.5 * (startBytes - bytes) / startBytes)));
    }

    timer->start();
}
