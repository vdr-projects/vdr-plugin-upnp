/* 
 * File:   livereceiver.cpp
 * Author: savop
 * 
 * Created on 4. Juni 2009, 13:28
 */

#include <vdr/thread.h>
#include <vdr/remux.h>
#include <vdr/device.h>
#include <vdr/channels.h>
#include <vdr/ringbuffer.h>
#include "livereceiver.h"

cLiveReceiver* cLiveReceiver::newInstance(cChannel* Channel, int Priority){
    cDevice *Device = cDevice::GetDevice(Channel, Priority, true);

    if(!Device){
        ERROR("No matching device found to serve this channel!");
        return NULL;
    }

    cLiveReceiver *Receiver = new cLiveReceiver(Channel, Device);
    if(Receiver){
        MESSAGE("Receiver for channel \"%s\" created successfully.", Channel->Name());
        return Receiver;
    }
    else {
        ERROR("Failed to create receiver!");
        return NULL;
    }
}

cLiveReceiver::cLiveReceiver(cChannel *Channel, cDevice *Device)
: cReceiver( Channel->GetChannelID(), 0, Channel->Vpid(), Channel->Apids(), Channel->Dpids(), Channel->Spids()),
  mDevice(Device), mChannel(Channel){
    this->mLiveBuffer = NULL;
    this->mOutputBuffer = NULL;
    this->mFrameDetector = NULL;
}

cLiveReceiver::~cLiveReceiver(void){
    if(this->IsAttached())
        this->Detach();
}

void cLiveReceiver::open(UpnpOpenFileMode){
    this->mLiveBuffer = new cRingBufferLinear(RECEIVER_LIVEBUFFER_SIZE, RECEIVER_RINGBUFFER_MARGIN, true, "Live TV buffer");
    this->mOutputBuffer = new cRingBufferLinear(RECEIVER_OUTPUTBUFFER_SIZE, RECEIVER_RINGBUFFER_MARGIN, true, "Streaming buffer");

    this->mLiveBuffer->SetTimeouts(0, 100);
    this->mOutputBuffer->SetTimeouts(0, 500);
    
    this->mFrameDetector = new cFrameDetector(this->mChannel->Vpid(), this->mChannel->Vtype());
    
    this->mPatPmtGenerator.SetChannel(this->mChannel);
    
    this->mDevice->SwitchChannel(this->mChannel, false);
    this->mDevice->AttachReceiver(this);
}

void cLiveReceiver::Activate(bool On){
    if(On){
        this->Start();
        MESSAGE("Live receiver started.");
    }
    else {
        if(this->Running()){
            this->Cancel(2);
        }
        MESSAGE("Live receiver stopped");
    }
}

void cLiveReceiver::Receive(uchar* Data, int Length){
    if(this->Running()){
        int bytesWrote = this->mLiveBuffer->Put(Data, Length);
        if(bytesWrote != Length && this->Running()){
            this->mLiveBuffer->ReportOverflow(Length - bytesWrote);
        }
    }
}

void cLiveReceiver::Action(void){
    MESSAGE("Started buffering...");
    while(this->Running()){
        int bytesRead;
        //MESSAGE("Buffer is filled with %d bytes", this->mLiveBuffer->Available());
        uchar* bytes = this->mLiveBuffer->Get(bytesRead);
        if(bytes){
            int count = this->mFrameDetector->Analyze(bytes, bytesRead);
            if(count){
                //MESSAGE("%d bytes analyzed", count);
                //MESSAGE("%2.2f FPS", this->mFrameDetector->FramesPerSecond());
                if(!this->Running() && this->mFrameDetector->IndependentFrame())
                    break;
                if(this->mFrameDetector->Synced()){
                    //MESSAGE("Frame detector synced to data stream");
                    if(this->mFrameDetector->IndependentFrame()){
                        this->mOutputBuffer->Put(this->mPatPmtGenerator.GetPat(), TS_SIZE);
                        int i = 0;
                        while(uchar* pmt = this->mPatPmtGenerator.GetPmt(i)){
                            this->mOutputBuffer->Put(pmt, TS_SIZE);
                        }
                    }
                    int bytesWrote = this->mOutputBuffer->Put(bytes, count);
                    if(bytesWrote != count){
                        this->mLiveBuffer->ReportOverflow(count - bytesWrote);
                    }
                    //MESSAGE("Wrote %d to output buffer", bytesWrote);
                    if(bytesWrote){
                        this->mLiveBuffer->Del(bytesWrote);
                    }
                    else {
                        cCondWait::SleepMs(100);
                    }
                }
                else {
                    ERROR("Cannot sync to stream");
                }
            }
        }
    }
    MESSAGE("Receiver was detached from device");
}

int cLiveReceiver::read(char* buf, size_t buflen){
    int bytesRead;    
    if(!this->IsAttached())
        bytesRead = -1;
    else {
        while(!this->mOutputBuffer->Available()){
            WARNING("No data, waiting...");
            cCondWait::SleepMs(50);
            if(!this->IsAttached()){
                MESSAGE("Lost device...");
                return 0;
            }
        }

        uchar* buffer = this->mOutputBuffer->Get(bytesRead);
        if(buffer){
            if(buflen > (size_t)bytesRead){
                memcpy(buf,(char*)buffer,bytesRead);
                this->mOutputBuffer->Del(bytesRead);
            }
            else {
                memcpy(buf,(char*)buffer,buflen);
                this->mOutputBuffer->Del(buflen);
            }
        }

    }
    MESSAGE("Read %d bytes from live feed", bytesRead);
    return bytesRead;
}

int cLiveReceiver::seek(off_t, int){
    ERROR("Seeking not supported on broadcasts");
    return 0;
}

int cLiveReceiver::write(char*, size_t){
    ERROR("Writing not allowed on broadcasts");
    return 0;
}

void cLiveReceiver::close(){
    MESSAGE("Closing live receiver");
    this->Detach();
    delete this->mOutputBuffer; this->mOutputBuffer = NULL;
    delete this->mLiveBuffer; this->mLiveBuffer = NULL;
    this->mFrameDetector = NULL;
    MESSAGE("Live receiver closed.");
}