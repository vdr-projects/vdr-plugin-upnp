/* 
 * File:   recplayer.cpp
 * Author: savop
 * 
 * Created on 8. Juni 2009, 11:57
 */

#include <stdio.h>
#include <fcntl.h>
#include <vdr/recording.h>
#include <vdr/tools.h>
#include "recplayer.h"

cRecordingPlayer *cRecordingPlayer::newInstance(cRecording* Recording){
    if(Recording->IsPesRecording()){
        ERROR("Sorry, but only TS is supported, yet!");
        return NULL;
    }

    cRecordingPlayer *Player = new cRecordingPlayer(Recording);
    return Player;
}
cRecordingPlayer::~cRecordingPlayer() {
    delete this->mOffsets;
}

cRecordingPlayer::cRecordingPlayer(cRecording *Recording) {
    MESSAGE(VERBOSE_SDK, "Created Recplayer");
    this->mFile      = NULL;
    this->mTotalLenght = 0;
    this->mRecording = Recording;
    this->mOffsets   = new off_t[VDR_MAX_FILES_PER_RECORDING];
    this->mOffset    = 0;
    this->mIndex     = 1;
    
}

void cRecordingPlayer::open(UpnpOpenFileMode){
    this->Scan();
}

void cRecordingPlayer::close(){
    delete [] this->mOffsets;
    if(this->mFile) fclose(this->mFile);
}

int cRecordingPlayer::write(char*, size_t){
    ERROR("Writing not allowed on recordings");
    return 0;
}

int cRecordingPlayer::read(char* buf, size_t buflen){
    FILE *File;
    off_t fileEndOffset = this->mOffsets[this->mIndex];
    if(this->mOffset > fileEndOffset){
        File = this->NextFile();
    }
    else {
        File = this->GetFile();
    }
                         // do not read more bytes than the actual file has
    size_t bytesToRead = ((fileEndOffset - this->mOffset) < (off_t)buflen)? fileEndOffset - this->mOffset : buflen;
    size_t bytesRead = fread((char*)buf, sizeof(char), bytesToRead, File);

    this->mOffset += (off_t)bytesRead;
    
    return (int)bytesRead;
}

int cRecordingPlayer::seek(off_t offset, int origin){
    // Calculate the new offset
    switch(origin){
        case SEEK_CUR:
            if(this->mOffset + offset > this->mTotalLenght){
                ERROR("Can't read behind end of file!");
                return -1;
            }
            this->mOffset += (off_t)offset;
            break;
        case SEEK_END:
            if(offset > 0){
                ERROR("Can't read behind end of file!");
                return -1;
            }
            this->mOffset = this->mTotalLenght + offset;
            break;
        case SEEK_SET:
            if(offset > this->mTotalLenght){
                ERROR("Can't read behind end of file!");
                return -1;
            }
            this->mOffset = (off_t)offset;
            break;
        default:
            ERROR("Unknown seek mode (%d)!", origin);
            return -1;
    }
    // Seek to the very first file;
    this->SeekInFile(1,0);
    off_t fileEndOffset = this->mOffsets[this->mIndex];
    // Spin until the new offset is in range of a specific file
    while(this->mOffset > (fileEndOffset = this->mOffsets[this->mIndex])){
        // If its not possible to switch to next file, there was an error
        if(!this->NextFile()){
            ERROR("Offset %ld not in the range of a file!", offset);
            return -1;
        }
    }
    off_t relativeOffset =
            this->mOffset - (this->mOffsets[this->mIndex-1])
                            ? this->mOffsets[this->mIndex-1]
                            : 0;
    if(!this->SeekInFile(this->mIndex, relativeOffset)){
        ERROR("Cannot set offset!");
        return -1;
    }
    return 0;
}

void cRecordingPlayer::Scan(){
    MESSAGE(VERBOSE_RECORDS, "Scanning video files...");
    // Reset the offsets
    int i = 1;
    while(this->mOffsets[i++]) this->mOffsets[i] = 0;
    MESSAGE(VERBOSE_RECORDS, "Offsets reseted.");

    i = 0;
    FILE *File;
    while((File = this->GetFile(i))){
        if(VDR_MAX_FILES_PER_RECORDING < i+1){
            ERROR("Maximum file offsets exceeded!");
            break;
        }
        fseek(File, 0, SEEK_END);
        off_t offset = ftell(File);
        MESSAGE(VERBOSE_RECORDS, "File %d has its last offset at %ld", i, offset);
        this->mOffsets[i+1] = this->mOffsets[i] + offset;
        this->mTotalLenght  = this->mOffsets[i+1];
        i++;
    }
}

FILE *cRecordingPlayer::GetFile(int Index){
    // No Index given: set current index
    if(Index == 0) Index = this->mIndex;
    // Index not changed: return current file
    if(this->mIndex == Index && this->mFile) return this->mFile;

    // Index changed: close open file and open new file
    if(this->mFile) fclose(this->mFile);
    char *filename = new char[VDR_FILENAME_BUFSIZE];
    snprintf(filename, VDR_FILENAME_BUFSIZE, VDR_RECORDFILE_PATTERN_TS, this->mRecording->FileName(), Index );
    MESSAGE(VERBOSE_BUFFERS, "Filename: %s", filename);
    this->mFile = NULL;
    if(this->mFile = fopen(filename, "r")){
        this->mIndex = Index;
        return this->mFile;
    }
    return NULL;
}

FILE *cRecordingPlayer::NextFile(void){
    return this->GetFile(this->mIndex++);
}

int cRecordingPlayer::SeekInFile(int Index, off_t Offset){
    FILE *File = this->GetFile(Index);
    fseek(File, Offset, SEEK_SET);
    return ftell(File);
}

