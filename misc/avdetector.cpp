/* 
 * File:   avdetector.cpp
 * Author: savop
 * 
 * Created on 26. Oktober 2009, 13:01
 */

#include "avdetector.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using namespace std;

int cAudioVideoDetector::detectVideoProperties(cUPnPResource* Resource, const char* Filename){
//    // Register avformat
    av_register_all();

    AVFormatContext *FormatCtx = NULL;
    if(av_open_input_file(&FormatCtx, Filename, NULL, 0, NULL)){
        ERROR("AVDetector: Error while opening file %s", Filename);
        return -1;
    }

    if(av_find_stream_info(FormatCtx)<0){
        ERROR("AVDetector: Cannot find the stream information");
        return -1;
    }

#ifdef DEBUG
    dump_format(FormatCtx, 0, Filename, 0);
#endif

    unsigned int i; int videoStream = -1;

    AVCodecContext *CodecCtx = NULL;
    for(i = 0; i < FormatCtx->nb_streams; i++){
        if(FormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO){
            videoStream = i;
            break;
        }
    }
    if(videoStream == -1){
        ERROR("AVDetector: No video stream found");
        return -1;
    }

    CodecCtx = FormatCtx->streams[videoStream]->codec;

    AVCodec* Codec = avcodec_find_decoder(CodecCtx->codec_id);

    unsigned int width = CodecCtx->width;
    unsigned int height = CodecCtx->height;
    unsigned int bitrate = CodecCtx->bit_rate;
    const char* codecName = (Codec)?Codec->name:"unknown";

    MESSAGE("AVDetector: %s-stream %dx%d at %d bit/s", codecName, width, height, bitrate);

    Resource->mBitrate = bitrate;
    Resource->mSampleFrequency = CodecCtx->sample_rate;
    Resource->mResolution = cString::sprintf("%dx%d", width, height);
    
    return 0;

}
