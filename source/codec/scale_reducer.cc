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

#include "codec/scale_reducer.h"

#include <libyuv/scale_argb.h>

#include "desktop_capture/desktop_frame_aligned.h"

namespace aspia {

namespace {

const int kMinScaleFactor = 50;
const int kMaxScaleFactor = 100;

QSize scaleSize(const QSize& source_size, int scale_factor)
{
    return QSize(((source_size.width() * scale_factor) / 100),
                 ((source_size.height() * scale_factor) / 100));
}

QRect scaleRect(const QRect& source_rect, int scale_factor)
{
    return QRect(((source_rect.x() * scale_factor) / 100),
                 ((source_rect.y() * scale_factor) / 100),
                 ((source_rect.width() * scale_factor) / 100),
                 ((source_rect.height() * scale_factor) / 100));
}

} // namespace

ScaleReducer::ScaleReducer(int scale_factor)
    : scale_factor_(scale_factor)
{
    // Nothing
}

// static
ScaleReducer* ScaleReducer::create(int scale_factor)
{
    if (scale_factor < kMinScaleFactor || scale_factor > kMaxScaleFactor)
        return nullptr;

    return new ScaleReducer(scale_factor);
}

const DesktopFrame* ScaleReducer::scaleFrame(const DesktopFrame* source_frame)
{
    Q_ASSERT(source_frame);
    Q_ASSERT(!source_frame->constUpdatedRegion().isEmpty());
    Q_ASSERT(source_frame->format() == PixelFormat::ARGB());

    if (scale_factor_ == kMaxScaleFactor)
        return source_frame;

    if (!scaled_frame_)
    {
        QSize scaled_size = scaleSize(source_frame->size(), scale_factor_);

        scaled_frame_ = DesktopFrameAligned::create(scaled_size, source_frame->format());
        if (!scaled_frame_)
            return nullptr;
    }

    *scaled_frame_->updatedRegion() = QRegion();

    for (const auto& source_rect : source_frame->constUpdatedRegion())
    {
        QRect scaled_rect = scaleRect(source_rect, scale_factor_);

        libyuv::ARGBScale(source_frame->frameDataAtPos(source_rect.topLeft()),
                          source_frame->stride(),
                          source_rect.width(),
                          source_rect.height(),
                          scaled_frame_->frameDataAtPos(scaled_rect.topLeft()),
                          scaled_frame_->stride(),
                          scaled_rect.width(),
                          scaled_rect.height(),
                          libyuv::kFilterBox);

        *scaled_frame_->updatedRegion() +=
            scaled_rect.intersected(QRect(QPoint(0, 0), scaled_frame_->size()));
    }

    return scaled_frame_.get();
}

} // namespace aspia
