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

#ifndef __otbTreeNeighborhood_h
#define __otbTreeNeighborhood_h

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "otbShape.h"

namespace otb
{

/** \class TreeNeighborhood
 *  \brief Classe de base pour tous les objets de type "TreeNeighborhood".
 *
 * ....
 *
 * \ingroup TreeNeighborhoodObjects
 */

class ITK_EXPORT Neighbor
{
public:
  Point_plane point;    /* Neighbor pixel */
  float       value;    /* Its gray level */
  Neighbor() {};
  ~Neighbor() {};

protected:

private:

};

class ITK_EXPORT Neighborhood
{
public:
  typedef enum  {AMBIGUOUS, MIN, MAX, INVALID} TypeOfTree;

  Neighbor   *tabPoints; /* The array of neighbors, organized as a binary tree */
  int        iNbPoints;  /* The size of the previous arrays */
  TypeOfTree type;       /* max- or min- oriented heap? */
  float      otherBound; /* Min gray level if max-oriented, max if min-oriented */

  void reinit_neighborhood(TypeOfTree type);
  void init_neighborhood(int iMaxArea,int iWidth,int iHeight);
  void free_neighborhood();

  void print_neighborhood();

  int ORDER_MAX(int k,int l)  { return (tabPoints[k].value > tabPoints[l].value); };
  int ORDER_MIN(int k,int l)  { return (tabPoints[k].value < tabPoints[l].value); };
  int ORDER_MAX2(int k,int l) { return (tabPoints[k].value >= tabPoints[l].value); };
  int ORDER_MIN2(int k,int l) { return (tabPoints[k].value <= tabPoints[l].value); };
  void SWAP(int k,int l){ tabPoints[0] = tabPoints[k];
                          tabPoints[k] = tabPoints[l];
        tabPoints[l] = tabPoints[0]; };

  void fix_up();
  void fix_down();

  Neighborhood() {};
  ~Neighborhood() {};

protected:

private:
};


class ITK_EXPORT Connection
{
public:
  Shape *shape; /* Largest shape of the monotone section */
  float level;  /* Connection level */
  Connection() {};
  ~Connection() {};

protected:

private:

};


} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbTreeNeighborhood.txx"
#endif

#endif

