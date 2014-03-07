// NOT USED, INCOMPLETE
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>

#include "CypherReader.h"

#define FORMAT_HAS_QUOTES true
#define QUOTE "\""
#define C_VAR "C"
#define V_VAR "V"
#define SPACE_VAR "_"

void BuildKey(map<string, string> *key) {
  (*key)["S"] = SPACE_VAR;
  (*key)["T"] = SPACE_VAR;
  (*key)["U"] = SPACE_VAR;
  (*key)["V"] = SPACE_VAR;
  (*key)["W"] = SPACE_VAR;
  (*key)["X"] = SPACE_VAR;
  (*key)["Y"] = SPACE_VAR;
  (*key)["Z"] = SPACE_VAR;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "Usage: ./<exec> <copiale-cyphertext>" << endl;
    return 0;
  }
  string filename_for_cypher = argv[1];
  vector<string> observed_data;
  set<string> obs_symbols;
  bool got_obs_data = CypherReader::GetObservedData(filename_for_cypher,
                                                    &observed_data,
                                                    &obs_symbols);
  if (!got_obs_data) {
    cerr << "Error getting observed data." << endl;
    return 0;
  }
  map<string, string> key;
  BuildKey(&key);
  ifstream fin(filename_for_cypher.c_str());
  if (fin.fail()) {
    cerr << "Could not open file " << filename_for_cypher << endl;
    return 0;
  }
  string line;
  while (getline(fin, line)) {
    stringstream ss; ss << line;
    string word;
    vector<string> words;
//     cout << "line: " << line << endl;
    if (!line.empty()) {
      while (ss >> word) {
        if (FORMAT_HAS_QUOTES)
          word = word.substr(1, word.size() - 2);  // Remove quotes.
        words.push_back(word);
      }
      for (unsigned int i = 0; i < words.size() - 1; ++i) {
        try {
          cout << QUOTE << key.at(words[i]) << QUOTE << " ";
        } catch (out_of_range &e) {
          cerr << "Error for word " << words[i] << "; " << e.what() << endl;
          exit(0);
        }
      }
      try {
        cout << QUOTE << key.at(words.back()) << QUOTE << endl;
      } catch (out_of_range &e) {
        cerr << "Error for word " << words.back() << "; " << e.what() << endl;
        exit(0);
      }
    }
    if (fin.eof())
      break;
  }
  fin.close();
  return 0;
}
