/***************************************************************************
 *   Copyright (c) 2015 FreeCAD Developers                                 *
 *   Author: Bernd Hahnebach <bernd@bimstatik.ch>                          *
 *   Based on src/Mod/Fem/Gui/DlgSettingsFemCcxImp.cpp                     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"
#ifndef _PreComp_
# include <thread>
# include <QMessageBox>
#endif

#include "DlgSettingsFemElmerImp.h"
#include "ui_DlgSettingsFemElmer.h"


using namespace FemGui;

DlgSettingsFemElmerImp::DlgSettingsFemElmerImp(QWidget* parent)
    : PreferencePage(parent)
    , ui(new Ui_DlgSettingsFemElmerImp)
{
    ui->setupUi(this);

    // determine number of CPU cores
    processor_count = std::thread::hardware_concurrency();
    // hardware check might fail and then returns 0
    if (processor_count > 0)
        ui->sb_elmer_num_cores->setMaximum(processor_count);

    connect(ui->fc_grid_binary_path, &Gui::PrefFileChooser::fileNameChanged,
            this, &DlgSettingsFemElmerImp::onfileNameChanged);
    connect(ui->fc_elmer_binary_path, &Gui::PrefFileChooser::fileNameChanged,
        this, &DlgSettingsFemElmerImp::onfileNameChanged);
    connect(ui->fc_elmer_binary_path, &Gui::PrefFileChooser::fileNameChanged,
            this, &DlgSettingsFemElmerImp::onfileNameChangedMT);
    connect(ui->sb_elmer_num_cores, qOverload<int>(&Gui::PrefSpinBox::valueChanged),
            this, &DlgSettingsFemElmerImp::onCoresValueChanged);
}

DlgSettingsFemElmerImp::~DlgSettingsFemElmerImp()
{
    // no need to delete child widgets, Qt does it all for us
}

void DlgSettingsFemElmerImp::saveSettings()
{
    ui->cb_elmer_binary_std->onSave();
    ui->fc_elmer_binary_path->onSave();

    ui->cb_grid_binary_std->onSave();
    ui->fc_grid_binary_path->onSave();

    ui->sb_elmer_num_cores->onSave();
}

void DlgSettingsFemElmerImp::loadSettings()
{
    ui->cb_elmer_binary_std->onRestore();
    ui->fc_elmer_binary_path->onRestore();

    ui->cb_grid_binary_std->onRestore();
    ui->fc_grid_binary_path->onRestore();

    ui->sb_elmer_num_cores->onRestore();
}

/**
 * Sets the strings of the subwidgets using the current language.
 */
void DlgSettingsFemElmerImp::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    else {
        QWidget::changeEvent(e);
    }
}

void DlgSettingsFemElmerImp::onfileNameChanged(QString FileName)
{
    if (!QFileInfo::exists(FileName)) {
        QMessageBox::critical(this, tr("File does not exist"),
                              tr("The specified executable \n'%1'\n does not exist!\n"
                                 "Specify another file please.").arg(FileName));
    }
}

void DlgSettingsFemElmerImp::onfileNameChangedMT(QString FileName)
{
    // reset in case it was previously set to 1
    // (hardware check might fail and then returns 0)
    if (processor_count > 0)
        ui->sb_elmer_num_cores->setMaximum(processor_count);

    if (ui->sb_elmer_num_cores->value() == 1)
        return;

    auto strName = FileName.toStdString();
#if defined(FC_OS_WIN32)
    // name ends with "_mpi.exe"
    if (strName.empty() || strName.substr(strName.length() - 8) != "_mpi.exe") {
        QMessageBox::warning(this, tr("FEM Elmer: Not suitable for multithreading"),
            tr("Wrong Elmer setting: You use more than one CPU core.\n"
                "Therefore an executable with the suffix '_mpi.exe' is required."));
        ui->sb_elmer_num_cores->setValue(1);
        ui->sb_elmer_num_cores->setMaximum(1);
        return;
    }
#elif defined(FC_OS_LINUX) || defined(FC_OS_CYGWIN) || defined(FC_OS_MACOSX) || defined(FC_OS_BSD)
    // name ends with "_mpi"
    if (strName.empty() || strName.substr(strName.length() - 4) != "_mpi") {
        QMessageBox::warning(this, tr("FEM Elmer: Not suitable for multithreading"),
            tr("Wrong Elmer setting: You use more than one CPU core.\n"
                "Therefore an executable with the suffix '_mpi' is required."));
        ui->sb_elmer_num_cores->setValue(1);
        ui->sb_elmer_num_cores->setMaximum(1);
        return;
    }
#endif
}

void DlgSettingsFemElmerImp::onCoresValueChanged(int cores)
{
    if (cores > 1) {
        // check if the right executable is loaded
        onfileNameChangedMT(ui->fc_elmer_binary_path->fileName());
    }
}

#include "moc_DlgSettingsFemElmerImp.cpp"
