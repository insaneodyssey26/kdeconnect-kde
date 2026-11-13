/**
 * SPDX-FileCopyrightText: 2026 Albert Vaca Cintora <albertvaka@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "wakeonlanplugin.h"

#include <KPluginFactory>
#include <QFile>
#include <QNetworkInterface>
#include <QTextStream>

#ifdef Q_OS_LINUX
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef WAKE_MAGIC
#define WAKE_MAGIC 0x20
#endif
#endif

K_PLUGIN_CLASS_WITH_JSON(WakeOnLanPlugin, "kdeconnect_wakeonlan.json")

bool WakeOnLanPlugin::isWakeOnLanEnabled(const QString &interfaceName)
{
#ifdef Q_OS_LINUX
    // 1. Try sysfs device wakeup state (requires no privileges)
    QFile wakeupFile(QStringLiteral("/sys/class/net/%1/device/power/wakeup").arg(interfaceName));
    if (wakeupFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&wakeupFile);
        QString status = in.readLine().trimmed();
        if (status == QLatin1String("enabled")) {
            return true;
        }
    }

    // 2. Try ethtool ioctl query (ETHTOOL_GWOL)
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd >= 0) {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        QByteArray ifnameBytes = interfaceName.toLatin1();
        strncpy(ifr.ifr_name, ifnameBytes.constData(), IFNAMSIZ - 1);

        struct ethtool_wolinfo wol;
        memset(&wol, 0, sizeof(wol));
        wol.cmd = ETHTOOL_GWOL;
        ifr.ifr_data = reinterpret_cast<char *>(&wol);

        if (::ioctl(fd, SIOCETHTOOL, &ifr) >= 0) {
            ::close(fd);
            return (wol.wolopts & WAKE_MAGIC) != 0;
        }
        ::close(fd);
    }
#endif
    return false;
}

void WakeOnLanPlugin::connected()
{
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!(iface.flags() & QNetworkInterface::IsUp) || (iface.flags() & QNetworkInterface::IsLoopBack)) {
            continue;
        }

        if (isWakeOnLanEnabled(iface.name())) {
            const QString mac = iface.hardwareAddress();
            if (!mac.isEmpty()) {
                NetworkPacket np(PACKET_TYPE_WAKEONLAN);
                np.set(QStringLiteral("macAddress"), mac);
                sendPacket(np);
                break;
            }
        }
    }
}

void WakeOnLanPlugin::receivePacket(const NetworkPacket &np)
{
    Q_UNUSED(np);
}

#include "moc_wakeonlanplugin.cpp"
#include "wakeonlanplugin.moc"
