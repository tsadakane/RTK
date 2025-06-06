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
#ifndef rtkMotionCompensatedFourDROOSTERConeBeamReconstructionFilter_h
#define rtkMotionCompensatedFourDROOSTERConeBeamReconstructionFilter_h

#include "rtkFourDROOSTERConeBeamReconstructionFilter.h"
#include "rtkMotionCompensatedFourDConjugateGradientConeBeamReconstructionFilter.h"

namespace rtk
{
/** \class MotionCompensatedFourDROOSTERConeBeamReconstructionFilter
 * \brief Implements Motion Compensated 4D RecOnstructiOn using Spatial and TEmporal
 * Regularization (short MC-ROOSTER)
 *
 * See the reference paper: "Cardiac C-arm computed tomography using
 * a 3D + time ROI reconstruction method with spatial and temporal regularization"
 * by Mory et al.
 *
 * MC ROOSTER reconstruction consists in performing Motion-Compensated 4D Conjugate
 * Gradient reconstruction, then applying several regularization steps :
 * - Replacing all negative values by zero
 * - Averaging along time where no movement is expected
 * - Applying total variation denoising in space
 * - Applying wavelets denoising in space
 * - Applying total variation denoising in time
 * - Applying gradient's L0 norm denoising in timeWarpSequenceFilterType
 * and starting over as many times as the number of main loop iterations desired.
 *
 * \dot
 * digraph MotionCompensatedFourDROOSTERConeBeamReconstructionFilter {
 *
 * PrimaryInput [label="Primary input (4D sequence of volumes)"];
 * PrimaryInput [shape=Mdiamond];
 * InputProjectionStack [label="Input projection stack"];
 * InputProjectionStack [shape=Mdiamond];
 * InputMotionMask [label="Input motion mask"];
 * InputMotionMask [shape=Mdiamond];
 * InputDisplacementField [label="Input displacement field"];
 * InputDisplacementField [shape=Mdiamond];
 * InputInverseDisplacementField [group=invwarp, label="Input inverse displacement field"];
 * InputInverseDisplacementField [shape=Mdiamond];
 * Output [label="Output (Reconstruction: 4D sequence of volumes)"];
 * Output [shape=Mdiamond];
 *
 * node [shape=box];
 * FourDCG [ label="rtk::FourDConjugateGradientConeBeamReconstructionFilter"
 *           URL="\ref rtk::FourDConjugateGradientConeBeamReconstructionFilter"];
 * Positivity [group=regul, label="itk::ThresholdImageFilter (positivity)" URL="\ref itk::ThresholdImageFilter"];
 * Resample [group=regul, label="itk::ResampleImageFilter" URL="\ref itk::ResampleImageFilter"];
 * MotionMask [group=regul, label="rtk::AverageOutOfROIImageFilter" URL="\ref rtk::AverageOutOfROIImageFilter"];
 * TVSpace [group=regul, label="rtk::TotalVariationDenoisingBPDQImageFilter (in space)"
 *          URL="\ref rtk::TotalVariationDenoisingBPDQImageFilter"];
 * Wavelets [group=regul, label="rtk::DaubechiesWaveletsDenoiseSequenceImageFilter (in space)"
 *           URL="\ref rtk::DaubechiesWaveletsDenoiseSequenceImageFilter"];
 * TVTime [group=regul, label="rtk::TotalVariationDenoisingBPDQImageFilter (along time)"
 *         URL="\ref rtk::TotalVariationDenoisingBPDQImageFilter"];
 * L0Time [group=regul, label="rtk::LastDimensionL0GradientDenoisingImageFilter (along time)"
 *         URL="\ref rtk::LastDimensionL0GradientDenoisingImageFilter"];
 * Unwarp [group=regul, label="rtk::UnwarpSequenceImageFilter" URL="\ref rtk::UnwarpSequenceImageFilter"];
 * Subtract [group=invwarp, label="itk::SubtractImageFilter" URL="\ref itk::SubtractImageFilter"];
 * Add [group=invwarp, label="itk::AddImageFilter" URL="\ref itk::AddImageFilter"];
 *
 * AfterPrimaryInput [group=invisible, label="", fixedsize="false", width=0, height=0, shape=none];
 * AfterFourDCG [group=invisible, label="m_PerformPositivity ?", fixedsize="false", width=0, height=0, shape=none];
 * AfterPositivity [group=invisible, label="m_PerformMotionMask ?", fixedsize="false", width=0, height=0, shape=none];
 * AfterMotionMask [group=invisible, label="m_PerformTVSpatialDenoising ?",
 *                  fixedsize="false", width=0, height=0, shape=none];
 * AfterTVSpace [group=invisible, label="m_PerformWaveletsSpatialDenoising ?",
 *               fixedsize="false", width=0, height=0, shape=none];
 * AfterWavelets [group=invisible, label="m_PerformWarping ?", fixedsize="false", width=0, height=0, shape=none];
 * AfterTVTime [group=invisible, label="m_PerformL0TemporalDenoising ?",
 *              fixedsize="false", width=0, height=0, shape=none];
 * AfterL0Time [group=invisible, label="m_ComputeInverseWarpingByConjugateGradient ?",
 *              fixedsize="false", width=0, height=0, shape=none];
 * InputDisplacementField -> FourDCG;
 * InputInverseDisplacementField -> FourDCG;
 *
 * PrimaryInput -> AfterPrimaryInput [arrowhead=none];
 * AfterPrimaryInput -> FourDCG;
 * InputProjectionStack -> FourDCG;
 * FourDCG -> AfterFourDCG;
 * AfterFourDCG -> Positivity [label="true"];
 * Positivity -> AfterPositivity;
 * AfterPositivity -> MotionMask [label="true"];
 * MotionMask -> AfterMotionMask;
 * InputMotionMask -> Resample;
 * Resample -> MotionMask;
 * AfterMotionMask -> TVSpace [label="true"];
 * TVSpace -> AfterTVSpace;FourDROOSTERConeBeamReconstructionFilter
 * AfterTVSpace -> Wavelets [label="true"];
 * Wavelets -> AfterWavelets;
 * AfterWavelets -> TVTime [label="true"];
 * TVTime -> AfterTVTime [arrowhead=none];
 * AfterTVTime -> L0Time [label="true"];
 * L0Time -> AfterL0Time;
 * AfterL0Time -> Output;
 * AfterL0Time -> AfterPrimaryInput [style=dashed];
 *
 * AfterFourDCG -> AfterPositivity  [label="false"];
 * AfterPositivity -> AfterMotionMask [label="false"];
 * AfterMotionMask -> AfterTVSpace [label="false"];
 * AfterTVSpace -> AfterWavelets [label="false"];
 * AfterWavelets -> AfterTVTime [label="false"];
 * AfterTVTime -> AfterL0Time [label="false"];
 *
 * // Invisible edges between the regularization filters
 * edge[style=invis];
 * Positivity -> MotionMask;
 * MotionMask -> TVSpace;
 * TVSpace -> Wavelets;
 * Wavelets -> TVTime;
 * TVTime -> L0Time;
 *
 * }
 * \enddot
 *
 * \test rtkmotioncompensatedfourdroostertest.cxx
 *
 * \author Cyril Mory
 *
 * \ingroup RTK ReconstructionAlgorithm
 */
template <typename VolumeSeriesType, typename ProjectionStackType>
class ITK_TEMPLATE_EXPORT MotionCompensatedFourDROOSTERConeBeamReconstructionFilter
  : public rtk::FourDROOSTERConeBeamReconstructionFilter<VolumeSeriesType, ProjectionStackType>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(MotionCompensatedFourDROOSTERConeBeamReconstructionFilter);

  /** Standard class type alias. */
  using Self = MotionCompensatedFourDROOSTERConeBeamReconstructionFilter;
  using Superclass = rtk::FourDROOSTERConeBeamReconstructionFilter<VolumeSeriesType, ProjectionStackType>;
  using Pointer = itk::SmartPointer<Self>;
  using VolumeType = ProjectionStackType;
  using CovariantVectorForSpatialGradient =
    itk::CovariantVector<typename VolumeSeriesType::ValueType, VolumeSeriesType::ImageDimension - 1>;
  using CovariantVectorForTemporalGradient = itk::CovariantVector<typename VolumeSeriesType::ValueType, 1>;
  using DVFVectorType = CovariantVectorForSpatialGradient;

  using ForwardProjectionType = typename Superclass::ForwardProjectionType;
  using BackProjectionType = typename Superclass::BackProjectionType;

#ifdef RTK_USE_CUDA
  using SpatialGradientImageType = itk::CudaImage<CovariantVectorForSpatialGradient, VolumeSeriesType::ImageDimension>;
  using TemporalGradientImageType =
    itk::CudaImage<CovariantVectorForTemporalGradient, VolumeSeriesType::ImageDimension>;
  using DVFSequenceImageType = itk::CudaImage<DVFVectorType, VolumeSeriesType::ImageDimension>;
  using DVFImageType = itk::CudaImage<DVFVectorType, VolumeSeriesType::ImageDimension - 1>;
#else
  using SpatialGradientImageType = itk::Image<CovariantVectorForSpatialGradient, VolumeSeriesType::ImageDimension>;
  using TemporalGradientImageType = itk::Image<CovariantVectorForTemporalGradient, VolumeSeriesType::ImageDimension>;
  using DVFSequenceImageType = itk::Image<DVFVectorType, VolumeSeriesType::ImageDimension>;
  using DVFImageType = itk::Image<DVFVectorType, VolumeSeriesType::ImageDimension - 1>;
#endif

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkOverrideGetNameOfClassMacro(MotionCompensatedFourDROOSTERConeBeamReconstructionFilter);

  using MotionCompensatedFourDCGFilterType =
    rtk::MotionCompensatedFourDConjugateGradientConeBeamReconstructionFilter<VolumeSeriesType, ProjectionStackType>;

  /** Neither the forward nor the back projection filter can be set by the user */
  void
  SetForwardProjectionFilter(ForwardProjectionType itkNotUsed(fwtype)) override
  {
    itkExceptionMacro(<< "ForwardProjection cannot be changed");
  }
  void
  SetBackProjectionFilter(BackProjectionType itkNotUsed(bptype)) override
  {
    itkExceptionMacro(<< "BackProjection cannot be changed");
  }

  /** Set the vector containing the signal in the sub-filters */
  void
  SetSignal(const std::vector<double> signal) override;

protected:
  MotionCompensatedFourDROOSTERConeBeamReconstructionFilter();
  ~MotionCompensatedFourDROOSTERConeBeamReconstructionFilter() override = default;

  /** Does the real work. */
  void
  GenerateData() override;

  void
  GenerateOutputInformation() override;

  void
  GenerateInputRequestedRegion() override;
};
} // namespace rtk


#ifndef ITK_MANUAL_INSTANTIATION
#  include "rtkMotionCompensatedFourDROOSTERConeBeamReconstructionFilter.hxx"
#endif

#endif
