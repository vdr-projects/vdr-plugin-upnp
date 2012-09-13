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

namespace upnp {

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
