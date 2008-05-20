/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "characteristics.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "characteristic.h"
#include "storedProcErrorLookup.h"

characteristics::characteristics(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_char, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainCharacteristics"))
  {
    connect(_char, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_char, SIGNAL(valid(bool)),     _delete, SLOT(setEnabled(bool)));
    connect(_char, SIGNAL(valid(bool)),       _edit, SLOT(setEnabled(bool)));
    connect(_char, SIGNAL(valid(bool)),       _view, SLOT(setEnabled(bool)));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_char, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    connect(_char, SIGNAL(valid(bool)),       _view, SLOT(setEnabled(bool)));
  }

  _char->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _char->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "char_notes");

  sFillList();
}

characteristics::~characteristics()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristics::languageChange()
{
  retranslateUi(this);
}

void characteristics::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  characteristic newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void characteristics::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("char_id", _char->id());

  characteristic newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void characteristics::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("char_id", _char->id());

  characteristic newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void characteristics::sDelete()
{
  q.prepare("SELECT deleteCharacteristic(:char_id) AS result;");
  q.bindValue(":char_id", _char->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("result").toInt();
    if (returnVal < 0)
    {
      QMessageBox::critical( this, tr("Cannot Delete Characteristic"),
                             storedProcErrorLookup("deleteCharacteristic",
						   returnVal));
      return;
    }

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void characteristics::sPrint()
{
  orReport report("CharacteristicsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void characteristics::sFillList()
{
  q.prepare("SELECT char_id, char_name, char_notes "
            "FROM char "
            "ORDER BY char_name;");
  q.exec();
  _char->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void characteristics::sPopulateMenu(QMenu *)
{
}
