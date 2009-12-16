/*
* This file is part of the Shiboken Python Bindings Generator project.
*
* Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
*
* Contact: PySide team <contact@pyside.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation. Please
* review the following information to ensure the GNU Lesser General
* Public License version 2.1 requirements will be met:
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*
* As a special exception to the GNU Lesser General Public License
* version 2.1, the object code form of a "work that uses the Library"
* may incorporate material from a header file that is part of the
* Library.  You may distribute such object code under terms of your
* choice, provided that the incorporated material (i) does not exceed
* more than 5% of the total size of the Library; and (ii) is limited to
* numerical parameters, data structure layouts, accessors, macros,
* inline functions and templates.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*/

#include <Python.h>
#include <QDebug>
#include "proxyslot.h"
#include "abstractqobjectconnection.h"
#include "signalmanager.h"

using namespace PySide;

ProxySlot::ProxySlot(const QObject* signalSource) : m_metaObject(signalSource), m_signalSource(signalSource)
{
    m_nextSlotIndex = m_signalSource->metaObject()->methodCount()-1;
}

bool ProxySlot::connect(AbstractQObjectConnection* connection)
{
    QObject* source = connection->source();
    attachAbstractConnection(connection);
    int slotIndex = connection->slotIndex();
    int signalIndex = connection->signalIndex();

    qDebug() << "conectando" << signalIndex << "em" << slotIndex;
    if (signalIndex >= 0) {
        return QMetaObject::connect(source, signalIndex, this, slotIndex, connection->type());
    } else { // dynamic signals!
        // TODO: Register dynamic signal
    }
    return true;

}

void ProxySlot::attachAbstractConnection(AbstractQObjectConnection* connection)
{
    m_nextSlotIndex++;
    m_connections[m_nextSlotIndex] = connection;
    connection->setSlotIndex(m_nextSlotIndex);
}

int ProxySlot::qt_metacall(QMetaObject::Call callType, int slotIndex, void** args)
{
    // 2 is the index of deleteLater slot
    if (slotIndex == 2) {
        deleteLater();
        SignalManager::instance().removeProxySlot(m_signalSource);
    }

    if (m_connections.contains(slotIndex)) {
//         thread_locker lock;
        m_connections[slotIndex]->trigger(args);
        return -1;
    }

    return QObject::qt_metacall(callType, slotIndex, args);
}

