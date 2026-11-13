/**
 * SPDX-FileCopyrightText: 2026 Albert Vaca Cintora <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <core/kdeconnectplugin.h>

#define PACKET_TYPE_WAKEONLAN QStringLiteral("kdeconnect.wakeonlan")

class WakeOnLanPlugin : public KdeConnectPlugin
{
    Q_OBJECT

public:
    using KdeConnectPlugin::KdeConnectPlugin;

    void connected() override;
    void receivePacket(const NetworkPacket &np) override;

private:
    static bool isWakeOnLanEnabled(const QString &interfaceName);
};
