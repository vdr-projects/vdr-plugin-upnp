/*
 * profile.h
 *
 *  Created on: 05.08.2012
 *      Author: savop
 */

#ifndef PROFILE_H_
#define PROFILE_H_

#include <string>
#include <stdint.h>

using namespace std;

#define DLNA_OPERATION_NONE                 00          ///< No seek operations supported
#define DLNA_OPERATION_TIME_SEEK_RANGE      10          ///< is the server supporting time based seeks?
#define DLNA_OPERATION_RANGE                01          ///< or byte based seeks?

#define DLNA_CONVERSION_TRANSCODED          1           ///< the content was converted from one media format to another
#define DLNA_CONVERSION_NONE                0           ///< the content is available without conversion

#define DLNA_PLAYSPEEDS_NONE                ""

#define DLNA_FLAG_NONE                      0
#define DLNA_FLAG_SENDER_PACED              1 << 31     ///< is the server setting the pace (i.e. RTP)?
#define DLNA_FLAG_TIME_BASED_SEEK           1 << 30     ///< is the server supporting time based seeks?
#define DLNA_FLAG_BYTE_BASED_SEEK           1 << 29     ///< or byte based seeking?
#define DLNA_FLAG_PLAY_CONTAINER            1 << 28     ///< is it possible to play all contents of a container?
#define DLNA_FLAG_S0_INCREASE               1 << 27     ///< is the beginning changing (time shift)?
#define DLNA_FLAG_SN_INCREASE               1 << 26     ///< is the end changing (live-TV)?
#define DLNA_FLAG_RTSP_PAUSE                1 << 25     ///< is pausing rtp streams permitted?
#define DLNA_FLAG_STREAMING_TRANSFER        1 << 24     ///< is the transfer a stream (Audio/AV)?
#define DLNA_FLAG_INTERACTIVE_TRANSFER      1 << 23     ///< is the transfer interactiv (printings)?
#define DLNA_FLAG_BACKGROUND_TRANSFER       1 << 22     ///< is the tranfer done in background (downloaded)?
#define DLNA_FLAG_CONNECTION_STALLING       1 << 21     ///< can the connection be paused on HTTP streams?
#define DLNA_FLAG_VERSION_1_5               1 << 20     ///< does the server complies with DLNA V1.5
#define DLNA_FLAG_CLEARTEXT_CONTENT         1 << 16     ///< (Link Protection) currently not used
#define DLNA_FLAG_CLEARTEXT_BYTE_FULL_SEEK  1 << 15     ///< (Link Protection) currently not used
#define DLNA_FLAG_CLEARTEXT_LIMITED_SEEK    1 << 14     ///< (Link Protection) currently not used

namespace upnp {

struct DLNA4thField {
  DLNA4thField();
  DLNA4thField(string pn,
               uint8_t op = DLNA_OPERATION_NONE,
               string ps = DLNA_PLAYSPEEDS_NONE,
               bool ci = DLNA_CONVERSION_NONE,
               uint32_t flags = DLNA_FLAG_NONE);

  string    profile;
  uint8_t   operations;
  string    playSpeeds;
  bool      conversionIndicator;
  uint32_t  primaryFlags;

  string ToString();
};

struct ProtocolInfo {
  enum StreamType {
    DLNA_STREAM_HTTP,
    DLNA_STREAM_RTP
  };

  ProtocolInfo();
  ProtocolInfo(string contentType, DLNA4thField fourthField, StreamType type = DLNA_STREAM_HTTP);

  StreamType    streamType;
  string        contentType;
  DLNA4thField  fourthField;

  string ToString();
};

namespace video {

}

namespace audio {

}

namespace image {

struct cIcon {
  const char* mime;     ///< the mime type of the image
  uint16_t width;    ///< image width in pixel
  uint16_t height;   ///< image height in pixel
  uint8_t  bitDepth; ///< bit depth in bits per pixel
};

/* JPEG Icons */
//extern cIcon DLNA_ICON_JEPG_TN;               ///< DLNA jpeg thumbnail profile of images
extern cIcon DLNA_ICON_JPEG_SM_24;            ///< DLNA icon profile of small jpeg images
extern cIcon DLNA_ICON_JPEG_LRG_24;           ///< DLNA icon profile of large jpeg images

/* PNG Icons */
extern cIcon DLNA_ICON_PNG_SM_24A;            ///< DLNA icon profile of small png images
extern cIcon DLNA_ICON_PNG_LRG_24A;           ///< DLNA icon profile of large png images
//extern cIcon DLNA_ICON_PNG_TN;                ///< DLNA png thumbnail profile of images

} /* NS:image */

} /* NS:upnp */


#endif /* PROFILE_H_ */
