#include "Settings.h"
#include "SettingsPersonal.h"
#include "SettingsConnection.h"
#include "SettingsDownloads.h"
#include "SettingsSharing.h"

#include "WulforUtil.h"

Settings::Settings()
{
    setupUi(this);

    init();

    setWindowTitle(tr("Options"));
}

Settings::~Settings(){
}

void Settings::init(){
    WulforUtil *WU = WulforUtil::getInstance();

    QListWidgetItem *item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiUSERS), tr("Personal"), listWidget);
    SettingsPersonal *personal = new SettingsPersonal(this);
    connect(this, SIGNAL(accepted()), personal, SLOT(ok()));
    widgets.insert(item, 0);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiCONNECT), tr("Connection"), listWidget);
    SettingsConnection *connection = new SettingsConnection(this);
    connect(this, SIGNAL(accepted()), connection, SLOT(ok()));
    widgets.insert(item, 1);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiDOWNLOAD), tr("Downloads"), listWidget);
    SettingsDownloads *downloads = new SettingsDownloads(this);
    connect(this, SIGNAL(accepted()), downloads, SLOT(ok()));
    widgets.insert(item, 2);

    item = new QListWidgetItem(WU->getPixmap(WulforUtil::eiFOLDER_BLUE), tr("Sharing"), listWidget);
    SettingsSharing *sharing = new SettingsSharing(this);
    connect(this, SIGNAL(accepted()), sharing, SLOT(ok()));
    widgets.insert(item, 3);

    stackedWidget->insertWidget(0, personal);
    stackedWidget->insertWidget(1, connection);
    stackedWidget->insertWidget(2, downloads);
    stackedWidget->insertWidget(3, sharing);

    stackedWidget->setCurrentIndex(0);

    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(slotItemActivated(QListWidgetItem*)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

void Settings::slotItemActivated(QListWidgetItem *item){
    if (widgets.contains(item)){
        stackedWidget->setCurrentIndex(widgets[item]);
    }
}
