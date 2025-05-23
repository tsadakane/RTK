/*=========================================================================
 *
 *  Copyright RTK Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef rtkAmsterdamShroudImageFilter_h
#define rtkAmsterdamShroudImageFilter_h

#include <itkImageToImageFilter.h>
#include <itkRecursiveGaussianImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkSumProjectionImageFilter.h>
#include <itkConvolutionImageFilter.h>
#include <itkSubtractImageFilter.h>
#include <itkPermuteAxesImageFilter.h>
#include <itkCropImageFilter.h>
#include "rtkThreeDCircularProjectionGeometry.h"

namespace rtk
{

/** \class AmsterdamShroudImageFilter
 * \brief Compute the Amsterdam shroud image for respiratory signal extraction.
 *
 * The Amsterdam shroud is an image that is used to extract a respiratory
 * signal from cone-beam projection images. The Y-axis is time and the X-axis
 * is the cranio-caudal position. More information is available in
 * [Zijp, ICCR, 2004], [Sonke, Med Phys, 2005] and [Rit, IJROBP, 2012].
 *
 * The following mini-pipeline of ITK filters is used for its RTK implementation:
 *
 * \dot
 * digraph AmsterdamShroud {
 *
 * Input [label="Input (Projections)", shape=Mdiamond];
 * Output [label="Output (Amsterdam Shroud)", shape=Mdiamond];
 *
 * node [shape=box];
 *
 * Derivative [label="itk::RecursiveGaussianImageFilter" URL="\ref itk::RecursiveGaussianImageFilter"];
 * Negative [label="itk::MultiplyImageFilter" URL="\ref itk::MultiplyImageFilter"];
 * Threshold [label="itk::ThresholdImageFilter" URL="\ref itk::ThresholdImageFilter"];
 * Sum [label="itk::SumProjectionImageFilter" URL="\ref itk::SumProjectionImageFilter"];
 * Convolution [label="itk::ConvolutionImageFilter" URL="\ref itk::ConvolutionImageFilter"];
 * Subtract [label="itk::SubtractImageFilter" URL="\ref itk::SubtractImageFilter"];
 * Permute [label="itk::PermuteAxesImageFilter" URL="\ref itk::PermuteAxesImageFilter"];
 *
 * Input->Derivative
 * Derivative->Negative
 * Negative->Threshold
 * Threshold->Sum
 * Sum->Subtract
 * Sum->Convolution
 * Convolution->Subtract
 * Subtract->Permute
 * Permute->Output
 * }
 * \enddot
 *
 * \test rtkamsterdamshroudtest.cxx
 *
 * \author Simon Rit
 *
 * \ingroup RTK ImageToImageFilter
 */
template <class TInputImage>
class ITK_TEMPLATE_EXPORT AmsterdamShroudImageFilter
  : public itk::ImageToImageFilter<TInputImage, itk::Image<double, TInputImage::ImageDimension - 1>>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(AmsterdamShroudImageFilter);

  /** Standard class type alias. */
  using Self = AmsterdamShroudImageFilter;
  using Superclass = itk::ImageToImageFilter<TInputImage, itk::Image<double, TInputImage::ImageDimension - 1>>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  /** Convenient type alias. */
  using TOutputImage = itk::Image<double, TInputImage::ImageDimension - 1>;
  using PointType = itk::Point<double, 3>;
  using GeometryType = rtk::ThreeDCircularProjectionGeometry;
  using GeometryPointer = typename GeometryType::Pointer;

  /** ImageDimension constants */
  static constexpr unsigned int InputImageDimension = TInputImage::ImageDimension;
  static constexpr unsigned int OutputImageDimension = TOutputImage::ImageDimension;
  static constexpr unsigned int ImageDimension = TOutputImage::ImageDimension;

  /** Standard New method. */
  itkNewMacro(Self);

  /** Size parameter of the unsharp mask. This is the number of pixels along the
   * time direction (X) along which it averages. The unsharp mask allows after
   * computation of the shroud to enhance fast varying motions, e.g., breathing,
   * and remove slow varying motions, e.g., rotation around the table. The default
   * value is 17 pixels. */
  itkGetMacro(UnsharpMaskSize, unsigned int);
  itkSetMacro(UnsharpMaskSize, unsigned int);

  /** Get / Set the object pointer to projection geometry */
  itkGetModifiableObjectMacro(Geometry, GeometryType);
  itkSetObjectMacro(Geometry, GeometryType);

  /** 3D clipbox corners for selecting part of the projections. Each corner is
   * projected and rounded to the nearest 2D pixel and only those pixels within
   * the two pixels are kept. */
  itkGetMacro(Corner1, PointType);
  itkSetMacro(Corner1, PointType);
  itkGetMacro(Corner2, PointType);
  itkSetMacro(Corner2, PointType);

  /** Runtime information support. */
  itkOverrideGetNameOfClassMacro(AmsterdamShroudImageFilter);

protected:
  AmsterdamShroudImageFilter();
  ~AmsterdamShroudImageFilter() override = default;

  void
  GenerateOutputInformation() override;
  void
  GenerateInputRequestedRegion() override;
  void
  UpdateUnsharpMaskKernel();

  /** Single-threaded version of GenerateData.  This filter delegates
   * to other filters. */
  void
  GenerateData() override;

  /** Function that actually projects the 3D box defined by m_Corner1 and
   * m_Corner2 and set everything outside to 0. */
  virtual void
  CropOutsideProjectedBox();

private:
  using DerivativeType = itk::RecursiveGaussianImageFilter<TInputImage, TInputImage>;
  using NegativeType = itk::MultiplyImageFilter<TInputImage, TInputImage, TInputImage>;
  using ThresholdType = itk::ThresholdImageFilter<TInputImage>;
  using SumType = itk::SumProjectionImageFilter<TInputImage, TOutputImage>;
  using ConvolutionType = itk::ConvolutionImageFilter<TOutputImage, TOutputImage>;
  using SubtractType = itk::SubtractImageFilter<TOutputImage, TOutputImage>;
  using PermuteType = itk::PermuteAxesImageFilter<TOutputImage>;

  typename DerivativeType::Pointer  m_DerivativeFilter;
  typename NegativeType::Pointer    m_NegativeFilter;
  typename ThresholdType::Pointer   m_ThresholdFilter;
  typename SumType::Pointer         m_SumFilter;
  typename ConvolutionType::Pointer m_ConvolutionFilter;
  typename SubtractType::Pointer    m_SubtractFilter;
  typename PermuteType::Pointer     m_PermuteFilter;
  unsigned int                      m_UnsharpMaskSize{ 17 };
  GeometryPointer                   m_Geometry{ nullptr };
  PointType                         m_Corner1{ 0. };
  PointType                         m_Corner2{ 0. };
}; // end of class

} // end namespace rtk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "rtkAmsterdamShroudImageFilter.hxx"
#endif

#endif
