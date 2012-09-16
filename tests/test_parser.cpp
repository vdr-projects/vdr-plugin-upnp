/*
 * test_parser.cpp
 *
 *  Created on: 16.09.2012
 *      Author: savop
 */

#include "../include/parser.h"
#include <iostream>

void checkSearch(string t, string e) {

  string r = upnp::cSearch::parse(t);

  cout << "Suche:    \"" << t << "\"" << endl;
  cout << "Erwartet: \"" << e << "\"" << endl;
  cout << "Ergebnis: \"" << r << "\"" << endl;
  cout << "------->  " << (e.compare(r) ? "FEHLGESCHLAGEN!" : "ERFOLGREICH!") << endl << endl;

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

  return 0;
}
