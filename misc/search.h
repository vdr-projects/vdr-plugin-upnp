/* 
 * File:   search.h
 * Author: savop
 *
 * Created on 27. August 2009, 21:21
 */

#ifndef _SEARCH_H
#define	_SEARCH_H

#include <map>
#include <vdr/tools.h>
#include "util.h"

struct cSortCrit : public cListObject {
    const char* Property;
    bool SortDescending;
};

typedef std::map<const char*, const char*, strCmp> propertyMap;

class cPathParser {
private:
    cString             mKey;
    propertyMap         mProperties;
    int                 mSection;
    int                 mMethod;
    bool parsePath(const char* Path, int* Section, int* Method, propertyMap* Properties);
    void pushPropertyKey(const char* Start, const char* End);
    void pushPropertyValue(const char* Start, const char* End);
    void pushMethod(int Method);
    void pushSection(int Section);
    cPathParser();
public:
    virtual ~cPathParser();
    static bool parse(const char* Path, int* Section, int* Method, propertyMap* Properties);
};

class cSortCriteria {
private:
    cSortCrit*        mCurrentCrit;
    cList<cSortCrit>* mCriteriaList;
    bool parseSort(const char* Sort);
    void pushProperty(const char* Property);
    void pushDirection(const char Direction);
    cList<cSortCrit>* getSortList() const { return this->mCriteriaList; }
    cSortCriteria();
public:
    virtual ~cSortCriteria();
    static cList<cSortCrit>* parse(const char* Sort);
};

class cFilterCriteria {
private:
    cStringList* mFilterList;
    cFilterCriteria();
    bool parseFilter(const char* Filter);
    void pushProperty(const char* Property);
    void pushAsterisk(const char Asterisk);
    cStringList* getFilterList() const { return this->mFilterList; }
public:
    virtual ~cFilterCriteria();
    static cStringList* parse(const char* Filter);
};

class cSearch {
private:
    char* SQLWhereStmt;
    const char* CurrentProperty;
    const char* CurrentOperator;
    const char* CurrentValue;
    static cSearch* mInstance;
    cSearch();
    bool parseCriteria(const char* Search);
    void pushExistance (const char* Exists);
    void pushProperty  (const char* Property);
    void pushOperator  (const char* Operator);
    void pushConcatOp  (const char* Operator);
    void pushStartBrackedExp(const char);
    void pushEndBrackedExp(const char);
    void pushValue     (const char* Start, const char* End);
    void pushExpression(const char* Start, const char* End);
public:
    virtual ~cSearch();
    static const char* parse(const char* Search);
};


#endif	/* _SEARCH_H */

