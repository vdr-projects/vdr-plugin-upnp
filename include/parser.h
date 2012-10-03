/*
 * parser.h
 *
 *  Created on: 16.09.2012
 *      Author: savop
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <map>
#include <string>
#include <sstream>
#include <list>
#include "../include/tools.h"

using namespace std;

namespace upnp {


/**
 * Sort criteria
 *
 * This is a structure for sorting objects. It has a certain property and
 * a direction flag.
 */
struct SortCrit {
    string property;               ///< the Property, which shall be sorted
    bool sortDescending;           ///< sort the objects in descending order
};

//typedef std::map<const char*, const char*, strCmp> propertyMap;

/**
 * Creates a list with sort criteria
 *
 * This parser creates a list of sort criteria. It parses the sort criteria string
 * from a \em Browse or \em Search request and stores the information in a \c cSortCrit
 * structure.
 */
class cSortCriteria {
public:
  typedef list<SortCrit> SortCriteriaList;
private:
  SortCrit         mCurrentCrit;
  SortCriteriaList mCriteriaList;
  bool parseSort(string sort);
  void pushProperty(string property);
  void pushDirection(bool desc);
  SortCriteriaList getSortList() const { return this->mCriteriaList; }
  cSortCriteria();
public:
  virtual ~cSortCriteria();
  /**
   * Parses the sort criteria
   *
   * This parses the sort criteria and returns a list with valid criterias
   *
   * @return returns
   * - a list with valid sort criterias
   * - \bc null, otherwise
   */
  static SortCriteriaList parse(
      const string& sort            ///< the string container the sort criteria
  );
};

/**
 * Parses the filter criteria
 *
 * This parses the filter criteria which comes from a \em Browse or \em Search
 * request.
 */
class cFilterCriteria {
private:
    StringList mFilterList;
    cFilterCriteria();
    bool parseFilter(string filter);
    void pushProperty(const char* start, const char* end);
    void pushAsterisk(const char asterisk);
    StringList getFilterList() const { return this->mFilterList; }
public:
    virtual ~cFilterCriteria();
    /**
     * Parses the filter criteria
     *
     * This parses the filter criteria. It may be a empty string list, a \bc NULL
     * pointer or a list with properties which shall be shown in the \em DIDL-Lite fragment.
     *
     * @return the stringlist containing the filter
     */
    static StringList parse(
        const string& Filter = "*"         ///< the filter string
    );
};

/**
 * @private
 */
class cSearch {
private:
    stringstream sqlWhereStmt;
    string currentProperty;
    string currentOperator;
    string currentValue;
    cSearch();
    bool parseCriteria(string Search);
    void pushExistance (string Exists);
    void pushProperty  (string Property);
    void pushOperator  (string Operator);
    void pushConcatOp  (string Operator);
    void pushStartBrackedExp(const char);
    void pushEndBrackedExp(const char);
    void pushValue     (const char* Start, const char* End);
    void pushExpression(const char* Start, const char* End);
public:
    virtual ~cSearch();
    static string parse(const string& Search);
};

}  // namespace upnp

#endif /* PARSER_H_ */
