/*
 * codec.h
 *
 *  Created on: 12.10.2012
 *      Author: savop
 */

#ifndef CODEC_H_
#define CODEC_H_

#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 #include <stdint.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <string>

using namespace std;

namespace upnp {

namespace codec {

class cFormatContext {
public:
  cFormatContext();
  virtual ~cFormatContext();

  bool Open(const string& file);

  const AVCodec* GetCodec(AVMediaType Type) const;
  const AVCodecContext* GetCodecContext(AVMediaType Type) const;
  const AVStream* GetStream(AVMediaType Type) const;

  AVFormatContext* operator*() const;
private:
  AVFormatContext* formatCtx;
};

}

}


#endif /* CODEC_H_ */
