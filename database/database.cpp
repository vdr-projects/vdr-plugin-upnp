/*
 * File:   database.h
 * Author: savop
 *
 * Created on 3. September 2009, 22:20
 */

#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "database.h"
#include "../common.h"
#include "object.h"
#include "../upnp.h"

cSQLiteDatabase* cSQLiteDatabase::mInstance = NULL;

cSQLiteDatabase::cSQLiteDatabase(){
    this->mActiveTransaction = false;
    this->mDatabase = NULL;
    this->mLastRow = NULL;
    this->mRows = NULL;
}

cSQLiteDatabase::~cSQLiteDatabase(){
    sqlite3_close(this->mDatabase);
}

cSQLiteDatabase* cSQLiteDatabase::getInstance(){
    if(cSQLiteDatabase::mInstance == NULL){
        cSQLiteDatabase::mInstance = new cSQLiteDatabase;
        DatabaseLocker.Wait();
        cSQLiteDatabase::mInstance->initialize();
    }

    if(cSQLiteDatabase::mInstance != NULL)
        return cSQLiteDatabase::mInstance;
    else
        return NULL;
}

int cSQLiteDatabase::exec(const char* Statement){
    char* Error;
    if(!this->mDatabase){
        ERROR("Database not open. Cannot continue");
        return -1;
    }
    this->mRows = new cRows;
    MESSAGE(VERBOSE_SQL_STATEMENTS,"SQLite: %s", Statement);
    if(sqlite3_exec(this->mDatabase, Statement, cSQLiteDatabase::getResultRow, (cSQLiteDatabase*)this, &Error)!=SQLITE_OK){
        ERROR("Database error: %s", Error);
        ERROR("Statement was: %s", Statement);
        delete this->mRows; this->mRows = NULL;
        sqlite3_free(Error);
        return -1;
    }

    sqlite3_free(Error);
    return 0;
}

const char* cSQLiteDatabase::sprintf(const char* Format, ...){
    va_list vlist;
    va_start(vlist, Format);
    char* SQLStatement = sqlite3_vmprintf(Format, vlist);
    va_end(vlist);
    return SQLStatement;
}

int cSQLiteDatabase::execStatement(const char* Statement, ...){
    va_list vlist;
    va_start(vlist, Statement);
    char* SQLStatement = sqlite3_vmprintf(Statement, vlist);
    va_end(vlist);
    int ret = this->exec(SQLStatement);
    sqlite3_free(SQLStatement);
    return ret;
}

int cSQLiteDatabase::getResultRow(void* DB, int NumCols, char** Values, char** ColNames){
    cRow* Row = new cRow;
    Row->ColCount = NumCols;
    Row->Columns = new char*[NumCols];
    Row->Values = new char*[NumCols];
    for(int i=0; i < NumCols; i++){
        Row->Columns[i] = strdup0(ColNames[i]);
        Row->Values[i] = strdup0(Values[i]);
    }
    cSQLiteDatabase* Database = (cSQLiteDatabase*)DB;
    Database->mRows->Add(Row);
    return 0;
}

cRows::cRows(){
    this->mLastRow = NULL;
}

cRows::~cRows(){
    this->mLastRow = NULL;
}

bool cRows::fetchRow(cRow** Row){
    if(this->mLastRow==NULL){
        this->mLastRow = this->First();
    }
    else {
        this->mLastRow = this->Next(this->mLastRow);
    }
    if(this->mLastRow != NULL){
        *Row = this->mLastRow;
        return true;
    }
    else {
        *Row = NULL;
        return false;
    }
    return false;
}

cRow::cRow(){
    this->currentCol = 0;
    this->ColCount = 0;
    this->Columns = NULL;
    this->Values = NULL;
}

cRow::~cRow(){
    delete [] this->Columns;
    delete [] this->Values;
    this->Columns = NULL;
    this->Values = NULL;
}

bool cRow::fetchColumn(cString* Column, cString* Value){
    char *Col, *Val;
    bool ret = this->fetchColumn(&Col, &Val);
    if(ret){
        *Column = cString(Col,true);
        *Value = cString(Val,true);
    }
    return ret;
}


bool cRow::fetchColumn(char** Column, char** Value){
    if(currentCol>=this->ColCount){
        return false;
    }
    MESSAGE(VERBOSE_SQL_FETCHES,"Fetching column %s='%s' (%d/%d)", this->Columns[currentCol], this->Values[currentCol], currentCol+1, this->ColCount);
    *Column = strdup0(this->Columns[currentCol]);
    if(this->Values[currentCol]){
        *Value = strcasecmp(this->Values[currentCol],"NULL")?strdup(this->Values[currentCol]):NULL;
    }
    else {
        *Value = NULL;
    }
    currentCol++;
    return true;
}

int cSQLiteDatabase::initialize(){
    int ret;
    cString File = cString::sprintf("%s/%s", cPluginUpnp::getConfigDirectory(), SQLITE_DB_FILE);
    if((ret = sqlite3_open(File, &this->mDatabase))){
        ERROR("Unable to open database file %s (Error code: %d)!", *File, ret);
        sqlite3_close(this->mDatabase);
        return -1;
    }
    MESSAGE(VERBOSE_SDK,"Database file %s opened.", *File);
    if(this->initializeTables()){
        ERROR("Error while creating tables");
        return -1;
    }
    else if(this->initializeTriggers()){
        ERROR("Error while setting triggers");
        return -1;
    }
    return 0;
}

void cSQLiteDatabase::startTransaction(){
    if(this->mActiveTransaction){
        if(this->mAutoCommit){
            this->commitTransaction();
        }
        else {
            this->rollbackTransaction();
        }
    }
    this->execStatement("BEGIN TRANSACTION");
    MESSAGE(VERBOSE_SQL,"Start new transaction");
    this->mActiveTransaction = true;
}

void cSQLiteDatabase::commitTransaction(){
    this->execStatement("COMMIT TRANSACTION");
    MESSAGE(VERBOSE_SQL,"Commited transaction");
    this->mActiveTransaction = false;
}

void cSQLiteDatabase::rollbackTransaction(){
    this->execStatement("ROLLBACK TRANSACTION");
    MESSAGE(VERBOSE_SQL,"Rolled back transaction");
    this->mActiveTransaction = false;
}

int cSQLiteDatabase::initializeTables(){
    int ret = 0;
    this->startTransaction();
    if(this->execStatement(SQLITE_CREATE_TABLE_ITEMFINDER)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_SYSTEM)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_PRIMARY_KEYS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_ALBUMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_AUDIOBROADCASTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_AUDIOITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_CONTAINER)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_IMAGEITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_ITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_MOVIES)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_OBJECTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_PHOTOS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_PLAYLISTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_RESOURCES)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_SEARCHCLASS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_VIDEOBROADCASTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_CREATE_TABLE_VIDEOITEMS)==-1) ret = -1;
    if(ret){
        this->rollbackTransaction();
    }
    else {
        this->commitTransaction();
    }
    return ret;
}

int cSQLiteDatabase::initializeTriggers(){
    int ret = 0;
    this->startTransaction();
    if(this->execStatement(SQLITE_TRIGGER_D_OBJECTS_ITEMFINDER)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_UPDATE_SYSTEM)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_UPDATE_OBJECTID)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_AUDIOITEMS_AUDIOBROADCASTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_CONTAINERS_ALBUMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_CONTAINERS_PLAYLISTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_CONTAINERS_SEARCHCLASSES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_IMAGEITEMS_PHOTOS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_ITEMS_AUDIOITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_ITEMS_IMAGEITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_ITEMS_ITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_ITEMS_VIDEOITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_OBJECTS_OBJECTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_OBJECT_CONTAINERS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_OBJECT_ITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_OBJECT_RESOURCES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_VIDEOITEMS_MOVIES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_D_VIDEOITEMS_VIDEOBROADCASTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_AUDIOITEMS_AUDIOBROADCASTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_CONTAINERS_ALBUMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_CONTAINERS_PLAYLISTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_CONTAINERS_SEARCHCLASSES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_IMAGEITEMS_PHOTOS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_ITEMS_AUDIOITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_ITEMS_IMAGEITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_ITEMS_ITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_ITEMS_VIDEOITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_OBJECTS_OBJECTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_OBJECT_CONTAINERS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_OBJECT_ITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_OBJECT_RESOURCES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_VIDEOITEMS_MOVIES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_I_VIDEOITEMS_VIDEOBROADCASTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_AUDIOITEMS_AUDIOBROADCASTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_CONTAINERS_ALBUMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_CONTAINERS_PLAYLISTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_CONTAINERS_SEARCHCLASSES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_IMAGEITEMS_PHOTOS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_ITEMS_AUDIOITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_ITEMS_IMAGEITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_ITEMS_ITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_ITEMS_VIDEOITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_OBJECTS_OBJECTS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_OBJECT_CONTAINERS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_OBJECT_ITEMS)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_OBJECT_RESOURCES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_VIDEOITEMS_MOVIES)==-1) ret = -1;
    if(this->execStatement(SQLITE_TRIGGER_U_VIDEOITEMS_VIDEOBROADCASTS)==-1) ret = -1;
    if(ret){
        this->rollbackTransaction();
    }
    else {
        this->commitTransaction();
    }
    return ret;
}

long cSQLiteDatabase::getLastInsertRowID() const {
    return (long)sqlite3_last_insert_rowid(this->mDatabase);
}