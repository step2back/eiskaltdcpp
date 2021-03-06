/***************************************************************************
*                                                                         *
*   Copyright (C) 2009-2010  Alexandr Tkachev <tka4ev@gmail.com>          *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

// Created on: 17.08.2009

#ifndef SERVERTHREAD_H_
#define SERVERTHREAD_H_

#include "dcpp/QueueManagerListener.h"
#include "dcpp/TimerManager.h"
#include "dcpp/LogManager.h"
#include "dcpp/ClientListener.h"
#include "dcpp/ShareManager.h"
#include "dcpp/Thread.h"
#include "dcpp/SearchManager.h"
#include "dcpp/SearchManagerListener.h"
#include "dcpp/SearchResult.h"
#include "dcpp/Singleton.h"

class ServerThread :
        private TimerManagerListener,
        private QueueManagerListener,
        private LogManagerListener,
        private ClientListener,
        public SearchManagerListener,
        public Thread,
        public Singleton<ServerThread>
{

public:
    ServerThread();
    ~ServerThread();

    void Resume();
    void Close();
    void WaitFor();

    void connectClient(const string& address, const string& encoding);
    void disconnectClient(const string& address);
    void sendMessage(const string& hubUrl, const string& message);
    void listConnectedClients(string& listhubs,const string& separator);
    bool findHubInConnectedClients(const string& hub);
    string sendPrivateMessage(const string& hub, const string& nick, const string& message);
    string getFileList_client(const string& hub, const string& nick, bool match);
    void getChatPubFromClient(string& chat, const string& hub, const string& separator);
    bool sendSearchonHubs(const string& search, const int& mode, const int& sizemode, const int& sizetype, const double& size, const string& huburls);
    void listSearchStrings(string& listsearchstrings, const string& separator);
    void returnSearchResults(vector<StringMap>& resultarray, const int& index, const string& huburls);
    void addStringinSearchList(const string& s);

private:

    virtual int run();
    void startSocket(bool changed);
    void autoConnect();
    void showPortsError(const std::string& port);
    bool disconnect_all();
    void parseSearchResult_gui(SearchResultPtr result, StringMap &resultMap);
    string revertSeparator(const string &ps);
    vector<string> retlistsearchs;
    typedef struct {
            deque<string> curchat;
            Client* curclient;
            unordered_map<string, SearchResultList> cursearchresult;
    } CurHub;
    //unordered_map<string, SearchResultList> gsearchresult;
    SearchResultList gsearchresult;
    typedef unordered_map <string, CurHub> ClientMap;
    typedef ClientMap::const_iterator ClientIter;
    static ClientMap clientsMap;

    // TimerManagerListener
    virtual void on(TimerManagerListener::Second, uint64_t aTick) noexcept;

    // ClientListener
    virtual void on(Connecting, Client* cur) noexcept;
    virtual void on(Connected, Client* cur) noexcept;
    virtual void on(UserUpdated, Client* cur, const OnlineUserPtr&) noexcept;
    virtual void on(UsersUpdated, Client* cur, const OnlineUserList&) noexcept;
    virtual void on(UserRemoved, Client* cur, const OnlineUserPtr&) noexcept;
    virtual void on(Redirect, Client* cur, const string&) noexcept;
    virtual void on(Failed, Client* cur, const string&) noexcept;
    virtual void on(GetPassword, Client* cur) noexcept;
    virtual void on(HubUpdated, Client* cur) noexcept;
    virtual void on(StatusMessage, Client* cur, const string&, int = ClientListener::FLAG_NORMAL) noexcept;
    virtual void on(ClientListener::Message, Client*, const ChatMessage&) noexcept;
    virtual void on(NickTaken, Client* cur) noexcept;
    virtual void on(SearchFlood, Client* cur, const string&) noexcept;

    //SearchManagerListener
    virtual void on(SearchManagerListener::SR, const SearchResultPtr &result) noexcept;

    int64_t lastUp;
    int64_t lastDown;
    uint64_t lastUpdate;

    CriticalSection shutcs;
    static const int maxLines = 1000;
};

#endif /* SERVERTHREAD_H_ */
