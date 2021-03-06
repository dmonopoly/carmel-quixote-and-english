#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include "EMViterbiPackage/Notation.h"
#include "TagGrammarFinderSparse.h"
#include "CypherReader.h"

using namespace std;

const string WFSA_FILE = "cipher.wfsa";
const string FST_FILE = "cipher.fst";
const string EMPTY = "*e*";
const string NODE_NAME_DELIM = "-";
const double PROB_TO_END = .00001;

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
  bool found = TagGrammarFinderSparse::GetTrigramTagGrammarFromOrganizedRows(
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

  // Begin writing out the WFSA, sparsely connected version.
  ofstream fout;
  fout.open(WFSA_FILE.c_str());
  fout << "END" << endl;
  double lambda2 = .9;
  // Unigram probs: START->x
  for (auto s : tag_list) {
    string node_name = s;
    string node_name_sharp = node_name + "#";
    Notation n("P", {s});
    try {
      double prob = data.at(n);
      WriteLine(fout, "START", node_name, EMPTY, s, prob, "!");
      // x->START
      WriteLine(fout, node_name, "START", EMPTY, EMPTY, 1 - lambda2, "!");
      // x->x#
      WriteLine(fout, node_name, node_name_sharp, EMPTY, EMPTY, lambda2, "!");
      // x#->START
      WriteLine(fout, node_name_sharp, "START", EMPTY, EMPTY, .0001, "!");
      // x->END
      WriteLine(fout, node_name, "END", EMPTY, EMPTY, .0001, "!");
      // x#->END
      WriteLine(fout, node_name_sharp, "END", EMPTY, EMPTY, .0001, "!");
    } catch (out_of_range &e) {
      cerr << "Out of range error for notation " << n << "; " << e.what() <<
        endl;
      exit(0);
    }
  }
  // Bigram probs.
  double lambda1 = .9;
  for (auto s1 : tag_list) {
    for (auto s2 : tag_list) {
      Notation n("P", {s2}, TagGrammarFinderSparse::GIVEN_DELIM, {s1});
      string node1_name = s1;
      string node1_name_sharp = s1 + "#";
      string node2_name = s1 + NODE_NAME_DELIM + s2;
      // Trigram prep things.
      string node1a_name = s1 + NODE_NAME_DELIM + s2;
      string node1a_name_sharp = s1 + NODE_NAME_DELIM + s2 + "#";
      try {
        double prob = data.at(n);
        // x#->xy
        WriteLine(fout, node1_name_sharp, node2_name, EMPTY, s2, prob, "!");

        // Trigram preparation.
        // xy->xy#
        WriteLine(fout, node1a_name, node1a_name_sharp, EMPTY, EMPTY, lambda1, "!");
        // xy->y
        WriteLine(fout, node1a_name, s2, EMPTY, EMPTY, 1 - lambda1, "!");
        // xy->END
        WriteLine(fout, node1a_name, "END", EMPTY, EMPTY, .0001, "!");
        // xy#->END
        WriteLine(fout, node1a_name_sharp, "END", EMPTY, EMPTY, .0001, "!");
        // xy->START
        WriteLine(fout, node1a_name, "START", EMPTY, EMPTY, .0001, "!");
        // xy#->START
        WriteLine(fout, node1a_name_sharp, "START", EMPTY, EMPTY, .0001, "!");
      } catch (out_of_range &e) {
        cerr << "Out of range error for notation " << n << "; " << e.what() <<
          endl;
        exit(0);
      }
    }
  }
  // Trigram probs.
  for (auto s1 : tag_list) {
    for (auto s2 : tag_list) {
      for (auto s3 : tag_list) {
        Notation n("P", {s3}, TagGrammarFinderSparse::GIVEN_DELIM, {s1, s2});
        string node1_name_sharp = s1 + NODE_NAME_DELIM + s2 + "#";
        string node2_name = s2 + NODE_NAME_DELIM + s3;
        try {
          double prob = data.at(n);
          // Trigram prob path, if prob is available.
          if (prob != 0)
            WriteLine(fout, node1_name_sharp, node2_name, EMPTY, s3, prob, "!");
        } catch (out_of_range &e) {
          // We expect some probabilities to be nonexistent. In these cases,
          // reduce the number of edges.
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
  obs_symbols.erase(obs_symbols.find("_"));
  for (int i = 0; i < tag_list.size(); ++i) {
    if (tag_list[i] == "_") {
      tag_list.erase(tag_list.begin() + i);
      break;
    }
  }
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
