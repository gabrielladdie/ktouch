/*
 * Copyright 2012  Sebastian Gottfried <sebastiangottfried@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "charactersmodel.h"

#include <QSignalMapper>

#include <KLocale>

#include <core/key.h>
#include <core/keychar.h>

#include <KDebug>

CharactersModel::CharactersModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_key(0),
    m_signalMapper(new QSignalMapper(this))
{
}

Key* CharactersModel::key() const
{
    return m_key;
}

void CharactersModel::setKey(Key* key)
{
    if (key != m_key)
    {
        beginResetModel();

        if (m_key)
        {
            m_key->disconnect(this);
        }

        m_key = key;

        if (m_key)
        {
            connect(m_key, SIGNAL(keyCharAboutToBeAdded(KeyChar*,int)), SLOT(onKeyCharAboutToBeAdded(KeyChar*,int)));
            connect(m_key, SIGNAL(keyCharAdded()), SLOT(onKeyCharAdded()));
            connect(m_key, SIGNAL(keyCharsAboutToBeRemoved(int,int)), SLOT(onKeyCharsAboutToBeRemoved(int,int)));
            connect(m_key, SIGNAL(keyCharsRemoved()), SLOT(onKeyCharsRemoved()));
        }

        emit keyChanged();

        endResetModel();
    }
}

Qt::ItemFlags CharactersModel::flags(const QModelIndex& index) const
{
    return QAbstractTableModel::flags(index);
}

QVariant CharactersModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_key->keyCharCount())
        return QVariant();

    KeyChar* keyChar = m_key->keyChar(index.row());

    switch (index.column())
    {
    case 0:
        return characterData(keyChar, role);
    case 1:
        return modifierIdData(keyChar, role);
    case 2:
        return positionData(keyChar, role);
    default:
        return QVariant();
    }
}

QVariant CharactersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return QVariant(section + 1);

    switch (section)
    {
    case 0:
        return QVariant(i18n("Character"));
    case 1:
        return QVariant(i18n("Modifier ID"));
    case 2:
        return QVariant(i18n("Position"));
    default:
        return QVariant();
    }
}

int CharactersModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return 3;
}

int CharactersModel::rowCount(const QModelIndex& parent) const
{
    if (!m_key)
        return 0;

    if (parent.isValid())
        return 0;

    return m_key->keyCharCount();
}

void CharactersModel::onKeyCharAboutToBeAdded(KeyChar* keyChar, int index)
{
    connect(keyChar, SIGNAL(valueChanged()), m_signalMapper, SLOT(map()));
    connect(keyChar, SIGNAL(modifierChanged()), m_signalMapper, SLOT(map()));
    connect(keyChar, SIGNAL(positionChanged()), m_signalMapper, SLOT(map()));
    beginInsertRows(QModelIndex(), index, index);
}

void CharactersModel::onKeyCharAdded()
{
    updateMappings();
    endInsertRows();
}

void CharactersModel::onKeyCharsAboutToBeRemoved(int first, int last)
{
    beginRemoveRows(QModelIndex(), first, last);
}

void CharactersModel::onKeyCharsRemoved()
{
    endRemoveRows();
}

void CharactersModel::emitCharacterChanged(int row)
{
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}

void CharactersModel::updateMappings()
{
    for (int i = 0; i < m_key->keyCharCount(); i++)
    {
        m_signalMapper->setMapping(m_key->keyChar(i), i);
    }
}

QVariant CharactersModel::characterData(KeyChar* keyChar, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        return keyChar->value();
    default:
        return QVariant();
    }
}

QVariant CharactersModel::modifierIdData(KeyChar *keyChar, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        return keyChar->modifier();
    default:
        return QVariant();
    }
}

QVariant CharactersModel::positionData(KeyChar *keyChar, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        switch (keyChar->position())
        {
        case KeyChar::TopLeft:
            return i18n("Top left");
        case KeyChar::TopRight:
            return i18n("Top right");
        case KeyChar::BottomLeft:
            return i18n("Bottom left");
        case KeyChar::BottomRight:
            return i18n("Bottom right");
        case KeyChar::Hidden:
            return i18n("Hidden");
        }
    default:
        return QVariant();
    }
}
