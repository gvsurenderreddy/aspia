//
// PROJECT:         Aspia
// FILE:            network/channel.h
// LICENSE:         GNU Lesser General Public License 2.1
// PROGRAMMERS:     Dmitry Chapyshev (dmitry@aspia.ru)
//

#ifndef _ASPIA_NETWORK__CHANNEL_H
#define _ASPIA_NETWORK__CHANNEL_H

#include <QPair>
#include <QPointer>
#include <QQueue>
#include <QTcpSocket>

namespace aspia {

class Server;

class Channel : public QObject
{
    Q_OBJECT

public:
    explicit Channel(QObject* parent);
    ~Channel();

    void connectToHost(const QString& address, int port);

signals:
    void channelConnected();
    void channelDisconnected();
    void channelError(const QString& message);
    void channelMessage(const QByteArray& buffer);
    void messageWritten(int message_id);

public slots:
    void readMessage();
    void writeMessage(int message_id, const QByteArray& buffer);
    void stopChannel();

private slots:
    void onConnected();
    void onError(QAbstractSocket::SocketError error);
    void onBytesWritten(qint64 bytes);
    void onReadyRead();

private:
    friend class Server;

    Channel(QTcpSocket* socket, QObject* parent);
    void initChannel();

    void scheduleWrite();

    using MessageSizeType = quint32;

    QPointer<QTcpSocket> socket_;

    QQueue<QPair<int, QByteArray>> write_queue_;
    MessageSizeType write_size_ = 0;
    qint64 written_ = 0;

    bool read_required_ = false;
    bool read_size_received_ = false;
    QByteArray read_buffer_;
    MessageSizeType read_size_ = 0;
    qint64 read_ = 0;

    Q_DISABLE_COPY(Channel)
};

} // namespace aspia

#endif // _ASPIA_NETWORK__CHANNEL_H
