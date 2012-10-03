/*
 * parser.cpp
 *
 *  Created on: 16.09.2012
 *      Author: savop
 */

// uncomment this to enable debuging of the grammar
//#define BOOST_SPIRIT_DEBUG

#include <string>
#include "../include/parser.h"
#include "../include/plugin.h"
#include <boost/spirit/include/classic.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

using namespace std;
using namespace boost;

namespace sp = boost::spirit::classic;

namespace upnp {


// This is the standard callback function which will be overloaded
// with all the specific callbacks
typedef function2<void, const char*, const char*> expCallback;
typedef function1<void, const char*> propCallback;
typedef function1<void, const char*> opCallback;
typedef function1<void, const char> charCallback;
typedef function1<void, int> intCallback;

// The defined ColumnNames
/** @private */
struct cProperties : sp::symbols<const char*> {
  //struct cProperties : symbols<> {
  cProperties(){
    add
    (property::object::KEY_TITLE, "title")
    (property::object::KEY_CREATOR, "creator")
    (property::object::KEY_DESCRIPTION, "description")
    (property::object::KEY_LONG_DESCRIPTION, "longDescription")
    (property::object::KEY_CLASS, "class")
    (property::object::KEY_DATE, "date")
    (property::object::KEY_LANGUAGE, "language")
    (property::resource::KEY_PROTOCOL_INFO, "protocolInfo")
    ;
  }
} Properties;

/** @private */
struct cOperators : sp::symbols<const char*> {
  cOperators(){
    add
    ("=", "==")
    ("!=", "!=")
    ("<", "<")
    (">", ">")
    ("<=", "<=")
    (">=", ">=")
    ("contains", "LIKE")
    ("doesNotContain", "NOT LIKE")
    ("derivedfrom", "LIKE")
    ;
  }
} Operators;

/** @private */
struct cConcatOperators : sp::symbols<const char*> {
  cConcatOperators(){
    add
    ("and", "AND")
    ("or", "OR")
    ;
  }
} ConcatOperators;

/** @private */
struct cExistanceOperator : sp::symbols<const char*> {
  cExistanceOperator(){
    add
    ("true", "NOT NULL")
    ("false", "NULL")
    ;
  }
} Existance;

// THE GRAMMAR!
// This is the grammar including the functors which calls the member functions
// of search. The callback definitions at the end of the constructor are
// essential. DO NOT MODIFY if you don't know how!
/** @private */
struct cSearchGrammar : public sp::grammar<cSearchGrammar> {
  // The callbacks members
  charCallback &endBrackedExp;
  expCallback &pushSimpleExp;
  opCallback  &pushOperator;
  expCallback &pushQuotedValue;
  opCallback  &pushExistance;
  propCallback &pushProperty;
  opCallback &pushConcatOp;
  charCallback  &startBrackedExp;

  // Constructor with the callback functions
  cSearchGrammar(
      charCallback &endBrackedExp,
      expCallback &pushSimpleExp,
      opCallback &pushOperator,
      expCallback &pushQuotedValue,
      opCallback &pushExistance,
      propCallback &pushProperty,
      opCallback &pushConcatOp,
      charCallback  &startBrackedExp):
        endBrackedExp(endBrackedExp),
        pushSimpleExp(pushSimpleExp),
        pushOperator(pushOperator),
        pushQuotedValue(pushQuotedValue),
        pushExistance(pushExistance),
        pushProperty(pushProperty),
        pushConcatOp(pushConcatOp),
        startBrackedExp(startBrackedExp){}

  template <typename scanner>
  /** @private */
  struct definition {
    sp::rule<scanner> searchCrit, searchExp, logOp, \
    relExp, binOp, relOp, stringOp, \
    existsOp, boolVal, quotedVal, \
    wChar, property, brackedExp, exp;
    const sp::rule<scanner> &start(){
      return searchCrit;
    }
    definition(const cSearchGrammar &self){
      /*************************************************************************\
       *                                                                         *
       *                   The grammar of a UPnP search expression               *
       *                                                                         *
       * searchCrit        ::= searchExp | asterisk                              *
       *                                                                         *
       * searchExp         ::= relExp |                                          *
       *                       searchExp wChar+ logOp wChar+ searchExp |         *
       *                       '(' wChar* searchExp wChar* ')'                   *
       *                                                                         *
       * logOp             ::= 'and' | 'or'                                      *
       *                                                                         *
       * relExp            ::= property wChar+ binOp wChar+ quotedVal |          *
       *                       property wChar* existsOp wChar+ boolVal           *
       *                                                                         *
       * binOp             ::= relOp | stringOp                                  *
       *                                                                         *
       * relOp             ::= '=' | '!=' | '<' | '<=' | '>' | '>='              *
       *                                                                         *
       * stringOp          ::= 'contains' | 'doesNotContain' | 'derivedfrom'     *
       *                                                                         *
       * existsOp          ::= 'exists'                                          *
       *                                                                         *
       * boolVal           ::= 'true' | 'false'                                  *
       *                                                                         *
       * quotedVal         ::= dQuote escapedQuote dQuote                        *
       *                                                                         *
       * wChar             ::= space | hTab | lineFeed |                         *
       *                       vTab | formFeed | return                          *
       *                                                                         *
       * property          ::= See ContentDirectory Section 2.4                  *
       *                                                                         *
       * escapedQuote      ::= See ContentDirectory Section 2.3.1                *
       *                                                                         *
            \*************************************************************************/
      searchCrit  = searchExp | "*";

      searchExp   = exp >> *(+wChar >> logOp >> +wChar >> exp);
      ;

      exp         = relExp
          | brackedExp
          ;

      brackedExp  = sp::confix_p(
          sp::ch_p('(')[self.startBrackedExp],
              *wChar >> searchExp >> *wChar,
              sp::ch_p(')')[self.endBrackedExp]
      )
                              ;

      logOp       = ConcatOperators[self.pushConcatOp]
                                    ;

      relExp      = (property >> +wChar >> binOp >> +wChar >> quotedVal) [self.pushSimpleExp]
                    | (property >> +wChar >> existsOp >> +wChar >> boolVal) [self.pushSimpleExp]
                    ;

      binOp       = Operators[self.pushOperator]
                    ;

      existsOp    = sp::str_p("exists")
                    ;

      boolVal     = Existance[self.pushExistance]
                    ;

      quotedVal   = sp::confix_p('"', (*sp::c_escape_ch_p)[self.pushQuotedValue], '"')
                    ;

      wChar       = sp::space_p
                    ;

      property    = Properties[self.pushProperty]
                    ;

      // Debug mode
#ifdef BOOST_SPIRIT_DEBUG
      BOOST_SPIRIT_DEBUG_NODE(searchCrit);
      BOOST_SPIRIT_DEBUG_NODE(searchExp);
      BOOST_SPIRIT_DEBUG_NODE(logOp);
      BOOST_SPIRIT_DEBUG_NODE(relExp);
      BOOST_SPIRIT_DEBUG_NODE(binOp);
      BOOST_SPIRIT_DEBUG_NODE(relOp);
      BOOST_SPIRIT_DEBUG_NODE(stringOp);
      BOOST_SPIRIT_DEBUG_NODE(existsOp);
      BOOST_SPIRIT_DEBUG_NODE(boolVal);
      BOOST_SPIRIT_DEBUG_NODE(quotedVal);
      BOOST_SPIRIT_DEBUG_NODE(wChar);
      BOOST_SPIRIT_DEBUG_NODE(property);
#endif
    }
  };
};

/**********************************************\
 *                                              *
 *  The actors                                  *
 *                                              *
 \**********************************************/

void cSearch::pushEndBrackedExp(const char){
  sqlWhereStmt << ") ";
}

void cSearch::pushExpression(const char*, const char*){

  sqlWhereStmt << currentProperty << " " << currentOperator << " " << currentValue << " ";

  return;
}

void cSearch::pushProperty(string property){
  currentProperty = property;
}

void cSearch::pushOperator(string op){
  currentOperator = op;
}

void cSearch::pushValue(const char* start, const char* end){
  stringstream s;

  s << "'";

  if(currentOperator.find("LIKE") != string::npos)
    s << "%" << string(start, end) << "%";
  else
    s << string(start, end);

  s << "'";

  currentValue = s.str();
}

void cSearch::pushExistance(string Exists){
  currentValue = Exists;
  currentOperator = "IS";
}

void cSearch::pushConcatOp(string Operator){
  sqlWhereStmt << Operator << " ";
}

void cSearch::pushStartBrackedExp(const char){
  sqlWhereStmt << "( ";
}

/**********************************************\
 *                                              *
 *  The rest                                    *
 *                                              *
 \**********************************************/

string cSearch::parse(const string& search){
  static cSearch searchCrit;

  if(searchCrit.parseCriteria(search)){
    return searchCrit.sqlWhereStmt.str();
  }
  return string();
}

bool cSearch::parseCriteria(string search){

  sqlWhereStmt.str(string());

  charCallback endBrackedExpCB(bind(&cSearch::pushEndBrackedExp, this, _1));
  expCallback pushSimpleExpCB(bind(&cSearch::pushExpression, this, _1, _2));
  opCallback pushOperatorCB(bind(&cSearch::pushOperator, this, _1));
  expCallback pushQuotedValueCB(bind(&cSearch::pushValue, this, _1, _2));
  opCallback pushExistanceCB(bind(&cSearch::pushExistance, this, _1));
  propCallback pushPropertyCB(bind(&cSearch::pushProperty, this, _1));
  opCallback pushConcatOpCB(bind(&cSearch::pushConcatOp, this, _1));
  charCallback startBrackedExpCB(bind(&cSearch::pushStartBrackedExp, this, _1));

  // Craft the grammar
  cSearchGrammar Grammar(endBrackedExpCB,
      pushSimpleExpCB,
      pushOperatorCB,
      pushQuotedValueCB,
      pushExistanceCB,
      pushPropertyCB,
      pushConcatOpCB,
      startBrackedExpCB);

  if(!sp::parse(search.c_str(), Grammar).full){
    return false;
  }
  return true;
}

cSearch::cSearch(){}

cSearch::~cSearch(){}

/**********************************************\
 *                                              *
 *  The filter                                  *
 *                                              *
 \**********************************************/

/** @private */
struct cFilterGrammar : public sp::grammar<cFilterGrammar> {
  // The callback members
  expCallback &pushProperty;
  charCallback &pushAsterisk;

  cFilterGrammar(
      expCallback &pushProperty,
      charCallback &pushAsterisk):
        pushProperty(pushProperty),
        pushAsterisk(pushAsterisk){}

  template <typename scanner>
  /** @private */
  struct definition {
    sp::rule<scanner> filterCrit, filterExp, property, propString;

    const sp::rule<scanner> &start(){
      return filterCrit;
    }
    definition(const cFilterGrammar &self){
      filterCrit = sp::ch_p('*')[self.pushAsterisk]
                   | filterExp
                   ;

      filterExp  = property >> *(sp::ch_p(',') >> *sp::space_p >> filterExp)
                   ;

      property   = propString[self.pushProperty]
                   ;

      // TODO: improve this grammer.
      // A property has the following pattern:
      // [<namespace>:]property[@attribute] OR @id, @parentID, @restricted
      //
      propString = *(sp::alpha_p | sp::ch_p(':') | sp::ch_p('@'))
                   ;
    }
  };
};

cFilterCriteria::cFilterCriteria(){}

cFilterCriteria::~cFilterCriteria(){}

StringList cFilterCriteria::parse(const string& filter){
  static cFilterCriteria filterParser;

  StringList list;

  if(filterParser.parseFilter(filter)){
    list = filterParser.getFilterList();
  }

  return list;
}

bool cFilterCriteria::parseFilter(string filter){

  mFilterList.clear();

  mFilterList.push_back(property::object::KEY_OBJECTID);
  mFilterList.push_back(property::object::KEY_PARENTID);
  mFilterList.push_back(property::object::KEY_TITLE);
  mFilterList.push_back(property::object::KEY_CLASS);
  mFilterList.push_back(property::object::KEY_RESTRICTED);
  mFilterList.push_back(property::resource::KEY_RESOURCE);
  mFilterList.push_back(property::resource::KEY_PROTOCOL_INFO);

  if(filter.empty()){
    return true;
  }

  charCallback pushAsteriskCB(bind(&cFilterCriteria::pushAsterisk,this,_1));
  expCallback pushPropertyCB(bind(&cFilterCriteria::pushProperty,this,_1, _2));

  cFilterGrammar Grammar(pushPropertyCB, pushAsteriskCB);

  if(!sp::parse(filter.c_str(), Grammar).full){
    return false;
  }
  return true;
}

/**********************************************\
 *                                              *
 *  The actors                                  *
 *                                              *
 \**********************************************/

void cFilterCriteria::pushProperty(const char* start, const char* end){
  string s(start, end);

  for(StringList::iterator it = mFilterList.begin(); it != mFilterList.end(); ++it){
    if((*it).compare(s) == 0)
      return;
  }

  mFilterList.push_back(s);
}

void cFilterCriteria::pushAsterisk(const char){
  mFilterList.clear();
  mFilterList.push_back("*");
  return;
}

/**********************************************\
 *                                              *
 *  The sorter                                  *
 *                                              *
 \**********************************************/

/** @private */
struct cSortGrammar : public sp::grammar<cSortGrammar> {
  // The callback members
  propCallback &pushProperty;
  charCallback &pushDirection;

  cSortGrammar(
      propCallback &pushProperty,
      charCallback &pushDirection):
        pushProperty(pushProperty),
        pushDirection(pushDirection){}

  template <typename scanner>
  /** @private */
  struct definition {
    sp::rule<scanner> sortCrit, sortExp, property, direction;

    const sp::rule<scanner> &start(){
      return sortCrit;
    }
    definition(const cSortGrammar &self){
      sortCrit   = sortExp
          ;

      sortExp    = direction >> property >> *(sp::ch_p(',') >> sortExp)
                   ;

      direction  = sp::sign_p[self.pushDirection]
                   ;

      property   = Properties[self.pushProperty]
                   ;
    }
  };
};

cSortCriteria::cSortCriteria(){}

cSortCriteria::~cSortCriteria(){}

list<SortCrit> cSortCriteria::parse(const string& sort){
  static cSortCriteria sortParser;

  list<SortCrit> list;

  if(sortParser.parseSort(sort)){
    list = sortParser.getSortList();
  }

  return list;
}

bool cSortCriteria::parseSort(string sort){

  mCriteriaList.clear();

  if(sort.empty()){
    return true;
  }

  charCallback pushDirectionCB(bind(&cSortCriteria::pushDirection,this,_1));
  propCallback pushPropertyCB(bind(&cSortCriteria::pushProperty,this,_1));

  cSortGrammar Grammar(pushPropertyCB, pushDirectionCB);

  if(!sp::parse(sort.c_str(), Grammar).full){
    return false;
  }
  return true;
}

/**********************************************\
 *                                              *
 *  The actors                                  *
 *                                              *
 \**********************************************/

void cSortCriteria::pushProperty(string property){
  mCurrentCrit.property = property;
  mCriteriaList.push_back(mCurrentCrit);
  return;
}

void cSortCriteria::pushDirection(bool desc){
  mCurrentCrit.sortDescending = (desc)?true:false;
  return;
}

}  // namespace upnp
