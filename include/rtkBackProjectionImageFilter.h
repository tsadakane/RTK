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

#ifndef rtkBackProjectionImageFilter_h
#define rtkBackProjectionImageFilter_h

#include "rtkConfiguration.h"

#include <itkInPlaceImageFilter.h>
#include <itkConceptChecking.h>

#include "rtkThreeDCircularProjectionGeometry.h"

#include <type_traits>
#include <typeinfo>

namespace rtk
{

/** \class BackProjectionImageFilter
 * \brief 3D backprojection
 *
 * Backprojects a stack of projection images (input 1) in a 3D volume (input 0)
 * using linear interpolation according to a specified geometry. The operation
 * is voxel-based, meaning that the center of each voxel is projected in the
 * projection images to determine the interpolation location.
 *
 * \test rtkfovtest.cxx
 *
 * \author Simon Rit
 *
 * \ingroup RTK Projector
 */
template <class TInputImage, class TOutputImage>
class ITK_TEMPLATE_EXPORT BackProjectionImageFilter : public itk::InPlaceImageFilter<TInputImage, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(BackProjectionImageFilter);

  /** Standard class type alias. */
  using Self = BackProjectionImageFilter;
  using Superclass = itk::ImageToImageFilter<TInputImage, TOutputImage>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;
  using InputPixelType = typename TInputImage::PixelType;
  using InternalInputPixelType = typename TInputImage::InternalPixelType;
  using OutputImageRegionType = typename TOutputImage::RegionType;

  using GeometryType = rtk::ThreeDCircularProjectionGeometry;
  using GeometryConstPointer = typename GeometryType::ConstPointer;
  using ProjectionMatrixType = typename GeometryType::MatrixType;
  using ProjectionImageType = itk::Image<InputPixelType, TInputImage::ImageDimension - 1>;
  using ProjectionImagePointer = typename ProjectionImageType::Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkOverrideGetNameOfClassMacro(BackProjectionImageFilter);

  /** Get / Set the object pointer to projection geometry */
  itkGetConstObjectMacro(Geometry, GeometryType);
  itkSetConstObjectMacro(Geometry, GeometryType);

  /** Get / Set the transpose flag for 2D projections (optimization trick) */
  itkGetMacro(Transpose, bool);
  itkSetMacro(Transpose, bool);

protected:
  BackProjectionImageFilter()
    : m_Geometry(nullptr)
  {
    this->SetNumberOfRequiredInputs(2);
    this->SetInPlace(true);
  };
  ~BackProjectionImageFilter() override = default;

  /** Checks that inputs are correctly set. */
  void
  VerifyPreconditions() const override;

  /** Apply changes to the input image requested region. */
  void
  GenerateInputRequestedRegion() override;

  void
  BeforeThreadedGenerateData() override;

  void
  DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread) override;

  /** Special case when the detector is cylindrical and centered on source */
  virtual void
  CylindricalDetectorCenteredOnSourceBackprojection(
    const OutputImageRegionType &                                                         region,
    const ProjectionMatrixType &                                                          volIndexToProjPP,
    const itk::Matrix<double, TInputImage::ImageDimension, TInputImage::ImageDimension> & projPPToProjIndex,
    const ProjectionImagePointer                                                          projection);

  /** Optimized version when the rotation is parallel to X, i.e. matrix[1][0]
    and matrix[2][0] are zeros. */
  virtual void
  OptimizedBackprojectionX(const OutputImageRegionType & region,
                           const ProjectionMatrixType &  matrix,
                           const ProjectionImagePointer  projection);

  /** Optimized version when the rotation is parallel to Y, i.e. matrix[1][1]
    and matrix[2][1] are zeros. */
  virtual void
  OptimizedBackprojectionY(const OutputImageRegionType & region,
                           const ProjectionMatrixType &  matrix,
                           const ProjectionImagePointer  projection);

  /** The two inputs should not be in the same space so there is nothing
   * to verify. */
  void
  VerifyInputInformation() const override
  {}

  /** The input is a stack of projections, we need to interpolate in one projection
      for efficiency during interpolation. Use of itk::ExtractImageFilter is
      not threadsafe in ThreadedGenerateData, this one is. The output can be multiplied by a constant.
      The function is templated to allow getting an itk::CudaImage. */
  template <class TProjectionImage>
  typename TProjectionImage::Pointer
  GetProjection(const unsigned int iProj);

  /** Creates iProj index to index projection matrices with current inputs
      instead of the physical point to physical point projection matrix provided by Geometry */
  ProjectionMatrixType
  GetIndexToIndexProjectionMatrix(const unsigned int iProj);

  ProjectionMatrixType
  GetVolumeIndexToProjectionPhysicalPointMatrix(const unsigned int iProj);

  itk::Matrix<double, TInputImage::ImageDimension, TInputImage::ImageDimension>
  GetProjectionPhysicalPointToProjectionIndexMatrix(const unsigned int iProj);

  /** RTK geometry object */
  GeometryConstPointer m_Geometry;

private:
  /** Flip projection flag: infludences GetProjection and
    GetIndexToIndexProjectionMatrix for optimization */
  bool m_Transpose{ false };
};

} // end namespace rtk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "rtkBackProjectionImageFilter.hxx"
#endif

#endif
