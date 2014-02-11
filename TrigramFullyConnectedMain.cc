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
const string NODE_NAME_DELIM = "-";

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
    cerr << "Usage: ./<exec> <trigram-counts-file> <cyphertext>" << endl;
    return 0;
  }
  string filename_for_bigrams = argv[1];
  // Get LM data for WFSA.
  map<Notation, double> data;  // Storage for log probabilities and counts.
  vector<string> tag_list;
  bool found = TagGrammarFinder::GetTrigramTagGrammarFromOrganizedRows(
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

  // Begin writing out the WFSA, fully connected version.
  // START goes to every tag (unigram). Every tag goes back to START also (to
  // allow unigram usage again), and also to END.
  ofstream fout;
  fout.open(WFSA_FILE.c_str());
  fout << "END" << endl;
  // Create links to end. Assign a uniform small probability for each. Make sure
  // to take a bit off the unigram probabilities accordingly so the sum of all
  // probabilities exiting a node is 1.
  double prob_to_end = .0001;
  double small_compensation = prob_to_end / tag_list.size();
  // Unigram probs: START->x
  WriteLine(fout, "START", "END", EMPTY, EMPTY, prob_to_end, "!");
  for (auto s : tag_list) {
    string node_name = s;
    Notation n("P", {s});
    try {
      double prob = data.at(n) - small_compensation;
      WriteLine(fout, "START", node_name, EMPTY, s, prob, "!");
      WriteLine(fout, node_name, "START", EMPTY, EMPTY, .0001, "!"); // Sometimes go back for unigram prob again.
    } catch (out_of_range &e) {
      cerr << "Out of range error for notation " << n << "; " << e.what() <<
        endl;
      exit(0);
    }
  }
  // Bigram probs. x->xy
  for (auto s1 : tag_list) {
    for (auto s2 : tag_list) {
      Notation n("P", {s2}, TagGrammarFinder::GIVEN_DELIM, {s1});
      string node1_name = s1;
      // Middle separator prevents "r" & "rr" name clash.
      string node2_name = node1_name + NODE_NAME_DELIM + s2;
      try {
        double prob = data.at(n);
        WriteLine(fout, node1_name, node2_name, EMPTY, s2, prob, "!");
        // Reset to START edge. Sometimes go back to unigram.
        WriteLine(fout, node2_name, "START", EMPTY, EMPTY, .0001, "!");
        // Sometimes finish.
        WriteLine(fout, node2_name, "END", EMPTY, EMPTY, .0001, "!");
      } catch (out_of_range &e) {
        cerr << "Out of range error for notation " << n << "; " << e.what() <<
          endl;
        exit(0);
      }
    }
  }
  // Trigram probs. xy->yz, xy->z TODO
  for (auto s1 : tag_list) {
    for (auto s2 : tag_list) {
      for (auto s3 : tag_list) {
        Notation n("P", {s3}, TagGrammarFinder::GIVEN_DELIM, {s1, s2});
        string node1_name = s1 + NODE_NAME_DELIM + s2;
        string node2_name = s2 + NODE_NAME_DELIM + s3;
        try {
          double prob = data.at(n);
          // Trigram prob path.
          WriteLine(fout, node1_name, node2_name, EMPTY, s3, prob, "!");
        } catch (out_of_range &e) {
          cerr << "Out of range error for notation " << n << "; " << e.what() <<
            endl;
          exit(0);
        }
        // Back-to-bigram edge.
        Notation n2("P", {s3}, TagGrammarFinder::GIVEN_DELIM, {s2});
        node2_name = s3;
        try {
          double prob = data.at(n2);
          WriteLine(fout, node1_name, node2_name, EMPTY, s3, prob, "!");
        } catch (out_of_range &e) {
          cerr << "Out of range error for notation " << n << "; " << e.what() <<
            endl;
          exit(0);
        }
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
