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



#include "itkMacro.h"
#include <iostream>

#include "otbImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbLineCorrelationDetectorImageFilter.h"

int otbLineCorrelationDetector(int itkNotUsed(argc), char* argv[])
{
  const char * inputFilename  = argv[1];
  const char * outputFilename = argv[2];

  // Parameters of the line to detect
  unsigned int WidthLine((unsigned int) ::atoi(argv[3]));
  unsigned int LengthLine((unsigned int) ::atoi(argv[4]));

  typedef unsigned char InputPixelType;
  typedef double        OutputPixelType;
  const unsigned int Dimension = 2;

  typedef otb::Image<InputPixelType,  Dimension> InputImageType;
  typedef otb::Image<OutputPixelType, Dimension> OutputImageType;

  typedef otb::ImageFileReader<InputImageType>  ReaderType;
  typedef otb::ImageFileWriter<OutputImageType> WriterType;

  typedef otb::LineCorrelationDetectorImageFilter<InputImageType, OutputImageType> FilterType;

  FilterType::Pointer FilterLineCorrelation = FilterType::New();

  FilterLineCorrelation->SetWidthLine(WidthLine);
  FilterLineCorrelation->SetLengthLine(LengthLine);

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName(inputFilename);
  writer->SetFileName(outputFilename);

  FilterLineCorrelation->SetInput(reader->GetOutput());
  writer->SetInput(FilterLineCorrelation->GetOutput());

  writer->Update();

  return EXIT_SUCCESS;
}
