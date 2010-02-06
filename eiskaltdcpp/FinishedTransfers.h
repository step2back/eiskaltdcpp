#ifndef FINISHEDTRANSFERS_H
#define FINISHEDTRANSFERS_H

#include <QWidget>
#include <QEvent>
#include <QCloseEvent>
#include <QDir>
#include <QComboBox>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/FinishedManager.h"
#include "dcpp/FinishedManagerListener.h"
#include "dcpp/Util.h"
#include "dcpp/FinishedItem.h"
#include "dcpp/User.h"
#include "dcpp/Singleton.h"

#include "ui_UIFinishedTransfers.h"
#include "Func.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"
#include "FinishedTransfersModel.h"
#include "MainWindow.h"

using namespace dcpp;

class FinishedTransfersCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1205);

    FinishedTransfersCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~FinishedTransfersCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class FinishedTransferProxy: public QWidget{
Q_OBJECT
public:
    FinishedTransferProxy(QWidget *parent):QWidget(parent){}
    ~FinishedTransferProxy(){}

public slots:
    virtual void slotTypeChanged(int) = 0;
    virtual void slotClear() = 0;
};

template <bool isUpload>
class FinishedTransfers :
        public dcpp::FinishedManagerListener,
        private Ui::UIFinishedTransfers,
        public dcpp::Singleton< FinishedTransfers<isUpload> >,
        public ArenaWidget,
        public FinishedTransferProxy
{

typedef QMap<QString, QVariant> VarMap;
friend class dcpp::Singleton< FinishedTransfers<isUpload> >;

public:
    QWidget *getWidget() { return this;}
    QString getArenaTitle(){ return (isUpload? tr("Finished uploads"):tr("Finished downloads")); }
    QMenu *getMenu() { return NULL; }

protected:
    virtual void customEvent(QEvent *e){
        if (e->type() == FinishedTransfersCustomEvent::Event){
            FinishedTransfersCustomEvent *c_e = reinterpret_cast<FinishedTransfersCustomEvent*>(e);

            c_e->func()->call();
        }

        e->accept();
    }

    virtual void closeEvent(QCloseEvent *e){
        if (isUnload()){
            MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
            MainWindow::getInstance()->remWidgetFromArena(this);
            MainWindow::getInstance()->remArenaWidget(this);

            setAttribute(Qt::WA_DeleteOnClose);

            e->accept();
        }
        else {
            MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
            MainWindow::getInstance()->remWidgetFromArena(this);

            e->ignore();
        }
    }

private:
    FinishedTransfers(QWidget *parent = NULL) :
        FinishedTransferProxy(parent)
    {
        setupUi(this);

        model = new FinishedTransfersModel();

        treeView->setModel(model);

        loadList();

        MainWindow::getInstance()->addArenaWidget(this);
        FinishedManager::getInstance()->addListener(this);

        setUnload(false);

        QObject::connect(comboBox, SIGNAL(activated(int)), this, SLOT(slotTypeChanged(int)));
        QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(slotClear()));
    }

    ~FinishedTransfers(){
        FinishedManager::getInstance()->removeListener(this);

        delete model;
    }

    void loadList(){
        VarMap params;

        FinishedManager::getInstance()->lockLists();
        const FinishedManager::MapByFile &list = FinishedManager::getInstance()->getMapByFile(isUpload);
        const FinishedManager::MapByUser &user = FinishedManager::getInstance()->getMapByUser(isUpload);

        for (FinishedManager::MapByFile::const_iterator it = list.begin(); it != list.end(); ++it){
            params.clear();

            getParams(it->second, it->first, params);

            model->addFile(params);
        }

        for (FinishedManager::MapByUser::const_iterator uit = user.begin(); uit != user.end(); ++uit){
            params.clear();

            getParams(uit->second, uit->first, params);

            model->addUser(params);;
        }

        FinishedManager::getInstance()->unLockLists();
    }

    void getParams(const FinishedFileItemPtr& item, const string& file, FinishedTransfers::VarMap &params){
        QString nicks = "";

        params["FNAME"] = _q(file).split(QDir::separator()).last();
        params["TIME"]  = _q(Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime()));
        params["PATH"]  = _q(Util::getFilePath(file));

        for (UserList::const_iterator it = item->getUsers().begin(); it != item->getUsers().end(); ++it)
                nicks += WulforUtil::getInstance()->getNicks(it->get()->getCID()) + " ";

        params["USERS"] = nicks;
        params["TR"]    = (qlonglong)item->getTransferred();
        params["SPEED"] = (qlonglong)item->getAverageSpeed();
        params["CRC32"] = item->getCrc32Checked();
        params["TARGET"]= _q(file);
        params["ELAP"]  = (qlonglong)item->getMilliSeconds();
    }

    void getParams(const FinishedUserItemPtr& item, const UserPtr& user, FinishedTransfers::VarMap &params){
        QString files = "";

        params["TIME"]  = _q(Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime()));
        params["NICK"]  = WulforUtil::getInstance()->getNicks(user->getCID());

        for (StringList::const_iterator it = item->getFiles().begin(); it != item->getFiles().end(); ++it)
                files += _q(*it) + "; ";

        params["FILES"] = files;
        params["TR"]    = (qlonglong)item->getTransferred();
        params["SPEED"] = (qlonglong)item->getAverageSpeed();
        params["CID"]   = _q(user->getCID().toBase32());
        params["ELAP"]  = (qlonglong)item->getMilliSeconds();
    }

    void slotTypeChanged(int index){
        model->switchViewType(static_cast<FinishedTransfersModel::ViewType>(index));
    }

    void slotClear(){
        model->clearModel();
        FinishedManager::getInstance()->removeAll(isUpload);
    }

    void on(FinishedManagerListener::AddedFile, bool upload, const std::string &file, const FinishedFileItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, file, params);

            typedef Func1<FinishedTransfersModel, VarMap> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::addFile, params);

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::AddedUser, bool upload, const dcpp::UserPtr &user, const FinishedUserItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, user, params);

            typedef Func1<FinishedTransfersModel, VarMap> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::addUser, params);

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::UpdatedFile, bool upload, const std::string &file, const FinishedFileItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, file, params);

            typedef Func1<FinishedTransfersModel, VarMap> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::addFile, params);

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::RemovedFile, bool upload, const std::string &file) throw(){
        if (isUpload == upload){
            typedef Func1<FinishedTransfersModel, QString> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::remFile, _q(file));

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::UpdatedUser, bool upload, const UserPtr &user) throw(){
        if (isUpload == upload){
            const FinishedManager::MapByUser &umap = FinishedManager::getInstance()->getMapByUser(isUpload);
            FinishedManager::MapByUser::const_iterator userit = umap.find(user);
            if (userit == umap.end())
                return;

            const FinishedUserItemPtr &item = userit->second;

            VarMap params;

            getParams(item, user, params);

            typedef Func1<FinishedTransfersModel, VarMap> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::addUser, params);

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::RemovedUser, bool upload, const UserPtr &user) throw(){
        if (isUpload == upload){
            typedef Func1<FinishedTransfersModel, QString> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::remUser, _q(user->getCID().toBase32()));

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    FinishedTransfersModel *model;
};

typedef FinishedTransfers<true>  FinishedUploads;
typedef FinishedTransfers<false> FinishedDownloads;

#else

typedef FinishedTransfers<true>  FinishedUploads;
typedef FinishedTransfers<false> FinishedDownloads;

#endif // FINISHEDTRANSFERS_H
