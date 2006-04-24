#ifndef _otbCommandLineArgumentParser_txx
#define _otbCommandLineArgumentParser_txx

#include "otbCommandLineArgumentParser.h"

#include "otbMacro.h"

namespace otb
{


template< typename TypeValeur >
TypeValeur
CommandLineArgumentParseResult
::GetParameter(const char *option, unsigned int number)const
{
  std::string parameter = this->GetStringParameter(option, number);
  TypeValeur lValeur;
  ::otb::StringStream flux;
  flux << parameter;
  flux >> lValeur;
  return lValeur;
}


} // end namespace otb

#endif
