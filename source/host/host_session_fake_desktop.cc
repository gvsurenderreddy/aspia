//
// Aspia Project
// Copyright (C) 2018 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#include "host/host_session_fake_desktop.h"
#include "codec/video_encoder_vpx.h"
#include "codec/video_encoder_zstd.h"
#include "codec/video_util.h"
#include "common/desktop_session_constants.h"
#include "common/message_serialization.h"
#include "desktop/desktop_frame_simple.h"

namespace host {

SessionFakeDesktop::SessionFakeDesktop(QObject* parent)
    : SessionFake(parent)
{
    // Nothing
}

void SessionFakeDesktop::startSession()
{
    proto::desktop::HostToClient message;
    message.mutable_config_request()->set_video_encodings(common::kSupportedVideoEncodings);
    emit sendMessage(common::serializeMessage(message));
}

void SessionFakeDesktop::onMessageReceived(const QByteArray& buffer)
{
    proto::desktop::ClientToHost incoming_message;

    if (!common::parseMessage(buffer, incoming_message))
    {
        LOG(LS_WARNING) << "Unable to parse message";
        emit errorOccurred();
        return;
    }

    if (incoming_message.has_config())
    {
        std::unique_ptr<codec::VideoEncoder> video_encoder(createEncoder(incoming_message.config()));
        if (!video_encoder)
        {
            LOG(LS_WARNING) << "Unable to create video encoder";
            emit errorOccurred();
            return;
        }

        std::unique_ptr<desktop::Frame> frame = createFrame();
        if (!frame)
        {
            LOG(LS_WARNING) << "Unable to create video frame";
            emit errorOccurred();
            return;
        }

        proto::desktop::HostToClient outgoing_message;
        video_encoder->encode(frame.get(), outgoing_message.mutable_video_packet());
        emit sendMessage(common::serializeMessage(outgoing_message));
    }
    else
    {
        // Other messages are ignored.
    }
}

codec::VideoEncoder* SessionFakeDesktop::createEncoder(const proto::desktop::Config& config)
{
    switch (config.video_encoding())
    {
        case proto::desktop::VIDEO_ENCODING_VP8:
            return codec::VideoEncoderVPX::createVP8();

        case proto::desktop::VIDEO_ENCODING_VP9:
            return codec::VideoEncoderVPX::createVP9();

        case proto::desktop::VIDEO_ENCODING_ZSTD:
            return codec::VideoEncoderZstd::create(
                codec::VideoUtil::fromVideoPixelFormat(
                    config.pixel_format()), config.compress_ratio());

        default:
            LOG(LS_WARNING) << "Unsupported video encoding: " << config.video_encoding();
            return nullptr;
    }
}

std::unique_ptr<desktop::Frame> SessionFakeDesktop::createFrame()
{
    static const int kWidth = 800;
    static const int kHeight = 600;

    std::unique_ptr<desktop::FrameSimple> frame =
        desktop::FrameSimple::create(desktop::Size(kWidth, kHeight), desktop::PixelFormat::ARGB());
    if (!frame)
        return nullptr;

    memset(frame->frameData(), 0, frame->stride() * frame->size().height());

    frame->updatedRegion()->addRect(desktop::Rect::makeSize(frame->size()));
    return frame;
}

} // namespace host
