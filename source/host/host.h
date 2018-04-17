//
// PROJECT:         Aspia
// FILE:            host/host.h
// LICENSE:         GNU Lesser General Public License 2.1
// PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
//

#ifndef _ASPIA_HOST__HOST_H
#define _ASPIA_HOST__HOST_H

#include <QObject>
#include <QPointer>
#include <QQueue>

#include "protocol/authorization.pb.h"

namespace aspia {

class Channel;
class HostProcess;
class IpcChannel;
class IpcServer;

class Host : public QObject
{
    Q_OBJECT

public:
    enum State
    {
        StoppedState,
        StartingState,
        DetachedState,
        AttachedState
    };
    Q_ENUM(State);

    Host(proto::auth::SessionType session_type, Channel* channel, QObject* parent = nullptr);
    ~Host();

public slots:
    void start();
    void stop();
    void sessionChanged(quint32 event, quint32 session_id);

signals:
    void started();
    void finished();

protected:
    void timerEvent(QTimerEvent* event) override;

private slots:
    void networkMessage(const QByteArray& buffer);
    void ipcMessage(const QByteArray& buffer);
    void ipcServerStarted(const QString& channel_id);
    void ipcNewConnection(IpcChannel* channel);
    void attachSession(quint32 session_id);
    void dettachSession();

private:
    static const quint32 kInvalidSessionId = 0xFFFFFFFF;

    proto::auth::SessionType session_type_;
    quint32 session_id_ = kInvalidSessionId;
    int attach_timer_id_ = 0;
    State state_ = StoppedState;

    QPointer<Channel> network_channel_;
    QPointer<IpcChannel> ipc_channel_;
    QPointer<HostProcess> session_process_;

    QQueue<QByteArray> read_queue_;

    Q_DISABLE_COPY(Host)
};

} // namespace aspia

#endif // _ASPIA_HOST__HOST_H
