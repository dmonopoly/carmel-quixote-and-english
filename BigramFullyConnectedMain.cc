// Generates the WFSA based on the given bigram counts file and the FST using
// the given cypher and the counts file.

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include "EMViterbiPackage/Notation.h"
#include "TagGrammarFinder.h"
#include "CypherReader.h"

using namespace std;

const string WFSA_FILE = "cipher.wfsa";
const string FST_FILE = "cipher.fst";
const string EMPTY = "*e*";

void WriteLine(ofstream &fout, const string &node1, const string &node2,
    const string &first, const string &second, double prob, const string &end) {
  string input = first; // x in x:y
  string output = second; // y in x:y
  if (first != EMPTY) {
    stringstream ss;
    ss << "\"" << first << "\"";
    input = ss.str();
  }
  if (second != EMPTY) {
    stringstream ss;
    ss << "\"" << second << "\"";
    output = ss.str();
  }
  fout << "(" << node1 << " (" << node2 << " " << input << " " << output << " " 
    << prob << end << "))" << endl;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "Usage: ./<exec> <bigram-counts-file> <cyphertext>" << endl;
    return 0;
  }
  string filename_for_bigrams = argv[1];
  // Get LM data for WFSA.
  map<Notation, double> data;  // Storage for log probabilities and counts.
  vector<string> tag_list;
  bool found = TagGrammarFinder::GetBigramTagGrammarFromOrganizedRows(
      filename_for_bigrams, &data, &tag_list);
  if (!found) {
    cerr << "Error getting tag grammar." << endl;
    return 0;
  }

  // Get observed data for WFST.
  string filename_for_cypher = argv[2];
  vector<string> observed_data;
  set<string> obs_symbols;
  bool got_obs_data = CypherReader::GetObservedData(filename_for_cypher,
                                                    &observed_data,
                                                    &obs_symbols);
  if (!got_obs_data) {
    cerr << "Error getting observed data." << endl;
    return 0;
  }

  // Begin writing out the WFSA.
  // Every tag is a node which goes to every other tag-node (bigram). START goes
  // to every tag (unigram). Every tag goes back to START also (to allow unigram
  // usage again), and also to END.
  ofstream fout;
  fout.open(WFSA_FILE.c_str());
  fout << "END" << endl;
  double prob_to_end = .00001;
  // Unigram probs.
  for (auto s : tag_list) {
    string node_name = s;
    Notation n("P", {s});
    try {
      double prob = data.at(n);
      WriteLine(fout, "START", node_name, EMPTY, s, prob, "!");
      // End probs.
      WriteLine(fout, node_name, "END", EMPTY, EMPTY, prob_to_end, "!");
    } catch (out_of_range &e) {
      cerr << "Out of range error for notation " << n << "; " << e.what() <<
        endl;
      exit(0);
    }
  }
  // Bigram probs.
  for (auto s1 : tag_list) {
    for (auto s2 : tag_list) {
      Notation n("P", {s2}, TagGrammarFinder::GIVEN_DELIM, {s1});
      string node1_name = s1;
      string node2_name = s2;
      try {
        double prob = data.at(n);
        WriteLine(fout, node1_name, node2_name, EMPTY, s2, prob, "!");
      } catch (out_of_range &e) {
        cerr << "Out of range error for notation " << n << "; " << e.what() <<
          endl;
        exit(0);
      }
    }
  }
  fout.close();

  // Begin writing the FST.
  fout.open(FST_FILE.c_str());
  fout << 0 << endl;
  // Ad hoc: Assume _ is present for space. Deal with it separately from the
  // loop.
//   obs_symbols.erase(obs_symbols.find("_"));
  fout << "(0 (0 \"_\" \"_\"))" << endl;
  for (auto tag : tag_list) {
    for (auto obs : obs_symbols) {
      fout << "(0 (0 \"" << tag << "\" \"" << obs << "\"))" << endl;
    }
  }
  fout.close();
  cout << "Written to " << WFSA_FILE << endl;
  cout << "Written to " << FST_FILE << endl;
  return 0;
}
