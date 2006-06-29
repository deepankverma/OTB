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
#ifndef __otbLineDetectorImageFilterBase_txx
#define __otbLineDetectorImageFilterBase_txx

#include "otbLineDetectorImageFilterBase.h"

#include "itkDataObject.h"
#include "itkExceptionObject.h"
#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkProgressReporter.h"
#include <math.h>

#define M_PI 3.14159265358979323846

namespace otb
{

/**
 *
 */
template <class TInputImage, class TOutputImage, class InterpolatorType >
LineDetectorImageFilterBase<TInputImage, TOutputImage, InterpolatorType>::LineDetectorImageFilterBase()
{
  m_Radius.Fill(1);
  m_LengthLine = 1;
  m_WidthLine = 0;
  m_Threshold = 0;
  m_NumberOfDirections = 4;
  m_FaceList.Fill(0);
  m_OutputDirection = OutputImageType::New();

}

template <class TInputImage, class TOutputImage, class InterpolatorType>
void LineDetectorImageFilterBase<TInputImage, TOutputImage, InterpolatorType>::GenerateInputRequestedRegion() throw (itk::InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // get pointers to the input and output
  typename InputImageType::Pointer inputPtr   =  const_cast< TInputImage * >( this->GetInput() );
  typename OutputImageType::Pointer outputPtr = this->GetOutput();
  
  if ( !inputPtr || !outputPtr )
    {
    return;
    }

  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();

  // Define the size of the region by the radius
  m_Radius[1] = static_cast<unsigned int>(3*(2*m_WidthLine+1) + 2); 
  m_Radius[0] = 2*m_LengthLine+1 ;

  std::cout << m_Radius[0] << " " << m_Radius[1] << std::endl;
  
  // Define the size of the facelist by taking into account the rotation of the region
  m_FaceList[0] = static_cast<unsigned int>( sqrt( (m_Radius[0]*m_Radius[0]) + (m_Radius[1]*m_Radius[1]) ) + 1 );
  m_FaceList[1] = m_FaceList[0];
  
  // pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_FaceList );

  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()) )
    {
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    return;
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    
    // build an exception
    itk::InvalidRequestedRegionError e(__FILE__, __LINE__);
    itk::OStringStream msg;
    msg << static_cast<const char *>(this->GetNameOfClass())
        << "::GenerateInputRequestedRegion()";
    e.SetLocation(msg.str().c_str());
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr);
    throw e;
    }
}

/*
 * Set up state of filter before multi-threading.
 * InterpolatorType::SetInputImage is not thread-safe and hence
 * has to be set up before ThreadedGenerateData
 */
template <class TInputImage, class TOutputImage, class InterpolatorType>
void 
LineDetectorImageFilterBase< TInputImage, TOutputImage, InterpolatorType>
::BeforeThreadedGenerateData()
{

  typename OutputImageType::RegionType  region;    
  typename OutputImageType::Pointer     output = this->GetOutput();


  region.SetSize(output->GetLargestPossibleRegion().GetSize());
  region.SetIndex(output->GetLargestPossibleRegion().GetIndex());
  m_OutputDirection->SetRegions( region );
  m_OutputDirection->SetOrigin(output->GetOrigin());
  m_OutputDirection->SetSpacing(output->GetSpacing());
  m_OutputDirection->Allocate();




}

template <class TInputImage, class TOutputImage, class InterpolatorType>
const typename LineDetectorImageFilterBase< TInputImage, TOutputImage, InterpolatorType>::OutputImageType *
LineDetectorImageFilterBase< TInputImage, TOutputImage, InterpolatorType>
::GetOutputDirection()
{
  this->Update();
  return 	static_cast< const OutputImageType *> (m_OutputDirection);
}

template< class TInputImage, class TOutputImage, class InterpolatorType>
void LineDetectorImageFilterBase< TInputImage, TOutputImage, InterpolatorType>
::ThreadedGenerateData(	
			const 	OutputImageRegionType& 		outputRegionForThread,
                       	int 	threadId
		       )
{
  InterpolatorPointer      interpolator = InterpolatorType::New();
  interpolator->SetInputImage(this->GetInput());
  

  itk::ZeroFluxNeumannBoundaryCondition<InputImageType> 	nbc;
  itk::ConstNeighborhoodIterator<InputImageType> 		bit;
  typename itk::ConstNeighborhoodIterator<InputImageType>::OffsetType	off;
  itk::ImageRegionIterator<OutputImageType> 			it;
  itk::ImageRegionIterator<OutputImageType> 			itdir;
  
  // Allocate output
  typename InputImageType::ConstPointer input  = this->GetInput();
  typename OutputImageType::Pointer     output = this->GetOutput();  
  typename OutputImageType::Pointer     outputDir = m_OutputDirection;
  

   
  // Find the data-set boundary "faces"
  typename itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType 		faceList;
  typename itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator 	fit;


  itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC(input, outputRegionForThread, m_FaceList);


  // support progress methods/callbacks
  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
  
  typename TInputImage::IndexType     bitIndex;
  typename InterpolatorType::ContinuousIndexType Index;


  // --------------------------------------------------------------------------
  
  // Number of direction
  const int NB_DIR = this->GetNumberOfDirections();
  // Number of zone	  
  const int NB_ZONE = 3;  
  // Definition of the 4 directions
  double Theta[NB_DIR];

  // La rotation nulle correspond a un contour horizontal -> 0 !!
  for(int i=0; i<NB_DIR; i++)
    {
    Theta[i] = (M_PI*(i/double(NB_DIR)));
/*    if(Theta[i]>M_PI)
      Theta[i] = Theta[i]-M_PI;
    if((i/double(NB_DIR))==0.5)
      Theta[i]=0.;*/
//    std::cout << Theta[i] << std::endl;
    }

  // Number of the zone 
  unsigned int zone;
  
  
  // Intensity of the linear feature
  double R;
  
  // Direction of detection
  double Direction = 0.; 

  // Pixel location in the input image
  int X, Y;
  
  // Pixel location after rotation in the system axis of the region  
  double xout, yout;
  
  // location of the central pixel in the input image
  int Xc, Yc;

  // location of the central pixel between zone 1 and 2 and between zone 1 and 3
  int Yc12, Yc13;
  
  //---------------------------------------------------------------------------
 
  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  for (fit=faceList.begin(); fit != faceList.end(); ++fit)
    { 
    bit = itk::ConstNeighborhoodIterator<InputImageType>(m_Radius, input, *fit);
    unsigned int neighborhoodSize = bit.Size();

    
    it = itk::ImageRegionIterator<OutputImageType>(output, *fit);
    itdir = itk::ImageRegionIterator<OutputImageType>(outputDir, *fit);
    
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();



    while ( ! bit.IsAtEnd() )
      {
     
      // Location of the central pixel of the region
      off.Fill(0);
      bitIndex = bit.GetIndex(off);
      
      Xc = bitIndex[0];
      Yc = bitIndex[1];


            
      // Location of the central pixel between zone 1 and zone 2
      Yc12 = Yc - m_WidthLine - 1;
      
      // Location of the central pixel between zone 1 and zone 3
      Yc13 = Yc + m_WidthLine + 1;


      // Contains for the 4 directions the the pixels belonging to each zone
      std::vector<double> PixelValues[NB_DIR][NB_ZONE];

      // Loop on the region 
      //for (i = 0; i < neighborhoodSize; ++i)
      for (int i = 0; i < m_Radius[0]; ++i)
	for (int j = 0; j < m_Radius[1]; ++j)
        {

	off[0]=i-m_Radius[0]/2;
	off[1]=j-m_Radius[1]/2;
	
        bitIndex = bit.GetIndex(off);
        X = bitIndex[0];
        Y = bitIndex[1];
	


	// We determine in the horizontal direction with which zone the pixel belongs. 
         
        if ( Y < Yc12 )
      	  zone = 1;
        else if ( ( Yc12 < Y ) && ( Y < Yc13 ) )
          zone = 0;
        else if ( Y > Yc13 )
          zone = 2;
        else
          continue;

                 
        // Loop on the directions
        for (unsigned int dir=0; dir<NB_DIR; dir++ )
          {      
	  //ROTATION( (X-Xc), (Y-Yc), Theta[dir], xout, yout);

	  xout = (X-Xc)*cos(Theta[dir]) - (Y-Yc)*sin(Theta[dir]);
	  yout = (X-Xc)*sin(Theta[dir]) + (Y-Yc)*cos(Theta[dir]);

            
	  Index[0] = static_cast<CoordRepType>(xout + Xc);
	  Index[1] = static_cast<CoordRepType>(yout + Yc);
                        
	  PixelValues[dir][zone].push_back(static_cast<double>(interpolator->EvaluateAtContinuousIndex( Index )));
          
          }      

        } // end of the loop on the pixels of the region 
          
      R = -1.;
      Direction = 0.;
           
      // Loop on the 4 directions


	for (unsigned int dir=0; dir<NB_DIR; dir++ )
	  {
	  

//	  std::cout << "Direction " << dir << std::endl;
	  double Rtemp = this->ComputeMeasure(&PixelValues[dir][0], &PixelValues[dir][1], &PixelValues[dir][2]);
	  
	  if( Rtemp > R)
	    {
	    R = Rtemp;
	    Direction = Theta[dir];
	    }
	  
	  
	  } // end of the loop on the directions

//	std::cout << R << std::endl;
	if( R >= this->GetThreshold() )
	  {
      
	  // Assignment of this value to the output pixel
	  it.Set( static_cast<OutputPixelType>(R) );
	  
	  // Assignment of this value to the "outputdir" pixel
	  itdir.Set( static_cast<OutputPixelType>(Direction) );  
	  }
	else
	  {
	  
	  it.Set( itk::NumericTraits<OutputPixelType>::Zero );
	  
	  itdir.Set( static_cast<OutputPixelType>(0) );  
	  }
	
	++bit;
	++it;
	++itdir;
	progress.CompletedPixel();  
	
      }
    
    }
}

template <class TInputImage, class TOutput, class InterpolatorType>
double LineDetectorImageFilterBase<TInputImage, TOutput, InterpolatorType>::ComputeMeasure(std::vector<double>* m1, std::vector<double>* m2, std::vector<double>* m3)
{
  return 0;
}

/**
 * Standard "PrintSelf" method
 */
template <class TInputImage, class TOutput, class InterpolatorType>
void 
LineDetectorImageFilterBase<TInputImage, TOutput, InterpolatorType>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Length: " << m_LengthLine << std::endl;
  os << indent << "Width: " << m_WidthLine << std::endl;
  
}


} // end namespace otb


#endif
