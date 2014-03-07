// Takes two answer keys and compares symbol by symbol, outputting a count for
// differences.
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

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "Usage: ./<exec> <answer-key> <other-answer>" << endl;
    return 0;
  }
  string filename_for_key = argv[1];
  vector<string> answer_key_data;
  set<string> obs_symbols;
  bool got_key = CypherReader::GetObservedData(filename_for_key,
                                               &answer_key_data,
                                               &obs_symbols);
  if (!got_key) {
    cerr << "Error getting answer key observed data." << endl;
    return 0;
  }
  string filename_for_other = argv[2];
  vector<string> other_answer;
  bool got_obs_data = CypherReader::GetObservedData(filename_for_other,
                                                    &other_answer,
                                                    &obs_symbols);
  if (!got_obs_data) {
    cerr << "Error getting other answer." << endl;
    return 0;
  }
  // Compare the two answers. Both answers should have the same length.
  if (answer_key_data.size() != other_answer.size()) {
    cerr << "Warning: the two answers are not the same length.\n";
  }
  unsigned int num_dif = 0;
  for (unsigned int i = 0; i < answer_key_data.size(); ++i) {
    if (answer_key_data[i] != other_answer[i])
      ++num_dif;
  }
  cout << "Number of errors: " << num_dif << endl;
  cout << "Total # symbols: " << answer_key_data.size() << endl;
  cout << "Percentage correct: " << (double) (answer_key_data.size() - num_dif) /
                                    answer_key_data.size() * 100 << "%\n";
  return 0;
}

