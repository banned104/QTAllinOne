# FFmpeg 入门与视频播放器开发指南

> 本文档详细介绍 FFmpeg 的基础知识、核心 API 使用、以及如何实现一个完整的视频播放器（包括播放、暂停、快进、快退等功能）。

## 目录

1. [FFmpeg 简介](#1-ffmpeg-简介)
2. [环境搭建](#2-环境搭建)
3. [FFmpeg 核心概念](#3-ffmpeg-核心概念)
4. [基础 API 使用](#4-基础-api-使用)
5. [视频解码](#5-视频解码)
6. [音频解码](#6-音频解码)
7. [视频播放器实现](#7-视频播放器实现)
8. [播放控制](#8-播放控制)
9. [音视频同步](#9-音视频同步)
10. [完整播放器示例](#10-完整播放器示例)

---

## 1. FFmpeg 简介

### 1.1 什么是 FFmpeg？

FFmpeg 是一个开源的多媒体框架，能够录制、转换和流式传输音频和视频。

**核心组件**:
- **libavformat**: 封装格式处理（MP4、AVI、MKV 等）
- **libavcodec**: 编解码器（H.264、H.265、AAC 等）
- **libavutil**: 工具库（内存管理、数学函数等）
- **libswscale**: 图像缩放和格式转换
- **libswresample**: 音频重采样
- **libavfilter**: 音视频滤镜

### 1.2 FFmpeg 架构

```
输入文件 (MP4/AVI/MKV)
    ↓
AVFormatContext (解封装)
    ↓
AVPacket (压缩数据包)
    ↓
AVCodecContext (解码器)
    ↓
AVFrame (原始音视频帧)
    ↓
显示/播放
```

---

## 2. 环境搭建

### 2.1 Windows 环境

```bash
# 下载预编译库
# https://www.gyan.dev/ffmpeg/builds/

# 或使用 vcpkg
vcpkg install ffmpeg:x64-windows
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(FFmpegPlayer)

set(CMAKE_CXX_STANDARD 17)

# 查找 FFmpeg
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED 
    libavformat 
    libavcodec 
    libavutil 
    libswscale 
    libswresample
)

include_directories(${FFMPEG_INCLUDE_DIRS})
link_directories(${FFMPEG_LIBRARY_DIRS})

add_executable(player main.cpp)
target_link_libraries(player ${FFMPEG_LIBRARIES})
```

### 2.2 Linux 环境

```bash
# Ubuntu/Debian
sudo apt-get install libavformat-dev libavcodec-dev libavutil-dev \
                     libswscale-dev libswresample-dev

# Fedora/CentOS
sudo dnf install ffmpeg-devel
```

### 2.3 macOS 环境

```bash
# 使用 Homebrew
brew install ffmpeg
```

### 2.4 头文件包含

```cpp
// FFmpeg C 头文件
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include <iostream>
#include <string>
```

---

## 3. FFmpeg 核心概念

### 3.1 核心数据结构

```cpp
// ========== AVFormatContext ==========
// 封装格式上下文，包含文件信息和流信息
AVFormatContext *formatCtx = nullptr;

// ========== AVStream ==========
// 流信息（视频流、音频流、字幕流等）
AVStream *videoStream = nullptr;
AVStream *audioStream = nullptr;

// ========== AVCodecContext ==========
// 编解码器上下文
AVCodecContext *videoCodecCtx = nullptr;
AVCodecContext *audioCodecCtx = nullptr;

// ========== AVCodec ==========
// 编解码器
const AVCodec *videoCodec = nullptr;
const AVCodec *audioCodec = nullptr;

// ========== AVPacket ==========
// 压缩的数据包
AVPacket *packet = av_packet_alloc();

// ========== AVFrame ==========
// 解码后的原始帧
AVFrame *frame = av_frame_alloc();

// ========== SwsContext ==========
// 图像转换上下文
SwsContext *swsCtx = nullptr;

// ========== SwrContext ==========
// 音频重采样上下文
SwrContext *swrCtx = nullptr;
```

### 3.2 基本流程

```cpp
// 1. 打开输入文件
avformat_open_input(&formatCtx, filename, nullptr, nullptr);

// 2. 读取流信息
avformat_find_stream_info(formatCtx, nullptr);

// 3. 查找视频/音频流
av_find_best_stream(formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

// 4. 查找解码器
avcodec_find_decoder(codecId);

// 5. 打开解码器
avcodec_open2(codecCtx, codec, nullptr);

// 6. 读取数据包
av_read_frame(formatCtx, packet);

// 7. 发送数据包到解码器
avcodec_send_packet(codecCtx, packet);

// 8. 接收解码后的帧
avcodec_receive_frame(codecCtx, frame);

// 9. 处理帧数据（显示/播放）

// 10. 清理资源
```

---

## 4. 基础 API 使用

### 4.1 打开媒体文件

```cpp
#include <iostream>
extern "C" {
#include <libavformat/avformat.h>
}

class MediaFileOpener
{
public:
    static bool openMediaFile(const char *filename)
    {
        AVFormatContext *formatCtx = nullptr;
        
        // ========== 打开输入文件 ==========
        int ret = avformat_open_input(&formatCtx, filename, nullptr, nullptr);
        if (ret < 0) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, errbuf, sizeof(errbuf));
            std::cerr << "无法打开文件: " << errbuf << std::endl;
            return false;
        }
        
        // ========== 读取流信息 ==========
        ret = avformat_find_stream_info(formatCtx, nullptr);
        if (ret < 0) {
            std::cerr << "无法获取流信息" << std::endl;
            avformat_close_input(&formatCtx);
            return false;
        }
        
        // ========== 打印文件信息 ==========
        std::cout << "文件名: " << filename << std::endl;
        std::cout << "格式: " << formatCtx->iformat->long_name << std::endl;
        std::cout << "时长: " << formatCtx->duration / AV_TIME_BASE << " 秒" << std::endl;
        std::cout << "比特率: " << formatCtx->bit_rate / 1000 << " kb/s" << std::endl;
        std::cout << "流数量: " << formatCtx->nb_streams << std::endl;
        
        // ========== 遍历所有流 ==========
        for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
            AVStream *stream = formatCtx->streams[i];
            AVCodecParameters *codecpar = stream->codecpar;
            
            std::cout << "\n流 #" << i << ":" << std::endl;
            std::cout << "  类型: ";
            
            switch (codecpar->codec_type) {
                case AVMEDIA_TYPE_VIDEO:
                    std::cout << "视频" << std::endl;
                    std::cout << "  分辨率: " << codecpar->width << "x" << codecpar->height << std::endl;
                    std::cout << "  帧率: " << av_q2d(stream->avg_frame_rate) << " fps" << std::endl;
                    break;
                    
                case AVMEDIA_TYPE_AUDIO:
                    std::cout << "音频" << std::endl;
                    std::cout << "  采样率: " << codecpar->sample_rate << " Hz" << std::endl;
                    std::cout << "  声道数: " << codecpar->ch_layout.nb_channels << std::endl;
                    break;
                    
                case AVMEDIA_TYPE_SUBTITLE:
                    std::cout << "字幕" << std::endl;
                    break;
                    
                default:
                    std::cout << "其他" << std::endl;
                    break;
            }
            
            // 获取编解码器名称
            const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
            if (codec) {
                std::cout << "  编解码器: " << codec->long_name << std::endl;
            }
        }
        
        // ========== 清理 ==========
        avformat_close_input(&formatCtx);
        
        return true;
    }
};

// 使用示例
int main()
{
    MediaFileOpener::openMediaFile("video.mp4");
    return 0;
}
```

### 4.2 查找流

```cpp
class StreamFinder
{
public:
    static int findBestStream(AVFormatContext *formatCtx, 
                             AVMediaType type,
                             int &streamIndex)
    {
        // ========== 查找最佳流 ==========
        streamIndex = av_find_best_stream(
            formatCtx,
            type,           // 流类型（视频/音频）
            -1,             // 期望的流索引（-1 表示自动选择）
            -1,             // 相关流索引（用于音视频同步）
            nullptr,        // 返回的解码器
            0               // 标志
        );
        
        if (streamIndex < 0) {
            std::cerr << "未找到流" << std::endl;
            return -1;
        }
        
        return 0;
    }
    
    // ========== 手动查找流 ==========
    static int findStreamManually(AVFormatContext *formatCtx,
                                  AVMediaType type,
                                  int &streamIndex)
    {
        streamIndex = -1;
        
        for (unsigned int i = 0; i < formatCtx->nb_streams; i++) {
            if (formatCtx->streams[i]->codecpar->codec_type == type) {
                streamIndex = i;
                break;
            }
        }
        
        if (streamIndex == -1) {
            std::cerr << "未找到指定类型的流" << std::endl;
            return -1;
        }
        
        return 0;
    }
};
```

### 4.3 打开解码器

```cpp
class DecoderOpener
{
public:
    static AVCodecContext* openDecoder(AVFormatContext *formatCtx, 
                                       int streamIndex)
    {
        if (streamIndex < 0 || streamIndex >= (int)formatCtx->nb_streams) {
            std::cerr << "无效的流索引" << std::endl;
            return nullptr;
        }
        
        AVStream *stream = formatCtx->streams[streamIndex];
        AVCodecParameters *codecpar = stream->codecpar;
        
        // ========== 查找解码器 ==========
        const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
        if (!codec) {
            std::cerr << "未找到解码器" << std::endl;
            return nullptr;
        }
        
        std::cout << "使用解码器: " << codec->long_name << std::endl;
        
        // ========== 分配解码器上下文 ==========
        AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
        if (!codecCtx) {
            std::cerr << "无法分配解码器上下文" << std::endl;
            return nullptr;
        }
        
        // ========== 复制编解码器参数 ==========
        int ret = avcodec_parameters_to_context(codecCtx, codecpar);
        if (ret < 0) {
            std::cerr << "无法复制编解码器参数" << std::endl;
            avcodec_free_context(&codecCtx);
            return nullptr;
        }
        
        // ========== 打开解码器 ==========
        ret = avcodec_open2(codecCtx, codec, nullptr);
        if (ret < 0) {
            std::cerr << "无法打开解码器" << std::endl;
            avcodec_free_context(&codecCtx);
            return nullptr;
        }
        
        return codecCtx;
    }
};
```

---

## 5. 视频解码

### 5.1 基础视频解码

```cpp
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

class VideoDecoder
{
public:
    bool open(const char *filename)
    {
        // ========== 打开文件 ==========
        int ret = avformat_open_input(&m_formatCtx, filename, nullptr, nullptr);
        if (ret < 0) {
            std::cerr << "无法打开文件" << std::endl;
            return false;
        }
        
        // ========== 读取流信息 ==========
        ret = avformat_find_stream_info(m_formatCtx, nullptr);
        if (ret < 0) {
            std::cerr << "无法获取流信息" << std::endl;
            return false;
        }
        
        // ========== 查找视频流 ==========
        m_videoStreamIndex = av_find_best_stream(
            m_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        
        if (m_videoStreamIndex < 0) {
            std::cerr << "未找到视频流" << std::endl;
            return false;
        }
        
        // ========== 打开解码器 ==========
        AVStream *videoStream = m_formatCtx->streams[m_videoStreamIndex];
        const AVCodec *codec = avcodec_find_decoder(videoStream->codecpar->codec_id);
        
        if (!codec) {
            std::cerr << "未找到解码器" << std::endl;
            return false;
        }
        
        m_codecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(m_codecCtx, videoStream->codecpar);
        
        ret = avcodec_open2(m_codecCtx, codec, nullptr);
        if (ret < 0) {
            std::cerr << "无法打开解码器" << std::endl;
            return false;
        }
        
        // ========== 分配帧 ==========
        m_frame = av_frame_alloc();
        m_packet = av_packet_alloc();
        
        std::cout << "视频信息:" << std::endl;
        std::cout << "  分辨率: " << m_codecCtx->width << "x" << m_codecCtx->height << std::endl;
        std::cout << "  像素格式: " << av_get_pix_fmt_name(m_codecCtx->pix_fmt) << std::endl;
        
        return true;
    }
    
    bool decodeFrame()
    {
        while (true) {
            // ========== 读取数据包 ==========
            int ret = av_read_frame(m_formatCtx, m_packet);
            
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    std::cout << "文件结束" << std::endl;
                    return false;
                }
                std::cerr << "读取帧失败" << std::endl;
                return false;
            }
            
            // ========== 只处理视频流 ==========
            if (m_packet->stream_index != m_videoStreamIndex) {
                av_packet_unref(m_packet);
                continue;
            }
            
            // ========== 发送数据包到解码器 ==========
            ret = avcodec_send_packet(m_codecCtx, m_packet);
            av_packet_unref(m_packet);
            
            if (ret < 0) {
                std::cerr << "发送数据包失败" << std::endl;
                return false;
            }
            
            // ========== 接收解码后的帧 ==========
            ret = avcodec_receive_frame(m_codecCtx, m_frame);
            
            if (ret == AVERROR(EAGAIN)) {
                // 需要更多数据
                continue;
            } else if (ret < 0) {
                std::cerr << "接收帧失败" << std::endl;
                return false;
            }
            
            // 成功解码一帧
            std::cout << "解码帧 #" << m_codecCtx->frame_num 
                     << " (类型=" << av_get_picture_type_char(m_frame->pict_type) 
                     << ", size=" << m_frame->pkt_size << " bytes)" << std::endl;
            
            return true;
        }
    }
    
    void close()
    {
        if (m_frame) {
            av_frame_free(&m_frame);
        }
        if (m_packet) {
            av_packet_free(&m_packet);
        }
        if (m_codecCtx) {
            avcodec_free_context(&m_codecCtx);
        }
        if (m_formatCtx) {
            avformat_close_input(&m_formatCtx);
        }
    }
    
    AVFrame* getFrame() { return m_frame; }
    AVCodecContext* getCodecContext() { return m_codecCtx; }
    
private:
    AVFormatContext *m_formatCtx = nullptr;
    AVCodecContext *m_codecCtx = nullptr;
    AVFrame *m_frame = nullptr;
    AVPacket *m_packet = nullptr;
    int m_videoStreamIndex = -1;
};
```

### 5.2 图像格式转换

```cpp
class ImageConverter
{
public:
    bool init(int srcWidth, int srcHeight, AVPixelFormat srcFormat,
             int dstWidth, int dstHeight, AVPixelFormat dstFormat)
    {
        // ========== 创建转换上下文 ==========
        m_swsCtx = sws_getContext(
            srcWidth, srcHeight, srcFormat,      // 源
            dstWidth, dstHeight, dstFormat,      // 目标
            SWS_BILINEAR,                        // 缩放算法
            nullptr, nullptr, nullptr
        );
        
        if (!m_swsCtx) {
            std::cerr << "无法创建转换上下文" << std::endl;
            return false;
        }
        
        // ========== 分配目标帧 ==========
        m_dstFrame = av_frame_alloc();
        m_dstFrame->format = dstFormat;
        m_dstFrame->width = dstWidth;
        m_dstFrame->height = dstHeight;
        
        int ret = av_frame_get_buffer(m_dstFrame, 0);
        if (ret < 0) {
            std::cerr << "无法分配帧缓冲" << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool convert(AVFrame *srcFrame)
    {
        // ========== 执行转换 ==========
        int ret = sws_scale(
            m_swsCtx,
            srcFrame->data,           // 源数据
            srcFrame->linesize,       // 源行大小
            0,                        // 起始行
            srcFrame->height,         // 行数
            m_dstFrame->data,         // 目标数据
            m_dstFrame->linesize      // 目标行大小
        );
        
        if (ret < 0) {
            std::cerr << "转换失败" << std::endl;
            return false;
        }
        
        return true;
    }
    
    AVFrame* getFrame() { return m_dstFrame; }
    
    void close()
    {
        if (m_swsCtx) {
            sws_freeContext(m_swsCtx);
            m_swsCtx = nullptr;
        }
        if (m_dstFrame) {
            av_frame_free(&m_dstFrame);
        }
    }
    
private:
    SwsContext *m_swsCtx = nullptr;
    AVFrame *m_dstFrame = nullptr;
};
```



---

## 6. 音频解码

### 6.1 基础音频解码

```cpp
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

class AudioDecoder
{
public:
    bool open(const char *filename)
    {
        // ========== 打开文件 ==========
        int ret = avformat_open_input(&m_formatCtx, filename, nullptr, nullptr);
        if (ret < 0) return false;
        
        avformat_find_stream_info(m_formatCtx, nullptr);
        
        // ========== 查找音频流 ==========
        m_audioStreamIndex = av_find_best_stream(
            m_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
        
        if (m_audioStreamIndex < 0) {
            std::cerr << "未找到音频流" << std::endl;
            return false;
        }
        
        // ========== 打开解码器 ==========
        AVStream *audioStream = m_formatCtx->streams[m_audioStreamIndex];
        const AVCodec *codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
        
        m_codecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(m_codecCtx, audioStream->codecpar);
        avcodec_open2(m_codecCtx, codec, nullptr);
        
        m_frame = av_frame_alloc();
        m_packet = av_packet_alloc();
        
        std::cout << "音频信息:" << std::endl;
        std::cout << "  采样率: " << m_codecCtx->sample_rate << " Hz" << std::endl;
        std::cout << "  声道数: " << m_codecCtx->ch_layout.nb_channels << std::endl;
        std::cout << "  采样格式: " << av_get_sample_fmt_name(m_codecCtx->sample_fmt) << std::endl;
        
        return true;
    }
    
    bool decodeFrame()
    {
        while (true) {
            int ret = av_read_frame(m_formatCtx, m_packet);
            if (ret < 0) return false;
            
            if (m_packet->stream_index != m_audioStreamIndex) {
                av_packet_unref(m_packet);
                continue;
            }
            
            ret = avcodec_send_packet(m_codecCtx, m_packet);
            av_packet_unref(m_packet);
            
            if (ret < 0) return false;
            
            ret = avcodec_receive_frame(m_codecCtx, m_frame);
            
            if (ret == AVERROR(EAGAIN)) {
                continue;
            } else if (ret < 0) {
                return false;
            }
            
            std::cout << "解码音频帧: " << m_frame->nb_samples << " 采样" << std::endl;
            return true;
        }
    }
    
    void close()
    {
        if (m_frame) av_frame_free(&m_frame);
        if (m_packet) av_packet_free(&m_packet);
        if (m_codecCtx) avcodec_free_context(&m_codecCtx);
        if (m_formatCtx) avformat_close_input(&m_formatCtx);
    }
    
    AVFrame* getFrame() { return m_frame; }
    AVCodecContext* getCodecContext() { return m_codecCtx; }
    
private:
    AVFormatContext *m_formatCtx = nullptr;
    AVCodecContext *m_codecCtx = nullptr;
    AVFrame *m_frame = nullptr;
    AVPacket *m_packet = nullptr;
    int m_audioStreamIndex = -1;
};
```

### 6.2 音频重采样

```cpp
class AudioResampler
{
public:
    bool init(AVCodecContext *codecCtx, 
             int outSampleRate, 
             AVChannelLayout outChLayout,
             AVSampleFormat outSampleFmt)
    {
        // ========== 创建重采样上下文 ==========
        int ret = swr_alloc_set_opts2(
            &m_swrCtx,
            &outChLayout,                    // 输出声道布局
            outSampleFmt,                    // 输出采样格式
            outSampleRate,                   // 输出采样率
            &codecCtx->ch_layout,            // 输入声道布局
            codecCtx->sample_fmt,            // 输入采样格式
            codecCtx->sample_rate,           // 输入采样率
            0, nullptr
        );
        
        if (ret < 0) {
            std::cerr << "无法分配重采样上下文" << std::endl;
            return false;
        }
        
        ret = swr_init(m_swrCtx);
        if (ret < 0) {
            std::cerr << "无法初始化重采样上下文" << std::endl;
            return false;
        }
        
        m_outSampleRate = outSampleRate;
        m_outChannels = outChLayout.nb_channels;
        m_outSampleFmt = outSampleFmt;
        
        return true;
    }
    
    int resample(AVFrame *inFrame, uint8_t **outData)
    {
        // ========== 计算输出采样数 ==========
        int outSamples = av_rescale_rnd(
            swr_get_delay(m_swrCtx, inFrame->sample_rate) + inFrame->nb_samples,
            m_outSampleRate,
            inFrame->sample_rate,
            AV_ROUND_UP
        );
        
        // ========== 分配输出缓冲 ==========
        if (!*outData) {
            av_samples_alloc(
                outData,
                nullptr,
                m_outChannels,
                outSamples,
                m_outSampleFmt,
                0
            );
        }
        
        // ========== 执行重采样 ==========
        int samples = swr_convert(
            m_swrCtx,
            outData,
            outSamples,
            (const uint8_t**)inFrame->data,
            inFrame->nb_samples
        );
        
        if (samples < 0) {
            std::cerr << "重采样失败" << std::endl;
            return -1;
        }
        
        return samples;
    }
    
    void close()
    {
        if (m_swrCtx) {
            swr_free(&m_swrCtx);
        }
    }
    
private:
    SwrContext *m_swrCtx = nullptr;
    int m_outSampleRate = 0;
    int m_outChannels = 0;
    AVSampleFormat m_outSampleFmt = AV_SAMPLE_FMT_NONE;
};
```

---

## 7. 视频播放器实现

### 7.1 播放器核心类

```cpp
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

class VideoPlayer
{
public:
    VideoPlayer() = default;
    ~VideoPlayer() { close(); }
    
    // ========== 打开文件 ==========
    bool open(const char *filename)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // 打开输入文件
        int ret = avformat_open_input(&m_formatCtx, filename, nullptr, nullptr);
        if (ret < 0) {
            std::cerr << "无法打开文件" << std::endl;
            return false;
        }
        
        // 读取流信息
        ret = avformat_find_stream_info(m_formatCtx, nullptr);
        if (ret < 0) {
            std::cerr << "无法获取流信息" << std::endl;
            return false;
        }
        
        // 查找视频流
        m_videoStreamIndex = av_find_best_stream(
            m_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        
        // 查找音频流
        m_audioStreamIndex = av_find_best_stream(
            m_formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
        
        // 打开视频解码器
        if (m_videoStreamIndex >= 0) {
            if (!openVideoDecoder()) {
                return false;
            }
        }
        
        // 打开音频解码器
        if (m_audioStreamIndex >= 0) {
            if (!openAudioDecoder()) {
                return false;
            }
        }
        
        // 获取时长
        m_duration = m_formatCtx->duration / AV_TIME_BASE;
        
        std::cout << "文件打开成功" << std::endl;
        std::cout << "时长: " << m_duration << " 秒" << std::endl;
        
        return true;
    }
    
    // ========== 播放 ==========
    void play()
    {
        if (m_isPlaying) return;
        
        m_isPlaying = true;
        m_isPaused = false;
        
        // 启动解码线程
        m_decodeThread = std::thread(&VideoPlayer::decodeLoop, this);
    }
    
    // ========== 暂停 ==========
    void pause()
    {
        m_isPaused = true;
    }
    
    // ========== 恢复 ==========
    void resume()
    {
        m_isPaused = false;
        m_pauseCondition.notify_all();
    }
    
    // ========== 停止 ==========
    void stop()
    {
        m_isPlaying = false;
        m_isPaused = false;
        m_pauseCondition.notify_all();
        
        if (m_decodeThread.joinable()) {
            m_decodeThread.join();
        }
    }
    
    // ========== 跳转 ==========
    bool seek(double seconds)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        int64_t timestamp = seconds * AV_TIME_BASE;
        
        int ret = av_seek_frame(m_formatCtx, -1, timestamp, AVSEEK_FLAG_BACKWARD);
        if (ret < 0) {
            std::cerr << "跳转失败" << std::endl;
            return false;
        }
        
        // 刷新解码器
        if (m_videoCodecCtx) {
            avcodec_flush_buffers(m_videoCodecCtx);
        }
        if (m_audioCodecCtx) {
            avcodec_flush_buffers(m_audioCodecCtx);
        }
        
        m_currentTime = seconds;
        
        std::cout << "跳转到: " << seconds << " 秒" << std::endl;
        
        return true;
    }
    
    // ========== 快进 ==========
    void forward(double seconds = 10.0)
    {
        double newTime = m_currentTime + seconds;
        if (newTime > m_duration) {
            newTime = m_duration;
        }
        seek(newTime);
    }
    
    // ========== 快退 ==========
    void backward(double seconds = 10.0)
    {
        double newTime = m_currentTime - seconds;
        if (newTime < 0) {
            newTime = 0;
        }
        seek(newTime);
    }
    
    // ========== 获取状态 ==========
    bool isPlaying() const { return m_isPlaying; }
    bool isPaused() const { return m_isPaused; }
    double getCurrentTime() const { return m_currentTime; }
    double getDuration() const { return m_duration; }
    
    // ========== 关闭 ==========
    void close()
    {
        stop();
        
        if (m_videoCodecCtx) {
            avcodec_free_context(&m_videoCodecCtx);
        }
        if (m_audioCodecCtx) {
            avcodec_free_context(&m_audioCodecCtx);
        }
        if (m_formatCtx) {
            avformat_close_input(&m_formatCtx);
        }
    }
    
private:
    bool openVideoDecoder()
    {
        AVStream *stream = m_formatCtx->streams[m_videoStreamIndex];
        const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
        
        if (!codec) return false;
        
        m_videoCodecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(m_videoCodecCtx, stream->codecpar);
        
        int ret = avcodec_open2(m_videoCodecCtx, codec, nullptr);
        if (ret < 0) return false;
        
        // 计算帧率
        m_fps = av_q2d(stream->avg_frame_rate);
        m_frameDelay = 1.0 / m_fps;
        
        std::cout << "视频: " << m_videoCodecCtx->width << "x" 
                 << m_videoCodecCtx->height << " @ " << m_fps << " fps" << std::endl;
        
        return true;
    }
    
    bool openAudioDecoder()
    {
        AVStream *stream = m_formatCtx->streams[m_audioStreamIndex];
        const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
        
        if (!codec) return false;
        
        m_audioCodecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(m_audioCodecCtx, stream->codecpar);
        
        int ret = avcodec_open2(m_audioCodecCtx, codec, nullptr);
        if (ret < 0) return false;
        
        std::cout << "音频: " << m_audioCodecCtx->sample_rate << " Hz, " 
                 << m_audioCodecCtx->ch_layout.nb_channels << " 声道" << std::endl;
        
        return true;
    }
    
    void decodeLoop()
    {
        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();
        
        auto startTime = std::chrono::steady_clock::now();
        
        while (m_isPlaying) {
            // 暂停处理
            if (m_isPaused) {
                std::unique_lock<std::mutex> lock(m_pauseMutex);
                m_pauseCondition.wait(lock, [this]() { return !m_isPaused || !m_isPlaying; });
                startTime = std::chrono::steady_clock::now();
                continue;
            }
            
            // 读取数据包
            int ret = av_read_frame(m_formatCtx, packet);
            
            if (ret < 0) {
                if (ret == AVERROR_EOF) {
                    std::cout << "播放结束" << std::endl;
                    m_isPlaying = false;
                }
                break;
            }
            
            // 处理视频帧
            if (packet->stream_index == m_videoStreamIndex) {
                ret = avcodec_send_packet(m_videoCodecCtx, packet);
                
                while (ret >= 0) {
                    ret = avcodec_receive_frame(m_videoCodecCtx, frame);
                    
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    }
                    
                    if (ret >= 0) {
                        // 计算当前时间
                        m_currentTime = frame->pts * av_q2d(m_formatCtx->streams[m_videoStreamIndex]->time_base);
                        
                        // 显示帧（这里需要实现实际的显示逻辑）
                        displayFrame(frame);
                        
                        // 帧率控制
                        auto now = std::chrono::steady_clock::now();
                        auto elapsed = std::chrono::duration<double>(now - startTime).count();
                        
                        if (elapsed < m_frameDelay) {
                            std::this_thread::sleep_for(
                                std::chrono::duration<double>(m_frameDelay - elapsed));
                        }
                        
                        startTime = std::chrono::steady_clock::now();
                    }
                }
            }
            // 处理音频帧
            else if (packet->stream_index == m_audioStreamIndex) {
                ret = avcodec_send_packet(m_audioCodecCtx, packet);
                
                while (ret >= 0) {
                    ret = avcodec_receive_frame(m_audioCodecCtx, frame);
                    
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    }
                    
                    if (ret >= 0) {
                        // 播放音频（这里需要实现实际的音频播放逻辑）
                        playAudio(frame);
                    }
                }
            }
            
            av_packet_unref(packet);
        }
        
        av_frame_free(&frame);
        av_packet_free(&packet);
    }
    
    void displayFrame(AVFrame *frame)
    {
        // 这里应该实现实际的显示逻辑
        // 例如：使用 SDL、Qt、或其他图形库显示
        std::cout << "显示帧: " << m_currentTime << " 秒" << std::endl;
    }
    
    void playAudio(AVFrame *frame)
    {
        // 这里应该实现实际的音频播放逻辑
        // 例如：使用 SDL_Audio、PortAudio 等
    }
    
private:
    AVFormatContext *m_formatCtx = nullptr;
    AVCodecContext *m_videoCodecCtx = nullptr;
    AVCodecContext *m_audioCodecCtx = nullptr;
    
    int m_videoStreamIndex = -1;
    int m_audioStreamIndex = -1;
    
    std::atomic<bool> m_isPlaying{false};
    std::atomic<bool> m_isPaused{false};
    std::atomic<double> m_currentTime{0.0};
    double m_duration = 0.0;
    double m_fps = 0.0;
    double m_frameDelay = 0.0;
    
    std::thread m_decodeThread;
    std::mutex m_mutex;
    std::mutex m_pauseMutex;
    std::condition_variable m_pauseCondition;
};
```

---

## 8. 播放控制

### 8.1 播放控制接口

```cpp
// 使用示例
int main()
{
    VideoPlayer player;
    
    // 打开文件
    if (!player.open("video.mp4")) {
        return -1;
    }
    
    // 播放
    player.play();
    
    // 等待用户输入控制命令
    std::string command;
    while (player.isPlaying()) {
        std::cout << "\n命令: [p]暂停 [r]恢复 [f]快进 [b]快退 [s]停止 [q]退出" << std::endl;
        std::cout << "当前时间: " << player.getCurrentTime() << " / " << player.getDuration() << " 秒" << std::endl;
        std::cout << "> ";
        
        std::cin >> command;
        
        if (command == "p") {
            player.pause();
            std::cout << "已暂停" << std::endl;
        }
        else if (command == "r") {
            player.resume();
            std::cout << "已恢复" << std::endl;
        }
        else if (command == "f") {
            player.forward(10.0);
            std::cout << "快进 10 秒" << std::endl;
        }
        else if (command == "b") {
            player.backward(10.0);
            std::cout << "快退 10 秒" << std::endl;
        }
        else if (command == "s") {
            player.stop();
            std::cout << "已停止" << std::endl;
            break;
        }
        else if (command == "q") {
            player.stop();
            break;
        }
    }
    
    player.close();
    
    return 0;
}
```

---

## 总结

### 核心知识点

1. **FFmpeg 基础**
   - 核心组件和数据结构
   - 基本流程和 API

2. **视频解码**
   - 打开文件和查找流
   - 解码器使用
   - 图像格式转换

3. **音频解码**
   - 音频流处理
   - 音频重采样

4. **播放器实现**
   - 播放控制（播放、暂停、停止）
   - 跳转功能（快进、快退）
   - 多线程处理

### 最佳实践

✅ **推荐做法**
- 正确管理资源（及时释放）
- 使用多线程解码
- 实现音视频同步
- 处理错误情况

❌ **避免做法**
- 内存泄漏
- 阻塞主线程
- 忽略错误处理

### 参考资源

- **官方文档**: https://ffmpeg.org/documentation.html
- **API 文档**: https://ffmpeg.org/doxygen/trunk/
- **示例代码**: https://github.com/FFmpeg/FFmpeg/tree/master/doc/examples

---

**祝你在 FFmpeg 开发中取得成功！** 🎬
