/*
 * connection.cpp
 *
 *  Created on: 31.07.2012
 *      Author: savop
 */

#ifndef CONNECTION_CPP_
#define CONNECTION_CPP_

#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "../include/connection.h"

using namespace upnp;

cVirtualConnection::cVirtualConnection(
    int32_t connectionID,
    const std::string& remoteProtocolInfo,
    const std::string& peerConnectionManager,
    int32_t peerConnectionID,
    Direction direction) :
  mRemoteProtocolInfo(remoteProtocolInfo),
  mPeerConnectionManager(peerConnectionManager),
  mPeerConnectionID(peerConnectionID),
  mConnectionID(connectionID),
  mAVTransportID(-1),
  mRcsID(-1),
  mDirection(direction),
  mStatus(VC_OKAY)
{
}

cVirtualConnection::cVirtualConnection(const cVirtualConnection & other) :
  mRemoteProtocolInfo(other.mRemoteProtocolInfo),
  mPeerConnectionManager(other.mPeerConnectionManager),
  mPeerConnectionID(other.mPeerConnectionID),
  mConnectionID(other.mConnectionID),
  mAVTransportID(other.mAVTransportID),
  mRcsID(other.mRcsID),
  mDirection(other.mDirection),
  mStatus(other.mStatus)
{
}

cVirtualConnection & cVirtualConnection::operator =(const cVirtualConnection & other)
{
  mConnectionID = other.mConnectionID;
  mAVTransportID = other.mAVTransportID;
  mPeerConnectionID = other.mPeerConnectionID;
  mRcsID = other.mRcsID;
  mDirection = other.mDirection;
  mStatus = other.mStatus;
  mPeerConnectionManager = other.mPeerConnectionManager;
  mRemoteProtocolInfo = other.mRemoteProtocolInfo;

  return *this;
}

cVirtualConnection* cVirtualConnection::GenerateVirtualConnection(
    const std::string & remoteProtocolInfo,
    const std::string & peerConnectionManager,
    int32_t peerConnectionID,
    const std::string & direction)
{
  Direction eDirection;

  if        (direction.compare("Input")==0){
    eDirection = VC_INPUT;
  } else if (direction.compare("Output")==0){
    eDirection = VC_OUTPUT;
  } else {
    return NULL;
  }

  cVirtualConnection* connnection = new cVirtualConnection(
      cVirtualConnection::NextConnectionID(),
      remoteProtocolInfo, peerConnectionManager,
      peerConnectionID, eDirection );

  return connnection;
}

cVirtualConnection* cVirtualConnection::GenerateVirtualConnection(
    const std::string & remoteProtocolInfo,
    const std::string & peerConnectionManager,
    int32_t peerConnectionID,
    Direction direction)
{
  cVirtualConnection* connnection = new cVirtualConnection(
      cVirtualConnection::NextConnectionID(),
      remoteProtocolInfo, peerConnectionManager,
      peerConnectionID, direction );

  return connnection;
}

void cVirtualConnection::DestroyVirtualConnection(cVirtualConnection* connection){
  delete connection;
}

int32_t cVirtualConnection::NextConnectionID(){
  static int32_t lastConnectionID = 0;

  if(lastConnectionID == INT_MAX) lastConnectionID = 1;

  return lastConnectionID++;
}

std::string cVirtualConnection::GetDirectionString() const {
  switch (mDirection){
  case VC_INPUT:
    return "Input";
  case VC_OUTPUT:
    return "Output";
  default:
    return NULL;
  }
}

std::string cVirtualConnection::GetStatusString() const {
  switch (mStatus){
  case VC_CONTENT_FORMAT_MISMATCH:
    return "ContentFormatMismatch";
  case VC_INSUFFICIENT_BANDWIDTH:
    return "InsufficientBandwidth";
  case VC_OKAY:
    return "OK";
  case VC_UNKNOWN:
    return "Unknown";
  case VC_UNRELIABLE_CHANNEL:
    return "UnreliableChannel";
  default:
    return NULL;
  }
}

#endif /* CONNECTION_CPP_ */
