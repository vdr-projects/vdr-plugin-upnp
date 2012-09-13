/*
 * connection.h
 *
 *  Created on: 31.07.2012
 *      Author: savop
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <vdr/tools.h>
#include <string>

namespace upnp {

class cVirtualConnection {
public:
  enum Direction {
    VC_OUTPUT,
    VC_INPUT,
    NumDirections
  };

  enum Status {
    VC_OKAY,
    VC_CONTENT_FORMAT_MISMATCH,
    VC_INSUFFICIENT_BANDWIDTH,
    VC_UNRELIABLE_CHANNEL,
    VC_UNKNOWN,
    NumStatus
  };

  static cVirtualConnection* GenerateVirtualConnection(
      const std::string& pemoteProtocolInfo,
      const std::string& peerConnectionManager,
      int32_t peerConnectionID,
      Direction direction);

  static cVirtualConnection* GenerateVirtualConnection(
      const std::string& pemoteProtocolInfo,
      const std::string& peerConnectionManager,
      int32_t peerConnectionID,
      const std::string& direction);

  static void DestroyVirtualConnection(cVirtualConnection* connection);

  const std::string& GetRemoteProtocolInfo() const { return mRemoteProtocolInfo; }
  const std::string& GetPeerConnectionManager() const { return mPeerConnectionManager; }
  int32_t GetPeerConnectionID() const { return mPeerConnectionID; }
  int32_t GetConnectionID() const { return mConnectionID; }
  int32_t GetAVTransportID() const { return mAVTransportID; }
  int32_t GetRcsID() const { return mRcsID; }
  Direction GetDirection() const { return mDirection; }
  std::string GetDirectionString() const;
  Status GetStatus() const { return mStatus; }
  std::string GetStatusString() const;

private:
  cVirtualConnection(
      int32_t connectionID,
      const std::string & remoteProtocolInfo,
      const std::string & peerConnectionManager,
      int32_t peerConnectionID,
      Direction direction);
  cVirtualConnection(const cVirtualConnection & other);
  cVirtualConnection& operator=(const cVirtualConnection & other);

  virtual ~cVirtualConnection(){}

  static int32_t NextConnectionID();

  std::string mRemoteProtocolInfo;
  std::string mPeerConnectionManager;
  int32_t     mPeerConnectionID;
  int32_t     mConnectionID;
  int32_t     mAVTransportID;
  int32_t     mRcsID;
  Direction   mDirection;
  Status      mStatus;
};

};

#endif /* CONNECTION_H_ */
