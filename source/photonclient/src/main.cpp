//
// Created by carl on 20-3-31.
//

#include <iostream>
#include <zconf.h>
extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcode/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
#include "MediaPlayer.h"

class FFMPEGFrameData : public IVideoFrame {
public:
    FFMPEGFrameData()
        : avFrame_(av_frame_alloc())
    {
    }
    ~FFMPEGFrameData()
    {
        av_frame_unref(avFrame_);
    }
    const uint8_t* Y() const override
    {
        return avFrame_->data[0];
    }
    const uint8_t* U() const override
    {
        return avFrame_->data[1];
    }
    const uint8_t* V() const override
    {
        return avFrame_->data[2];
    }
    uint32_t YStride() const override
    {
        return avFrame_->linesize[0];
    }
    uint32_t UStride() const override
    {
        return avFrame_->linesize[1];
    }
    uint32_t VStride() const override
    {
        return avFrame_->linesize[2];
    }
    uint32_t Width() const override
    {
        return avFrame_->width;
    }
    uint32_t Height() const override
    {
        return avFrame_->height;
    }

public:
    AVFrame* avFrame_;
};

int main()
{
    MediaPlayer::Config config {
        "Photon Client",
        100, 100, 640, 480
    };
    MediaPlayer player(&config);
    player.Start();

    avdevice_register_all();

    // https://trac.ffmpeg.org/wiki/Capture/Webcam
    //    AVFormatContext* pFormatCtx = avformat_alloc_context();
    AVInputFormat* ifmt = av_find_input_format("v4l2");
    AVFormatContext* formatContext = avformat_alloc_context();
    const char* dev_name = "/dev/video0"; // here mine is video0 , it may vary.
    AVDictionary* options = NULL;
    av_dict_set(&options, "framerate", "20", 0);
    //    AVDictionary* options = NULL;
    //    av_dict_set(&options, "list_options", "true", 0);
    avformat_open_input(&formatContext, dev_name, ifmt, &options);
    AVDeviceInfoList* deviceInfoList;

    //    avdevice_list_devices(formatContext, &deviceInfoList);
    //    for (int i = 0; i < deviceInfoList->nb_devices; ++i) {
    //        std::cout << "=-===" << deviceInfoList->devices[i]->device_name << "\t" << deviceInfoList->devices[i]->device_description << std::endl;
    //    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    int videoindex = -1;
    for (int i = 0; i < formatContext->nb_streams; i++)
        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoindex = i;
            break;
        }
    if (videoindex == -1) {
        printf("Couldn't find a video stream.\n");
        return -1;
    }
    auto* pCodecCtx = formatContext->streams[videoindex]->codec;
    auto* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        printf("Codec not found.\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec.\n");
        return -1;
    }
    auto* pFrame = av_frame_alloc();
    //    auto* pFrameYUV = av_frame_alloc();

    AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));

    int got_picture;
    std::shared_ptr<FFMPEGFrameData> playerFrame;
    playerFrame = std::make_shared<FFMPEGFrameData>();
    struct SwsContext* img_convert_ctx;
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    while (true) {

        if (av_read_frame(formatContext, packet) >= 0) {
            if (packet->stream_index == videoindex) {

                auto ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
                if (ret < 0) {
                    printf("Decode Error.\n");
                    return -1;
                }
                if (got_picture) {
                    auto* pFrameYUV = playerFrame->avFrame_;



                    sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

                    player.SetVideoFrame(playerFrame);
                    playerFrame = std::make_shared<FFMPEGFrameData>();
                    std::cout << "Picture " << std::endl;
                    //                SDL_LockYUVOverlay(bmp);
                    //                pFrameYUV->data[0]=bmp->pixels[0];
                    //                pFrameYUV->data[1]=bmp->pixels[2];
                    //                pFrameYUV->data[2]=bmp->pixels[1];
                    //                pFrameYUV->linesize[0]=bmp->pitches[0];
                    //                pFrameYUV->linesize[1]=bmp->pitches[2];
                    //                pFrameYUV->linesize[2]=bmp->pitches[1];
                    //                sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
                    //
                    //#if OUTPUT_YUV420P
                    //                int y_size=pCodecCtx->width*pCodecCtx->height;
                    //						fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
                    //						fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
                    //						fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
                    //#endif
                    //
                    //                SDL_UnlockYUVOverlay(bmp);
                    //
                    //                SDL_DisplayYUVOverlay(bmp, &rect);
                }
            }
            av_packet_unref(packet);
        } else {
            //Exit Thread
        }
    }

    std::cout << "client" << std::endl;
    return 0;
}