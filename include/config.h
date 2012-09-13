/*
 * config.h
 *
 *  Created on: 03.08.2012
 *      Author: savop
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <stdint.h>
#include "media/profile.h"

using namespace std;

namespace upnp {

struct cConfig {
  cConfig();
  /**
   * Enable the media server
   *
   * If this is true, the media server is running on startup.
   */
  bool enabled;
  /**
   * Enable expert settings
   *
   * If this is true, the user may customize additional settings, which should
   * only be done, if the user is very experienced or want to use that for
   * debugging.
   */
  bool expertSettings;
  /**
   * Use the internal web server
   *
   * If this is true, the internal web server is used for streaming of files.
   * Otherwise an external URL is used.
   */
  bool useInternalWebserver;
  /**
   * Web server root directory
   *
   * This is the directory, where the internal web server is looking for files. If it
   * is empty, the default directory (plugins configuration folder) is used.
   */
  string webServerRoot;
  /**
   * Web server port
   *
   * This is the port where the web server is listening on.
   */
  uint16_t webServerPort;
  /**
   * External web server URL
   *
   * This is the URL, which is used instead of the internal web server URL. If it is
   * empty, the default presentation URL, which is /index.html is used.
   */
  string presentationURL;
  /**
   * Maximum size of SOAP messages
   *
   * This is the maximum size in bytes of soap messages received by the UPnP library.
   */
  size_t maxContentLength;

  /**
   * Maximum age of upnp announcements
   *
   * This is the number of seconds an announcement is valid until the device is declared
   * as out-dated and has to re-new its announcement.
   */
  int announceMaxAge;

  /**
   * DeviceUUID
   *
   * This is the unique identifier for this media server device. If this is empty
   * it will be generated.
   */
  string deviceUUID;

  /**
   * Service URL
   *
   * This is the URL to the services and service descriptors of that services.
   */
  string serviceURL;

  /**
   * Static content URL
   *
   * This is the URL to the static content of the server. They must be located within the
   * network of the server.
   */
  string staticContentURL;
  /**
   * Bind the server to an IP address
   *
   * If this is true, the media server is bound to the specified IP address,
   * otherwise the specified interface will be used.
   */
  bool bindToAddress;

  string address;
  string interface;
  /**
   * The media server listening port
   *
   * This is the port which the media server is listening for incoming connections
   */
  uint16_t port;

  /**
   * The sqlite database file
   *
   * This is the path to the database file.
   */
  string databaseFile;
};

}

#endif /* CONFIG_H_ */
