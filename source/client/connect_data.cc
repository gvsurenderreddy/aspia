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

#include "client/connect_data.h"

#include "crypto/secure_memory.h"

namespace aspia {

ConnectData::~ConnectData()
{
    secureMemZero(&computer_name_);
    secureMemZero(&address_);
    secureMemZero(&user_name_);
    secureMemZero(&password_);
}

void ConnectData::setDesktopConfig(const proto::desktop::Config& config)
{
    desktop_config_ = config;

    if (desktop_config_.scale_factor() < 50 || desktop_config_.scale_factor() > 100)
        desktop_config_.set_scale_factor(100);
}

} // namespace aspia
