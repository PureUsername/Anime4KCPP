#include <memory>
#include <string>
#include <sstream>

#include "AC/Core/Dispatch.hpp"
#include "AC/Core/Processor.hpp"
#include "AC/Core/Model/ACNet.hpp"

#include "ACExport.hpp" // Generated by CMake

namespace ac::core::cpu
{
    namespace arch
    {
        enum
        {
            Begin,
            Auto = Begin,
#           ifdef AC_CORE_WITH_EIGEN3
            Eigen3,
#           endif
#           ifdef AC_CORE_WITH_SSE
            SSE,
#           endif
#           ifdef AC_CORE_WITH_AVX
            AVX,
#           endif
#           ifdef AC_CORE_WITH_NEON
            NEON,
#           endif
            Generic,
            End
        };
        constexpr const char* NameList[] = 
        {
            "Auto",
#           ifdef AC_CORE_WITH_EIGEN3
            "Eigen3",
#           endif
#           ifdef AC_CORE_WITH_SSE
            "SSE",
#           endif
#           ifdef AC_CORE_WITH_AVX
            "AVX",
#           endif
#           ifdef AC_CORE_WITH_NEON
            "NEON",
#           endif
            "Generic",
        };
    }
    void conv3x3_1to8_generic(const Image& src, Image& dst, const float* kernels, const float* biases);
    void conv3x3_8to8_generic(const Image& src, Image& dst, const float* kernels, const float* biases);
    void deconv2x2_8to1_generic(const Image& src, Image& dst, const float* kernels);
#ifdef AC_CORE_WITH_EIGEN3
    void conv3x3_1to8_eigen3(const Image& src, Image& dst, const float* kernels, const float* biases);
    void conv3x3_8to8_eigen3(const Image& src, Image& dst, const float* kernels, const float* biases);
    void deconv2x2_8to1_eigen3(const Image& src, Image& dst, const float* kernels);
#endif
#ifdef AC_CORE_WITH_SSE
    void conv3x3_1to8_sse(const Image& src, Image& dst, const float* kernels, const float* biases);
    void conv3x3_8to8_sse(const Image& src, Image& dst, const float* kernels, const float* biases);
    void deconv2x2_8to1_sse(const Image& src, Image& dst, const float* kernels);
#endif
#ifdef AC_CORE_WITH_AVX
    void conv3x3_1to8_avx(const Image& src, Image& dst, const float* kernels, const float* biases);
    void conv3x3_8to8_avx(const Image& src, Image& dst, const float* kernels, const float* biases);
    void deconv2x2_8to1_avx(const Image& src, Image& dst, const float* kernels);
#endif
#ifdef AC_CORE_WITH_NEON
    void conv3x3_1to8_neon(const Image& src, Image& dst, const float* kernels, const float* biases);
    void conv3x3_8to8_neon(const Image& src, Image& dst, const float* kernels, const float* biases);
    void deconv2x2_8to1_neon(const Image& src, Image& dst, const float* kernels);
#endif
    template<typename Model>
    class CPUProcessor;
}

template<>
class ac::core::cpu::CPUProcessor<ac::core::model::ACNet> : public ac::core::Processor
{
public:
    CPUProcessor(int arch, const model::ACNet& model) noexcept;
    ~CPUProcessor() noexcept override;

    const char* name() const noexcept override;
private:
    void process(const Image& src, Image& dst) override;
private:
    model::ACNet model;
    void (*conv3x3_1to8)(const Image& src, Image& dst, const float* kernels, const float* biases);
    void (*conv3x3_8to8)(const Image& src, Image& dst, const float* kernels, const float* biases);
    void (*deconv2x2_8to1)(const Image& src, Image& dst, const float* kernels);
};

ac::core::cpu::CPUProcessor<ac::core::model::ACNet>::CPUProcessor(const int arch, const model::ACNet& model) noexcept :
    Processor((arch == arch::Auto) ? 
    []() -> int {
        // x86
#       ifdef AC_CORE_WITH_AVX
        if (dispatch::supportAVX()) return arch::AVX;
#       endif
#       ifdef AC_CORE_WITH_SSE
        if (dispatch::supportSSE()) return arch::SSE;
#       endif
        // arm
#       ifdef AC_CORE_WITH_NEON
        if (dispatch::supportNEON()) return arch::NEON;
#       endif
        // generic
#       ifdef AC_CORE_WITH_EIGEN3
        return arch::Eigen3;
#       else
        return arch::Generic;
#       endif
    }() : arch),
    model(model)
{
    switch (idx)
    {
#   ifdef AC_CORE_WITH_EIGEN3
    case arch::Eigen3 :
        conv3x3_1to8 = conv3x3_1to8_eigen3;
        conv3x3_8to8 = conv3x3_8to8_eigen3;
        deconv2x2_8to1 = deconv2x2_8to1_eigen3;
        break;
#   endif
#   ifdef AC_CORE_WITH_SSE
    case arch::SSE :
        conv3x3_1to8 = conv3x3_1to8_sse;
        conv3x3_8to8 = conv3x3_8to8_sse;
        deconv2x2_8to1 = deconv2x2_8to1_sse;
        break;
#   endif
#   ifdef AC_CORE_WITH_AVX
    case arch::AVX :
        conv3x3_1to8 = conv3x3_1to8_avx;
        conv3x3_8to8 = conv3x3_8to8_avx;
        deconv2x2_8to1 = deconv2x2_8to1_avx;
        break;
#   endif
#   ifdef AC_CORE_WITH_NEON
    case arch::NEON :
        conv3x3_1to8 = conv3x3_1to8_neon;
        conv3x3_8to8 = conv3x3_8to8_neon;
        deconv2x2_8to1 = deconv2x2_8to1_neon;
        break;
#   endif
    default:
        conv3x3_1to8 = conv3x3_1to8_generic;
        conv3x3_8to8 = conv3x3_8to8_generic;
        deconv2x2_8to1 = deconv2x2_8to1_generic;
        break;
    }
}
ac::core::cpu::CPUProcessor<ac::core::model::ACNet>::~CPUProcessor() noexcept = default;

const char* ac::core::cpu::CPUProcessor<ac::core::model::ACNet>::name() const noexcept
{
    return arch::NameList[idx];
}
void ac::core::cpu::CPUProcessor<ac::core::model::ACNet>::process(const Image& src, Image& dst)
{
    Image tmp1{src.width(), src.height(), 8, ac::core::Image::Float32};
    Image tmp2{src.width(), src.height(), 8, ac::core::Image::Float32};
    conv3x3_1to8(src, tmp1, model.kernels(0), model.biases(0));
    conv3x3_8to8(tmp1, tmp2, model.kernels(1), model.biases(1));
    conv3x3_8to8(tmp2, tmp1, model.kernels(2), model.biases(2));
    conv3x3_8to8(tmp1, tmp2, model.kernels(3), model.biases(3));
    conv3x3_8to8(tmp2, tmp1, model.kernels(4), model.biases(4));
    conv3x3_8to8(tmp1, tmp2, model.kernels(5), model.biases(5));
    conv3x3_8to8(tmp2, tmp1, model.kernels(6), model.biases(6));
    conv3x3_8to8(tmp1, tmp2, model.kernels(7), model.biases(7));
    conv3x3_8to8(tmp2, tmp1, model.kernels(8), model.biases(8));
    deconv2x2_8to1(tmp1, dst, model.kernels(9));
}

template<>
AC_EXPORT std::shared_ptr<ac::core::Processor> ac::core::Processor::create<ac::core::Processor::CPU ,ac::core::model::ACNet>(const int arch, const model::ACNet& model)
{
    return std::make_shared<cpu::CPUProcessor<model::ACNet>>(arch, model);
}
template<>
AC_EXPORT const char* ac::core::Processor::info<ac::core::Processor::CPU>()
{
    static auto infoBuffer = []() -> std::string {
        std::ostringstream buffer{"CPU:\n", std::ios_base::ate};
        for (int i = cpu::arch::Begin; i < cpu::arch::End; i++)
        {
            buffer << "  [" << i << "] " << cpu::arch::NameList[i] << '\n';
        }
        return buffer.str();
    }();
    return infoBuffer.c_str();
}
