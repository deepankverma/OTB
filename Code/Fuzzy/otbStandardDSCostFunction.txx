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
#ifndef __otbStandardDSCostFunction_txx
#define __otbStandardDSCostFunction_txx

#include "otbStandardDSCostFunction.h"

namespace otb
{
// Constructor
template <class TDSValidationFilter>
StandardDSCostFunction<TDSValidationFilter>
::StandardDSCostFunction() :
  m_CriterionFormula("((Belief + Plausibility)/2)"),
  m_Weight(0.5),
  m_NumberOfParameters(8)
{
 m_GTVectorData = VectorDataType::New();
 m_NSVectorData = VectorDataType::New();
 m_Parser =  ParserType::New();

 m_Hypothesis.insert("NDVI");
 m_Hypothesis.insert("RADIOM");
}

template <class TDSValidationFilter>
unsigned int
StandardDSCostFunction<TDSValidationFilter>
::GetNumberOfParameters() const
 {
  return m_NumberOfParameters;
 }

template <class TDSValidationFilter>
typename StandardDSCostFunction<TDSValidationFilter>
::MeasureType
 StandardDSCostFunction<TDSValidationFilter>
::GetValue(const ParametersType & parameters) const
 {
  //Initialize parser
  m_Parser->SetExpr(m_CriterionFormula);

  unsigned int nbParam = this->GetNumberOfParameters();

  std::vector<double> ndvi, radiom;
  for (unsigned int i=0; i<4; i++)
    {
      ndvi.push_back(parameters[i]);
    }
  for (unsigned int i=0; i<4; i++)
    {
      radiom.push_back(parameters[i+4]);
    }
  typename DSValidationFilterType::Pointer internalFunctionGT
    = DSValidationFilterType::New();
  internalFunctionGT->SetCriterionFormula("1");
  internalFunctionGT->SetInput(m_GTVectorData);
  internalFunctionGT->SetHypothesis(m_Hypothesis);
  try
  {
    internalFunctionGT->SetFuzzyModel("NDVI", ndvi);
    internalFunctionGT->SetFuzzyModel("RADIOM", radiom);
  }
  catch (itk::ExceptionObject & err)
  {
    return (m_Weight*m_GTVectorData->Size() + (1-m_Weight)*m_NSVectorData->Size()); ;
  }
  internalFunctionGT->Update();

  typename DSValidationFilterType::Pointer internalFunctionNS
    = DSValidationFilterType::New();
  internalFunctionNS->SetCriterionFormula("1");
  internalFunctionNS->SetInput(m_NSVectorData);
  internalFunctionNS->SetHypothesis(m_Hypothesis);
  try
  {
    internalFunctionNS->SetFuzzyModel("NDVI", ndvi);
    internalFunctionNS->SetFuzzyModel("RADIOM", radiom);
  }
  catch (itk::ExceptionObject & err)
  {
    return (m_Weight*m_GTVectorData->Size() + (1-m_Weight)*m_NSVectorData->Size()); ;
  }
  internalFunctionNS->Update();

  double accGT, accNS, belief, plausibility;
  accGT = 0.0;
  accNS = 0.0;

  TreeIteratorType itVectorGT(internalFunctionGT->GetOutput()->GetDataTree());
  itVectorGT.GoToBegin();
  while (!itVectorGT.IsAtEnd())
      {
      if (!itVectorGT.Get()->IsRoot() && !itVectorGT.Get()->IsDocument() && !itVectorGT.Get()->IsFolder())
        {
        belief       = itVectorGT.Get()->GetFieldAsDouble("Belief");
        plausibility = itVectorGT.Get()->GetFieldAsDouble("Plausi");

        m_Parser->DefineVar("Belief", &belief);
        m_Parser->DefineVar("Plausibility", &plausibility);

        accGT += (1 - m_Parser->Eval()) * (1 - m_Parser->Eval());

        m_Parser->ClearVar();
        }
      itVectorGT++;
      }

  TreeIteratorType itVectorNS(internalFunctionNS->GetOutput()->GetDataTree());
  itVectorNS.GoToBegin();
  while (!itVectorNS.IsAtEnd())
      {
      if (!itVectorNS.Get()->IsRoot() && !itVectorNS.Get()->IsDocument() && !itVectorNS.Get()->IsFolder())
        {
        belief       = itVectorNS.Get()->GetFieldAsDouble("Belief");
        plausibility = itVectorNS.Get()->GetFieldAsDouble("Plausi");

        m_Parser->DefineVar("Belief", &belief);
        m_Parser->DefineVar("Plausibility", &plausibility);

        accNS += m_Parser->Eval() * m_Parser->Eval();

        m_Parser->ClearVar();
        }
      itVectorNS++;
      }
  return (m_Weight*accGT + (1-m_Weight)*accNS);
 }

template <class TDSValidationFilter>
void
StandardDSCostFunction<TDSValidationFilter>
::GetDerivative(const ParametersType & parameters, DerivativeType & derivative) const
 {
  //Not necessary for Amoeba Optimizer
  itkExceptionMacro(<< "Not Supposed to be used when using Amoeba Optimizer!")
 }

// PrintSelf Method
template <class TDSValidationFilter>
void
StandardDSCostFunction<TDSValidationFilter>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}


}// end namespace otb

#endif