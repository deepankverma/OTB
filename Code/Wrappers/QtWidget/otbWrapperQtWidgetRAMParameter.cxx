/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "otbWrapperQtWidgetRAMParameter.h"

namespace otb
{
namespace Wrapper
{

QtWidgetRAMParameter::QtWidgetRAMParameter(RAMParameter* param, QtWidgetModel* m)
: QtWidgetParameterBase(param, m),
  m_RAMParam(param)
{
}

QtWidgetRAMParameter::~QtWidgetRAMParameter()
{
}

void QtWidgetRAMParameter::DoCreateWidget()
{
  // Set up input text edit
  m_QHBoxLayout = new QHBoxLayout;
  m_QHBoxLayout->setSpacing(0);
  m_QHBoxLayout->setContentsMargins(0, 0, 0, 0);

  m_QSpinBox = new QSpinBox;
//   m_QSpinBox->setRange(m_RAMParam->GetMinimumValue(), m_RAMParam->GetMaximumValue());
  m_QSpinBox->setToolTip(m_RAMParam->GetDescription());

  connect( m_QSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetValue(int)) );
  connect( m_QSpinBox, SIGNAL(valueChanged(int)), GetModel(), SLOT(NotifyUpdate()) );
  connect( GetModel(), SIGNAL(UpdateGui()), this, SLOT(UpdateGUI() ) );

  m_QHBoxLayout->addWidget(m_QSpinBox);
  m_QHBoxLayout->addStretch();

  this->setLayout(m_QHBoxLayout);
}

void QtWidgetRAMParameter::DoUpdateGUI()
{
//   // Update the valid range if updated
//   m_QSpinBox->setRange((int)(m_RAMParam->GetMinimumValue()),
//                        (int)(m_RAMParam->GetMaximumValue()));

  bool signalsBlocked = m_QSpinBox->blockSignals( true );

  if (m_RAMParam->HasValue())
    {
    m_QSpinBox->setValue(static_cast<int>(m_RAMParam->GetValue()));
    }
  m_QSpinBox->blockSignals( signalsBlocked );

  QFont font = m_QSpinBox->font();
  if (m_RAMParam->HasUserValue())
    {
    font.setBold(true);
    }
  else
    {
    font.setBold(false);
    }
  m_QSpinBox->setFont(font);
}

void QtWidgetRAMParameter::SetValue(int value)
{
  m_RAMParam->SetValue(static_cast<unsigned int>(value));
  m_RAMParam->SetUserValue(true);
  m_RAMParam->SetAutomaticValue(false);
}

}
}
