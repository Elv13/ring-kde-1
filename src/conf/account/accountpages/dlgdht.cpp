/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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
#include "dlgdht.h"

//Ring
#include <account.h>
#include <certificatemodel.h>
#include <bootstrapmodel.h>
#include <widgets/certificateviewer.h>

DlgDht::DlgDht(QWidget* parent) : QWidget(parent),m_pAccount(nullptr)
{
   setupUi(this);

//    connect(m_pKnownPeers, &QListView::doubleClicked, [this](const QModelIndex& index) {
//       CertificateViewer* v = new CertificateViewer(index,this);
//       v->show();
//       connect(v,&QDialog::finished,[v](int) { delete v; });
//    });
//    groupBox->setVisible(false);
}

void DlgDht::setAccount(Account* a)
{
   m_pAccount = a;

   if (a && a->protocol() == Account::Protocol::RING) {
//       lrcfg_username->setText(a->username());
//       m_pKnownPeers->setModel();
//       m_pKnownPeers->setModel(a->knownCertificateModel());
      m_pBootstrap->setModel(a->bootstrapModel());
      if (m_pBootstrap->horizontalHeader())
         m_pBootstrap->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
   }
}
