#ifndef AC_VIDEO_PIPELINE_HPP
#define AC_VIDEO_PIPELINE_HPP

#include <cstdint>
#include <memory>

#include "ACVideoExport.hpp" // Generated by CMake

namespace ac::video
{
    struct Frame;
    struct Info;
    struct DecoderHints;
    struct EncoderHints;
    class Pipeline;
}

struct ac::video::Frame
{
    struct {
        int width, height, channel, stride;
        std::uint8_t* data;
    } plane[3];
    int planes;
    // the definition is the same as the `ElementType` of `ac::core:Image`
    int elementType;
    // one based number
    int number;
    // referencing internal data, do not modify it
    void* ref;

    // for sequential comparison
    bool operator<(const Frame& other) const noexcept { return number < other.number; }
    bool operator>(const Frame& other) const noexcept { return number > other.number; }
};

struct ac::video::Info
{
    int width;
    int height;
    int bitDepth;
    int bitDepthMask;
    double duration;
    double fps;
};

struct ac::video::DecoderHints
{
    const char* decoder = nullptr;
    const char* format = nullptr;
};

struct ac::video::EncoderHints
{
    const char* encoder = nullptr;
    int bitrate = 0;
};

class ac::video::Pipeline
{
private:
    struct PipelineData;

public:
    // do nothing
    AC_VIDEO_EXPORT Pipeline() noexcept;
    // call `close()`
    AC_VIDEO_EXPORT ~Pipeline() noexcept;

    // open the decoder, call first.
    AC_VIDEO_EXPORT bool openDecoder(const char* filename, DecoderHints hints = {}) noexcept;
    // open the encoder, call after `openDecoder`.
    AC_VIDEO_EXPORT bool openEncoder(const char* filename, double factor, EncoderHints hints = {}) noexcept;
    // close decoder and encoder, if opened. this function will complete the file writing, and can be safely called multiple times.
    AC_VIDEO_EXPORT void close() noexcept;
    // get a decoded frame, which should be released later by `release()`.
    AC_VIDEO_EXPORT bool operator>>(Frame& frame) noexcept;
    // push a frame to encode, which should be released later by `release()`.
    AC_VIDEO_EXPORT bool operator<<(const Frame& frame) noexcept;
    // request a new frame with empty data for encoding later, usually call after `>>`.
    AC_VIDEO_EXPORT bool request(Frame& dst, const Frame& src) noexcept;
    // release a frame. Multiple calls are safe.
    AC_VIDEO_EXPORT void release(Frame& frame) noexcept;
    // get decoded video info.
    AC_VIDEO_EXPORT Info getInfo() noexcept;

private:
    const std::unique_ptr<PipelineData> dptr;

};

#endif
