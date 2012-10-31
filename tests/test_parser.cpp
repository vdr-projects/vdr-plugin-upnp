/*
 * test_parser.cpp
 *
 *  Created on: 16.09.2012
 *      Author: savop
 */

#include "../include/parser.h"
#include <iostream>

void checkSort(string s){

  upnp::cSortCriteria::SortCriteriaList list = upnp::cSortCriteria::parse(s);

  cout << "Sort:    \"" << s << "\"" << endl;
  for(upnp::cSortCriteria::SortCriteriaList::iterator it = list.begin(); it != list.end(); ++it){
    cout << "  " << (*it).property << " " <<((*it).sortDescending ? "DESC" : "ASC") << endl;
  }
  cout << endl;

}

void checkSearch(string t, string e) {

  string r = upnp::cSearch::parse(t);

  cout << "Suche:    \"" << t << "\"" << endl;
  cout << "Erwartet: \"" << e << "\"" << endl;
  cout << "Ergebnis: \"" << r << "\"" << endl;
  cout << "------->  " << (e.compare(r) ? "FEHLGESCHLAGEN!" : "ERFOLGREICH!") << endl << endl;

}

void checkFilter(string f){

  upnp::StringList list = upnp::cFilterCriteria::parse(f);

  cout << "Filter:  \"" << f << "\"" << endl;
  for(upnp::StringList::iterator it = list.begin(); it != list.end(); ++it){
    cout << "  " << (*it) << endl;
  }
  cout << endl;

}

int main(){
  checkSearch("upnp:class = \"object.item.imageItem\" and ( dc:date >= \"2001-10-01\" and dc:date <= \"2001-10-31\" )",
              "class == 'object.item.imageItem' AND ( date >= '2001-10-01' AND date <= '2001-10-31' ) ");

  checkSearch("@id = \"20\"",
              string());

  checkSearch("dc:title contains \"Christmas\"",
              "title LIKE '%Christmas%' ");

  checkSearch("upnp:class derivedfrom \"object.container.album\"",
              "class LIKE '%object.container.album%' ");

  checkFilter("@id,@parentID,@restricted,dc:title");

  checkFilter("*");

  checkFilter("@id,dc:title,upnp:longDescription,res");

  checkFilter("@id,@parentID,@restricted,dc:title,upnp:class,res,dc:date,@childCount,sec:CaptionInfo,sec:CaptionInfoEx,sec:dcmInfo,sec:MetaFileInfo,res@resolution,res@size,upnp:genre,dc:date,upnp:album,res@duration,upnp:albumArtURI,res@bitrate,dc:creator");

  checkFilter("dc:title,av:mediaClass,dc:date,@childCount,res,upnp:rating,upnp:rating@type,upnp:class,res@resolution,res@av:mpfEntries,upnp:album,upnp:genre,upnp:albumArtURI,upnp:albumArtURI@dlna:profileID,dc:creator,res@size,res@duration,res@bitrate,res@protocolInfo");

  checkSort("");

  checkSort("+dc:title,-dc:creator");

  checkSort("dc:title");

  checkSort("+upnp:class");

  checkSort("+@id");

  checkSort("-dc:date");

  return 0;
}
