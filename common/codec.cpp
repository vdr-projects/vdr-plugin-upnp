/*
 * codec.cpp
 *
 *  Created on: 13.10.2012
 *      Author: savop
 */

#include "../include/tools/codec.h"

namespace upnp {

namespace codec {

cFormatContext::cFormatContext()
: formatCtx(NULL)
{
  av_register_all();
}

cFormatContext::~cFormatContext(){
  av_free(formatCtx);
}

bool cFormatContext::Open(const string& file){
  if(avformat_open_input(&formatCtx, file.c_str(), NULL, NULL) == 0){
    if(avformat_find_stream_info(formatCtx, NULL) == 0){
      return true;
    }
  }

  return false;
}

const AVCodec* cFormatContext::GetCodec(AVMediaType Type) const {
  return avcodec_find_decoder(GetCodecContext(Type)->codec_id);
}

const AVCodecContext* cFormatContext::GetCodecContext(AVMediaType Type) const {
  const AVStream* strm = GetStream(Type);
  return strm?strm->codec:NULL;
}

const AVStream* cFormatContext::GetStream(AVMediaType Type) const {
  if(!formatCtx) return NULL;
  int Stream = -1; unsigned int i;
  for(i = 0; i < formatCtx->nb_streams; i++){
    if(formatCtx->streams[i]->codec->codec_type == Type){
      Stream = i;
      break;
    }
  }
  if(Stream == -1){
    return NULL;
  }

  return formatCtx->streams[Stream];
}

AVFormatContext* cFormatContext::operator*() const {
  return formatCtx;
}

}  // namespace codec

}  // namespace upnp
