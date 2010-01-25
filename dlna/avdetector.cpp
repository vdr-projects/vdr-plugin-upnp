/* 
 * File:   avdetector.cpp
 * Author: savop
 * 
 * Created on 26. Oktober 2009, 13:01
 */

#include "avdetector.h"
#include "profiles/container.h"
#include "object.h"
#include <sys/stat.h>

cAudioVideoDetector::cAudioVideoDetector(const char* Filename) : mResourceType(UPNP_RESOURCE_FILE) {
    this->mResource.Filename = Filename;
    this->init();
}

cAudioVideoDetector::cAudioVideoDetector(const cChannel* Channel) : mResourceType(UPNP_RESOURCE_CHANNEL) {
    this->mResource.Channel = Channel;
    this->init();
}

cAudioVideoDetector::cAudioVideoDetector(const cRecording* Recording) : mResourceType(UPNP_RESOURCE_RECORDING) {
    this->mResource.Recording = Recording;
    this->init();
}

cAudioVideoDetector::~cAudioVideoDetector(){
    this->uninit();
}

void cAudioVideoDetector::init(){
    this->mBitrate = 0;
    this->mBitsPerSample = 0;
    this->mColorDepth = 0;
    this->mDLNAProfile = NULL;
    this->mDuration = 0;
    this->mHeight = 0;
    this->mNrAudioChannels = 0;
    this->mSampleFrequency = 0;
    this->mSize = 0;
    this->mWidth = 0;
}

void cAudioVideoDetector::uninit(){
    this->mBitrate = 0;
    this->mBitsPerSample = 0;
    this->mColorDepth = 0;
    this->mDLNAProfile = NULL;
    this->mDuration = 0;
    this->mHeight = 0;
    this->mNrAudioChannels = 0;
    this->mSampleFrequency = 0;
    this->mSize = 0;
    this->mWidth = 0;
}

int cAudioVideoDetector::detectProperties(){
    int ret = 0;
    switch(this->mResourceType){
        case UPNP_RESOURCE_CHANNEL:
            ret = this->detectChannelProperties();
            break;
        case UPNP_RESOURCE_RECORDING:
            ret = this->detectRecordingProperties();
            break;
        case UPNP_RESOURCE_FILE:
            ret = this->detectFileProperties();
            break;
        default:
            WARNING("This resource type is not yet implemented.");
            ret = -1;
            break;
    }

    return ret;
}

int cAudioVideoDetector::detectChannelProperties(){
    MESSAGE(VERBOSE_METADATA, "Detecting channel properties");

    this->mBitrate = 0;
    this->mBitsPerSample = 0;
    this->mColorDepth = 0;
    this->mDuration = 0;
    this->mHeight = 0;
    this->mNrAudioChannels = 0;
    this->mSampleFrequency = 0;
    this->mSize = (off64_t)-1;
    this->mWidth = 0;

    switch(this->mResource.Channel->Vtype()){
        case 0x02:
            // MPEG2 Video
            this->mDLNAProfile = &DLNA_PROFILE_MPEG_TS_SD_EU_ISO;
            break;
        case 0x1B:
            this->mDLNAProfile = &DLNA_PROFILE_AVC_TS_HD_EU_ISO;
            break;
        default:
            ERROR("Unknown video type %d for channel %s!", this->mResource.Channel->Vtype(), this->mResource.Channel->Name());
            this->mDLNAProfile = NULL;
            return -1;
    }

    return 0;
}

int cAudioVideoDetector::detectRecordingProperties(){

    if(this->mResource.Recording->IsPesRecording()){
        ERROR("Sorry, PES Recordings are not supported");
        return -1;
    }

    int ret = 0;
    AVFormatContext *FormatCtx = NULL;
    
    cIndexFile* Index = new cIndexFile(this->mResource.Recording->FileName(), false, this->mResource.Recording->IsPesRecording());
    cFileName*  RecFile = new cFileName(this->mResource.Recording->FileName(), false, false, this->mResource.Recording->IsPesRecording());
    if(Index && Index->Ok()){
        this->mDuration = (off64_t) (Index->Last() * AVDETECTOR_TIME_BASE / SecondsToFrames(1, this->mResource.Recording->FramesPerSecond()));
        MESSAGE(VERBOSE_METADATA,"Record length: %llds", this->mDuration);

        uint16_t    FileNumber = 0;
        off_t       FileOffset = 0;

        if(Index->Get(Index->Last()-1, &FileNumber, &FileOffset))
            for(int i = 0; i < FileNumber; i++){
                struct stat Stats;
                RecFile->SetOffset(i+1);
                stat(RecFile->Name(),&Stats);
                this->mSize += (off64_t) Stats.st_size;
            }

        av_register_all();

        if(!(ret = av_open_input_file(&FormatCtx, RecFile->Name(), NULL, 0, NULL))){
            if((ret = av_find_stream_info(FormatCtx))<0){
                ERROR("AVDetector: Cannot find the stream information");
            }
            else {
                if((ret = this->analyseVideo(FormatCtx))<0){
                    ERROR("AVDetector: Error while analysing video");
                }
                if((ret = this->analyseAudio(FormatCtx))<0){
                    ERROR("AVDetector: Error while analysing audio");
                }
                if((ret = this->detectDLNAProfile(FormatCtx)<0)){
                    ERROR("AVDetector: Error while detecting DLNA Profile");
                }
            }
        }
    }
    else {
        ret = -1;
    }
    
    if(ret != 0){
        ERROR("Error occured while detecting properties");
    }
    
    delete RecFile;
    delete Index;
    av_free(FormatCtx);

    return ret;
}

int cAudioVideoDetector::detectFileProperties(){
    av_register_all();

    int ret = 0;

    AVFormatContext *FormatCtx = NULL;
    
    if(av_open_input_file(&FormatCtx, this->mResource.Filename, NULL, 0, NULL)){
        ERROR("AVDetector: Error while opening file %s", this->mResource.Filename);
        return -1;
    }
    else {
        if(av_find_stream_info(FormatCtx)<0){
            ERROR("AVDetector: Cannot find the stream information");
            return -1;
        }
        else {
            this->mSize = FormatCtx->file_size;
            this->mDuration = FormatCtx->duration;

            MESSAGE(VERBOSE_METADATA, "Format properties: %lld and %lld Bytes", this->mDuration, this->mSize);

            if((ret = this->analyseVideo(FormatCtx))<0){
                ERROR("AVDetector: Error while analysing video");
                return ret;
            }
            if((ret = this->analyseAudio(FormatCtx))<0){
                ERROR("AVDetector: Error while analysing audio");
                return ret;
            }
            if((ret = this->detectDLNAProfile(FormatCtx)<0)){
                ERROR("AVDetector: Error while detecting DLNA Profile");
                return ret;
            }

            return 0;
        }
    }
}

int cAudioVideoDetector::analyseVideo(AVFormatContext* FormatCtx)
{
    AVCodecContext* VideoCodec = cCodecToolKit::getFirstCodecContext(FormatCtx, CODEC_TYPE_VIDEO);
    
    if(!VideoCodec){
        ERROR("AVDetector: codec not found");
        return -1;
    }

    AVCodec* Codec = avcodec_find_decoder(VideoCodec->codec_id);

    this->mWidth = VideoCodec->width;
    this->mHeight = VideoCodec->height;
    this->mBitrate = VideoCodec->bit_rate;
    this->mSampleFrequency = VideoCodec->sample_rate;
    this->mBitsPerSample = VideoCodec->bits_per_raw_sample;

    // TODO: what's the color depth of the stream

    const char* codecName = (Codec)?Codec->name:"unknown";

    MESSAGE(VERBOSE_METADATA, "AVDetector: %s-stream %dx%d at %d bit/s", codecName, this->mWidth, this->mHeight, this->mBitrate);

    return 0;
}

int cAudioVideoDetector::analyseAudio(AVFormatContext* FormatCtx){
    AVCodecContext* AudioCodec = cCodecToolKit::getFirstCodecContext(FormatCtx, CODEC_TYPE_AUDIO);

    if(!AudioCodec){
        ERROR("AVDetector: codec not found");
        return -1;
    }

    AVCodec* Codec = avcodec_find_decoder(AudioCodec->codec_id);

    this->mNrAudioChannels = AudioCodec->channels;

    const char* codecName = (Codec)?Codec->name:"unknown";

    MESSAGE(VERBOSE_METADATA, "AVDetector: %s-stream at %d bit/s", codecName, AudioCodec->bit_rate);

    return 0;
}

int cAudioVideoDetector::detectDLNAProfile(AVFormatContext* FormatCtx){
    DLNAProfile* Profile = MPEG2Profiler.probeDLNAProfile(FormatCtx);
    if(Profile!=NULL){
        this->mDLNAProfile = Profile;
        return 0;
    }
    return -1;
}

AVCodecContext* cCodecToolKit::getFirstCodecContext(AVFormatContext* FormatCtx, CodecType Type){
    return cCodecToolKit::getFirstStream(FormatCtx, Type)->codec;
}

AVStream* cCodecToolKit::getFirstStream(AVFormatContext* FormatCtx, CodecType Type){
    int Stream = -1; unsigned int i;
    for(i = 0; i < FormatCtx->nb_streams; i++){
        if(FormatCtx->streams[i]->codec->codec_type == Type){
            Stream = i;
            break;
        }
    }
    if(Stream == -1){
        ERROR("AVDetector: No matching stream found");
        return NULL;
    }

    return FormatCtx->streams[Stream];
}

bool cCodecToolKit::matchesAcceptedBitrates(AcceptedBitrates Bitrates, AVCodecContext* Codec){
    if(Codec){
        if(Bitrates.VBR){
            if(Bitrates.bitrates[0] <= Codec->bit_rate && Codec->bit_rate <= Bitrates.bitrates[1] ){
                return true;
            }
            else {
                return false;
            }
        }
        else {
            for(int i=0; Bitrates.bitrates[i]; i++){
                if(Codec->bit_rate == Bitrates.bitrates[i]){
                    return true;
                }
            }
            return false;
        }
    }

    return false;
}

bool cCodecToolKit::matchesAcceptedSystemBitrate(AcceptedBitrates Bitrate, AVFormatContext* Format){
    if(Format){
        if(Bitrate.VBR){
            if(Bitrate.bitrates[0] <= Format->bit_rate && Format->bit_rate <= Bitrate.bitrates[1] ){
                return true;
            }
            else {
                return false;
            }
        }
        else {
            for(int i=0; Bitrate.bitrates[i]; i++){
                if(Format->bit_rate == Bitrate.bitrates[i]){
                    return true;
                }
            }
            return false;
        }
    }

    return false;
}

bool cCodecToolKit::matchesAcceptedAudioChannels(AcceptedAudioChannels Channels, AVCodecContext* Codec){
    if(Codec){
        if(Codec->channels <= Channels.max_channels){
            if(Codec->channel_layout){
                for(int i=0; Channels.layouts[i]; i++){
                    if(Channels.supportsLFE && Codec->channel_layout == (Channels.layouts[i]|CH_LOW_FREQUENCY)){
                        return true;
                    }
                    else if(Codec->channel_layout == Channels.layouts[i]){
                        return true;
                    }
                }
            }
            else {
                return true;
            }
        }
    }

    return false;
}

bool cCodecToolKit::matchesAcceptedSamplingRates(AcceptedSamplingRates SamplingRates, AVCodecContext* Codec){
    if(Codec){
        for(int i=0; SamplingRates.rates[i]; i++){
            if(Codec->sample_rate == SamplingRates.rates[i]){
                return true;
            }
        }
    }

    return false;
}

bool cCodecToolKit::matchesAcceptedResolutions(AcceptedResolution *Resolutions, int Count, AVStream* Stream){
    if(Stream && Resolutions && Stream->codec){
        for(int i=0; i < Count; i++){
            if(     Stream->codec->width == Resolutions[i].width &&
                    Stream->codec->height == Resolutions[i].height &&
                    Stream->r_frame_rate.num == Resolutions[i].fps &&
                    Stream->r_frame_rate.den == Resolutions[i].multiplier
               ){
                return true;
            }
        }
    }

    return false;
}