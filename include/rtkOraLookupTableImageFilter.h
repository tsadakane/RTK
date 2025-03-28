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

#ifndef rtkOraLookupTableImageFilter_h
#define rtkOraLookupTableImageFilter_h

#include "rtkLookupTableImageFilter.h"
#include <itkNumericTraits.h>

namespace rtk
{

/** \class OraLookupTableImageFilter
 * \brief Lookup table for Ora data.
 *
 * The lookup table uses the slope and intercept from the meta information to
 * create a linear lookup table. The log is taken depending on the flag
 * ComputeLineIntegral.
 *
 * \test rtkoratest.cxx
 *
 * \author Simon Rit
 *
 * \ingroup RTK ImageToImageFilter
 */
template <class TOutputImage>
class ITK_TEMPLATE_EXPORT OraLookupTableImageFilter
  : public LookupTableImageFilter<itk::Image<unsigned short, TOutputImage::ImageDimension>, TOutputImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(OraLookupTableImageFilter);

  /** Standard class type alias. */
  using Self = OraLookupTableImageFilter;
  using Superclass = LookupTableImageFilter<itk::Image<unsigned short, TOutputImage::ImageDimension>, TOutputImage>;
  using Pointer = itk::SmartPointer<Self>;
  using ConstPointer = itk::SmartPointer<const Self>;

  using InputImagePixelType = unsigned short;
  using OutputImagePixelType = typename TOutputImage::PixelType;
  using LookupTableType = typename Superclass::FunctorType::LookupTableType;
  using FileNamesContainer = std::vector<std::string>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkOverrideGetNameOfClassMacro(OraLookupTableImageFilter);

  void
  BeforeThreadedGenerateData() override;

  /** Convert the projection data to line integrals after pre-processing.
  ** Default is on. */
  itkSetMacro(ComputeLineIntegral, bool);
  itkGetConstMacro(ComputeLineIntegral, bool);
  itkBooleanMacro(ComputeLineIntegral);

  /** Set the vector of strings that contains the file names. Files
   * are processed in sequential order. */
  void
  SetFileNames(const FileNamesContainer & name)
  {
    if (m_FileNames != name)
    {
      m_FileNames = name;
      this->Modified();
    }
  }
  const FileNamesContainer &
  GetFileNames() const
  {
    return m_FileNames;
  }

protected:
  OraLookupTableImageFilter() = default;
  ~OraLookupTableImageFilter() override = default;

private:
  bool m_ComputeLineIntegral{ true };

  /** A list of filenames to be processed. */
  FileNamesContainer m_FileNames;
};

} // end namespace rtk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "rtkOraLookupTableImageFilter.hxx"
#endif

#endif
