/****************************************************************************
 *   Copyright (C) 2012-2013 by Savoir-Faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "audiocodecmodel.h"

//Qt
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

///Constructor
AudioCodecModel::AudioCodecModel(QObject* par) : QAbstractListModel(par?par:QCoreApplication::instance()) {

}

///Model data
QVariant AudioCodecModel::data(const QModelIndex& idx, int role) const {
   if(idx.column() == 0      && role == Qt::DisplayRole                   ) {
      return QVariant(m_lAudioCodecs[idx.row()]->name);
   }
   else if(idx.column() == 0 && role == Qt::CheckStateRole                ) {
      return QVariant(m_lEnabledCodecs[m_lAudioCodecs[idx.row()]->id] ? Qt::Checked : Qt::Unchecked);
   }
   else if (idx.column() == 0 && role == AudioCodecModel::NAME_ROLE       ) {
      return m_lAudioCodecs[idx.row()]->name;
   }
   else if (idx.column() == 0 && role == AudioCodecModel::BITRATE_ROLE    ) {
      return m_lAudioCodecs[idx.row()]->bitrate;
   }
   else if (idx.column() == 0 && role == AudioCodecModel::SAMPLERATE_ROLE ) {
      return m_lAudioCodecs[idx.row()]->samplerate;
   }
   else if (idx.column() == 0 && role == AudioCodecModel::ID_ROLE         ) {
      return m_lAudioCodecs[idx.row()]->id;
   }
   return QVariant();
}

///Number of audio codecs
int AudioCodecModel::rowCount(const QModelIndex& par) const {
   Q_UNUSED(par)
   return m_lAudioCodecs.size();
}

///Model flags
Qt::ItemFlags AudioCodecModel::flags(const QModelIndex& idx) const {
   if (idx.column() == 0)
      return QAbstractItemModel::flags(idx) | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   return QAbstractItemModel::flags(idx);
}

///Set audio codec data
bool AudioCodecModel::setData( const QModelIndex& idx, const QVariant &value, int role) {
   if (idx.column() == 0 && role == AudioCodecModel::NAME_ROLE) {
      m_lAudioCodecs[idx.row()]->name = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   else if (idx.column() == 0 && role == AudioCodecModel::BITRATE_ROLE) {
      m_lAudioCodecs[idx.row()]->bitrate = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   else if(idx.column() == 0 && role == Qt::CheckStateRole) {
      m_lEnabledCodecs[m_lAudioCodecs[idx.row()]->id] = value.toBool();
      emit dataChanged(idx, idx);
      return true;
   }
   else if (idx.column() == 0 && role == AudioCodecModel::SAMPLERATE_ROLE) {
      m_lAudioCodecs[idx.row()]->samplerate = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   else if (idx.column() == 0 && role == AudioCodecModel::ID_ROLE) {
      m_lAudioCodecs[idx.row()]->id = value.toInt();
      emit dataChanged(idx, idx);
      return true;
   }
   return false;
}

///Add a new audio codec
QModelIndex AudioCodecModel::addAudioCodec() {
   m_lAudioCodecs << new AudioCodecData;
   emit dataChanged(index(m_lAudioCodecs.size()-1,0), index(m_lAudioCodecs.size()-1,0));
   return index(m_lAudioCodecs.size()-1,0);
}

///Remove audio codec at 'idx'
void AudioCodecModel::removeAudioCodec(QModelIndex idx) {
   qDebug() << "REMOVING" << idx.row() << m_lAudioCodecs.size();
   if (idx.isValid()) {
      m_lAudioCodecs.removeAt(idx.row());
      emit dataChanged(idx, index(m_lAudioCodecs.size()-1,0));
      qDebug() << "DONE" << m_lAudioCodecs.size();
   }
   else {
      qDebug() << "Failed to remove an invalid audio codec";
   }
}

///Remove everything
void AudioCodecModel::clear()
{
   foreach(AudioCodecData* data2, m_lAudioCodecs) {
      delete data2;
   }
   m_lAudioCodecs.clear  ();
   m_lEnabledCodecs.clear();
}

///Increase codec priority
bool AudioCodecModel::moveUp(QModelIndex idx)
{
   if(idx.row() > 0 && idx.row() <= rowCount()) {
      AudioCodecData* data2 = m_lAudioCodecs[idx.row()];
      m_lAudioCodecs.removeAt(idx.row());
      m_lAudioCodecs.insert(idx.row() - 1, data2);
      emit dataChanged(index(idx.row() - 1, 0, QModelIndex()), index(idx.row(), 0, QModelIndex()));
      return true;
   }
   return false;
}

///Decrease codec priority
bool AudioCodecModel::moveDown(QModelIndex idx)
{
   if(idx.row() >= 0 && idx.row() < rowCount()) {
      AudioCodecData* data2 = m_lAudioCodecs[idx.row()];
      m_lAudioCodecs.removeAt(idx.row());
      m_lAudioCodecs.insert(idx.row() + 1, data2);
      emit dataChanged(index(idx.row(), 0, QModelIndex()), index(idx.row() + 1, 0, QModelIndex()));
      return true;
   }
   return false;
}
