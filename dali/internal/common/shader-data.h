#ifndef DALI_INTERNAL_SHADER_DATA_H
#define DALI_INTERNAL_SHADER_DATA_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// EXTERNAL INCLUDES
#include <string>
#include <string_view>

// INTERNAL INCLUDES
#include <dali/graphics-api/graphics-types.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/rendering/shader.h> // ShaderHints

namespace Dali
{
namespace Internal
{
class ShaderData;
using ShaderDataPtr = IntrusivePtr<ShaderData>;

namespace
{
template<typename StdStringType>
inline std::vector<char> StringToVector(const StdStringType& str)
{
  auto retval = std::vector<char>{};
  retval.insert(retval.begin(), str.begin(), str.end());
  retval.push_back('\0');
  return retval;
}

} // namespace

/**
 * ShaderData class.
 * A container for shader source code and compiled binary byte code.
 */
class ShaderData : public Dali::RefObject
{
public:
  /**
   * Constructor
   * @param[in] vertexSource   Source code for vertex program
   * @param[in] fragmentSource Source code for fragment program
   * @param[in] hints          Hints for rendering
   * @param[in] renderPassTag  RenderPassTag to match shader data and render task.
   * @param[in] name           Shader name for debug.
   */
  ShaderData(std::string vertexSource, std::string fragmentSource, const Dali::Shader::Hint::Value hints, uint32_t renderPassTag, std::string_view name)
  : mShaderHash(-1),
    mVertexShader(StringToVector(vertexSource)),
    mFragmentShader(StringToVector(fragmentSource)),
    mHints(hints),
    mSourceMode(Graphics::ShaderSourceMode::TEXT),
    mRenderPassTag(renderPassTag),
    mName(name)
  {
    UpdateShaderVersion(mVertexShader, mVertexShaderVersion);
    UpdateShaderVersion(mFragmentShader, mFragmentShaderVersion);
  }

  /**
   * Constructor
   * @param[in] vertexSource   Source code for vertex program
   * @param[in] fragmentSource Source code for fragment program
   * @param[in] hints          Hints for rendering
   * @param[in] renderPassTag  RenderPassTag to match shader data and render task.
   * @param[in] name           Shader name for debug.
   */
  ShaderData(std::string_view vertexSource, std::string_view fragmentSource, const Dali::Shader::Hint::Value hints, uint32_t renderPassTag, std::string_view name)
  : mShaderHash(-1),
    mVertexShader(StringToVector(vertexSource)),
    mFragmentShader(StringToVector(fragmentSource)),
    mHints(hints),
    mSourceMode(Graphics::ShaderSourceMode::TEXT),
    mRenderPassTag(renderPassTag),
    mName(name)
  {
    UpdateShaderVersion(mVertexShader, mVertexShaderVersion);
    UpdateShaderVersion(mFragmentShader, mFragmentShaderVersion);
  }

  /**
   * Creates a shader data containing binary content
   * @param[in] vertexSource   Source code for vertex program
   * @param[in] fragmentSource Source code for fragment program
   * @param[in] hints          Hints for rendering
   * @param[in] renderPassTag  RenderPassTag to match shader data and render task.
   * @param[in] name Shader name for debug.
   */
  ShaderData(std::vector<char>& vertexSource, std::vector<char>& fragmentSource, const Dali::Shader::Hint::Value hints, uint32_t renderPassTag, std::string_view name)
  : mShaderHash(-1),
    mVertexShader(vertexSource),
    mFragmentShader(fragmentSource),
    mHints(hints),
    mSourceMode(Graphics::ShaderSourceMode::BINARY),
    mRenderPassTag(renderPassTag),
    mName(name)
  {
    UpdateShaderVersion(mVertexShader, mVertexShaderVersion);
    UpdateShaderVersion(mFragmentShader, mFragmentShaderVersion);
  }

  /**
   * Query whether a shader hint is set.
   *
   * @warning This method is called from Update Algorithms.
   *
   * @pre The shader has been initialized.
   * @param[in] hint The hint to check.
   * @return True if the given hint is set.
   */
  [[nodiscard]] bool HintEnabled(Dali::Shader::Hint::Value hint) const
  {
    return mHints & hint;
  }

protected:
  /**
   * Protected Destructor
   * A reference counted object may only be deleted by calling Unreference()
   */
  ~ShaderData() override
  {
    // vector releases its data
  }

public: // API
  /**
   * Set hash value which is created with vertex and fragment shader code
   * @param [in] shaderHash  hash key created with vertex and fragment shader code
   */
  void SetHashValue(size_t shaderHash)
  {
    DALI_ASSERT_DEBUG(shaderHash != size_t(-1));
    mShaderHash = shaderHash;
  }

  /**
   * Get hash value which is created with vertex and fragment shader code
   * @return shaderHash  hash key created with vertex and fragment shader code
   */
  size_t GetHashValue() const
  {
    DALI_ASSERT_DEBUG(mShaderHash != size_t(-1));
    return mShaderHash;
  }

  /**
   * @return the vertex shader
   */
  const char* GetVertexShader() const
  {
    return &mVertexShader[0];
  }

  /**
   * @return the vertex shader
   */
  const char* GetFragmentShader() const
  {
    return &mFragmentShader[0];
  }

  /**
   * Returns a std::vector containing the shader code associated with particular pipeline stage
   * @param[in] the graphics pipeline stage
   * @return the shader code
   */
  const std::vector<char>& GetShaderForPipelineStage(Graphics::PipelineStage stage) const
  {
    if(stage == Graphics::PipelineStage::VERTEX_SHADER)
    {
      return mVertexShader;
    }
    else if(stage == Graphics::PipelineStage::FRAGMENT_SHADER)
    {
      return mFragmentShader;
    }
    else
    {
      //      DALI_LOG_ERROR("Unsupported shader stage\n");
      static const std::vector<char> emptyShader;
      return emptyShader;
    }
  }

  /**
   * @return the hints
   */
  Dali::Shader::Hint::Value GetHints() const
  {
    return mHints;
  }
  /**
   * Check whether there is a compiled binary available
   * @return true if this objects contains a compiled binary
   */
  bool HasBinary() const
  {
    return 0 != mBuffer.Size();
  }

  /**
   * Allocate a buffer for the compiled binary bytecode
   * @param[in] size  The size of the buffer in bytes
   */
  void AllocateBuffer(std::size_t size)
  {
    mBuffer.Resize(size);
  }

  /**
   * Get the program buffer
   * @return reference to the buffer
   */
  std::size_t GetBufferSize() const
  {
    return mBuffer.Size();
  }

  /**
   * Get the data that the buffer points to
   * @return raw pointer to the buffer data
   */
  uint8_t* GetBufferData()
  {
    DALI_ASSERT_DEBUG(mBuffer.Size() > 0);
    return &mBuffer[0];
  }

  /**
   * Get the data that the buffer points to
   * @return raw pointer to the buffer data
   */
  Dali::Vector<uint8_t>& GetBuffer()
  {
    return mBuffer;
  }

  /**
   * Get the source mode of shader data
   * @return the source mode of shader data ( text or binary )
   */
  Graphics::ShaderSourceMode GetSourceMode() const
  {
    return mSourceMode;
  }

  /**
   * Get Render Pass of shader data
   * @return RenderPassTag for Render Pass of this shader data, Default value is 0.
   */
  uint32_t GetRenderPassTag() const
  {
    return mRenderPassTag;
  }

  /**
   * Get Name of shader data
   * @return Name for this shader data, Default value is empty string.
   */
  const std::string& GetName() const
  {
    return mName;
  }

  /**
   * Returns DALi specific vertex shader version
   * @return valid version number
   */
  uint32_t GetVertexShaderVersion() const
  {
    return mVertexShaderVersion;
  }

  /**
   * Returns DALi specific fragment shader version
   * @return valid version number
   */
  uint32_t GetFragmentShaderVersion() const
  {
    return mFragmentShaderVersion;
  }

private:                                        // Not implemented
  ShaderData(const ShaderData& other);          ///< no copying of this object
  ShaderData& operator=(const ShaderData& rhs); ///< no copying of this object

private:
  /**
   * Updates shader version.
   */
  void UpdateShaderVersion(std::vector<char>& code, uint32_t& outVersion)
  {
    // The version may be updated only for GLSL language.
    // If we support direct SPIRV this will be skipped
    std::string_view strView = code.data();

    // find first occurence of 'version' tag
    // the tag is expected at the start of line
    static const std::string VERSION_TAG = "//@version";

    auto pos = strView.find(VERSION_TAG);
    if(pos != std::string_view::npos && (pos == 0 || strView[pos - 1] == '\n'))
    {
      char* end;
      // Update version
      outVersion = std::strtoul(strView.data() + pos + VERSION_TAG.length(), &end, 10);
    }
    else
    {
      outVersion = 0;
    }
  }

private:                                             // Data
  std::size_t                mShaderHash;            ///< hash key created with vertex and fragment shader code
  std::vector<char>          mVertexShader;          ///< source code for vertex program
  std::vector<char>          mFragmentShader;        ///< source code for fragment program
  Dali::Shader::Hint::Value  mHints;                 ///< take a hint
  Dali::Vector<uint8_t>      mBuffer;                ///< buffer containing compiled binary bytecode
  Graphics::ShaderSourceMode mSourceMode;            ///< Source mode of shader data ( text or binary )
  uint32_t                   mRenderPassTag{0u};     ///< Render Pass Tag for this shader
  std::string                mName{""};              ///< Name for this shader
  uint32_t                   mVertexShaderVersion;   ///< Vertex shader version
  uint32_t                   mFragmentShaderVersion; ///< Fragment shader version
};

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_SHADER_DATA_H
