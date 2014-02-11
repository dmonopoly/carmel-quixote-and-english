// Main method outputs results (quick test).

#include "TagGrammarFinder.h"

#define DO_MAIN false
#define EXTRA_PRINTING true

bool TagGrammarFinder::GetBigramTagGrammarFromOrganizedRows(
    const string &filename, map<Notation, double> *data,
    vector<string> *tag_list) {
  ifstream fin(filename.c_str());
  if (fin.fail()) {
    cerr << "Could not open file " << filename << endl;
    return false;
  } else {
    set<string> sounds;
    map<Notation, int> unigram_counts;
    map<Notation, int> bigram_counts;
    // Read bigram counts from file.
    int count;
    string sound1, sound2;
    // SUM_i C(s_i)
    Notation n_count_total(TagGrammarFinder::SIGMA + "C",
        {TagGrammarFinder::ARB_SOUND_PLACEHOLDER});
    while (true) {
      fin >> count;
      if (fin.eof())
        break;
      fin >> sound1 >> sound2;
      sounds.insert(sound1);
      sounds.insert(sound2);
      Notation n_count_seq("C", {sound1}, TagGrammarFinder::SEQ_DELIM,
                           {sound2});
      Notation n_count_seq_total(TagGrammarFinder::SIGMA + "C", {sound1},
                                 TagGrammarFinder::SEQ_DELIM,
                                 {TagGrammarFinder::ARB_SOUND_PLACEHOLDER});
      bigram_counts[n_count_seq] = count; // should only encounter once 
      bigram_counts[n_count_seq_total] += count;
      // Single probabilities. Treat C(s1) = SUM_i C(s1 s_i).
      Notation n_count("C", {sound1});  // C(s1), followed by SUM_i C(s_i).
      unigram_counts[n_count] += count;
      unigram_counts[n_count_total] += count;
    }
    fin.close();
    // Determine tag grammar probabilities.
    // Unigrams.
    for (auto s1 = sounds.begin(); s1 != sounds.end(); ++s1) {
      Notation nSingle("P", {*s1});
      Notation n_count("C", {*s1});
      try {
        (*data)[nSingle] = (double) unigram_counts.at(n_count) / 
                                  unigram_counts.at(n_count_total);
      } catch (out_of_range &e) {
        cerr << "Out of range error in unigram stuff: " << e.what() << endl;
      }
    }
    // Bigrams.
    for (auto s1 = sounds.begin(); s1 != sounds.end(); ++s1) {
      for (auto s2 = sounds.begin(); s2 != sounds.end(); ++s2) {
        Notation nGiven("P", {*s2}, TagGrammarFinder::GIVEN_DELIM, {*s1});
        Notation n_count_seq("C", {*s1}, TagGrammarFinder::SEQ_DELIM, {*s2});
        Notation n_count_seq_total(TagGrammarFinder::SIGMA + "C", {*s1},
            TagGrammarFinder::SEQ_DELIM,
            {TagGrammarFinder::ARB_SOUND_PLACEHOLDER});
        // Fixed-lambda interpolation smoothing.
        Notation n_count_s2("C", {*s2});
        double val = (double) (.9 * bigram_counts[n_count_seq] / // note the square brackets which default to 0 if not found
          bigram_counts.at(n_count_seq_total)) +
          .1 * unigram_counts.at(n_count_s2) / unigram_counts.at(n_count_total);
        // Old:
//         double val = (double) bigram_counts.at(n_count_seq) /
//           bigram_counts.at(n_count_seq_total);
        (*data)[nGiven] = val;
//         }
      }
    }
    // Pass sounds to tag_list.
    for (auto it = sounds.begin(); it != sounds.end(); ++it) {
      tag_list->push_back(*it);
    }
  }
  return true;
}

bool TagGrammarFinder::GetTrigramTagGrammarFromOrganizedRows(
    const string &filename, map<Notation, double> *data,
    vector<string> *tag_list) {
  ifstream fin(filename.c_str());
  if (fin.fail()) {
    cerr << "Could not open file " << filename << endl;
    return false;
  } else {
    set<string> sounds;
    map<Notation, int> unigram_counts;
    map<Notation, int> bigram_counts;
    map<Notation, int> trigram_counts;
    // Read trigram counts from file.
    int count;
    string sound1, sound2, sound3;
    // SUM_i C(s_i)
    Notation n_count_total(TagGrammarFinder::SIGMA + "C",
        {TagGrammarFinder::ARB_SOUND_PLACEHOLDER});
    while (true) {
      fin >> count;
      if (fin.eof())
        break;
      fin >> sound1 >> sound2 >> sound3;
      sounds.insert(sound1);
      sounds.insert(sound2);
      // Bigram. Do twice per line in counts file.
      // 1
      Notation n_bigram_count_seq("C", {sound1}, TagGrammarFinder::SEQ_DELIM,
                                  {sound2});
      Notation n_bigram_count_seq_total(TagGrammarFinder::SIGMA + "C", {sound1},
                                        TagGrammarFinder::SEQ_DELIM,
                                        {TagGrammarFinder::ARB_SOUND_PLACEHOLDER});
      bigram_counts[n_bigram_count_seq] += count;
      bigram_counts[n_bigram_count_seq_total] += count;
      // 2
      Notation n_bigram_count_seq2("C", {sound2}, TagGrammarFinder::SEQ_DELIM,
                                  {sound3});
      Notation n_bigram_count_seq_total2(TagGrammarFinder::SIGMA + "C", {sound2},
                                        TagGrammarFinder::SEQ_DELIM,
                                        {TagGrammarFinder::ARB_SOUND_PLACEHOLDER});
      bigram_counts[n_bigram_count_seq2] += count;
      bigram_counts[n_bigram_count_seq_total2] += count;

      // Trigram
      Notation n_trigram_count_seq("C", {sound1, sound2, sound3},
                                   TagGrammarFinder::SEQ_DELIM);
      Notation n_trigram_count_seq_total(TagGrammarFinder::SIGMA + "C",
                                 {sound1, sound2,
                                 TagGrammarFinder::ARB_SOUND_PLACEHOLDER},
                                 TagGrammarFinder::SEQ_DELIM);
      trigram_counts[n_trigram_count_seq] = count; // should only encounter once 
      trigram_counts[n_trigram_count_seq_total] += count;

      // Single probabilities. Treat C(s1) = SUM_i C(s1 s_i).
      Notation n_count("C", {sound1});  // C(s1), followed by SUM_i C(s_i).
      unigram_counts[n_count] += count;
      unigram_counts[n_count_total] += count;
    }
    fin.close();
    // Determine tag grammar probabilities.
    // Unigrams.
    for (auto s1 = sounds.begin(); s1 != sounds.end(); ++s1) {
      Notation nSingle("P", {*s1});
      Notation n_count("C", {*s1});
      try {
        (*data)[nSingle] = (double) unigram_counts.at(n_count) / 
                                  unigram_counts.at(n_count_total);
      } catch (out_of_range &e) {
        cerr << "Out of range error in unigram stuff: " << e.what() << endl;
      }
    }
    // Bigrams.
    double lambda2 = .9;
    for (auto s1 = sounds.begin(); s1 != sounds.end(); ++s1) {
      for (auto s2 = sounds.begin(); s2 != sounds.end(); ++s2) {
        Notation nGiven("P", {*s2}, TagGrammarFinder::GIVEN_DELIM, {*s1});
        Notation n_count_seq("C", {*s1}, TagGrammarFinder::SEQ_DELIM, {*s2});
        Notation n_count_seq_total(TagGrammarFinder::SIGMA + "C", {*s1},
            TagGrammarFinder::SEQ_DELIM,
            {TagGrammarFinder::ARB_SOUND_PLACEHOLDER});
        // Fixed-lambda interpolation smoothing.
        Notation n_count_s2("C", {*s2});
        double val = (double) (lambda2 * bigram_counts[n_count_seq] / // note the square brackets which default to 0 if not found
                     bigram_counts.at(n_count_seq_total)) +
                     (1 - lambda2) * unigram_counts.at(n_count_s2) /
                     unigram_counts.at(n_count_total);
        (*data)[nGiven] = val;
      }
    }
    // Trigrams.
    double lambda1 = .9;
    for (auto s1 = sounds.begin(); s1 != sounds.end(); ++s1) {
      for (auto s2 = sounds.begin(); s2 != sounds.end(); ++s2) {
        for (auto s3 = sounds.begin(); s3 != sounds.end(); ++s3) {
          // P(w3|w1 w2)
          Notation nGiven("P", {*s3}, TagGrammarFinder::GIVEN_DELIM, {*s1,
                            *s2});
          // C(w1 w2 w3)
          Notation cW1W2W3("C", {*s1, *s2, *s3},
                            TagGrammarFinder::SEQ_DELIM);
          // C(w1 w2)
          Notation cW1W2(TagGrammarFinder::SIGMA + "C",
              {*s1, *s2, TagGrammarFinder::ARB_SOUND_PLACEHOLDER},
              TagGrammarFinder::SEQ_DELIM);
          Notation cW2W3(TagGrammarFinder::SIGMA + "C",
              {*s2, *s3, TagGrammarFinder::ARB_SOUND_PLACEHOLDER},
              TagGrammarFinder::SEQ_DELIM);

          // C(w2), C(w3)
          Notation cW2("C", {*s2});
          Notation cW3("C", {*s3});

          double val = 0;
          if (bigram_counts.find(cW1W2) == bigram_counts.end()) {
            // Prevent divide by zero when this key does not exist.
            val = (double) (1 - lambda1) * (lambda2 * bigram_counts[cW2W3] / 
                  unigram_counts.at(cW2) + (1 - lambda2) *
                  unigram_counts.at(cW3) /
                  unigram_counts.at(n_count_total));
          } else {
            val = (double) (lambda1 * trigram_counts[cW1W2W3] / 
                bigram_counts.at(cW1W2)) +
                (1 - lambda1) * (lambda2 * bigram_counts[cW2W3] / 
                  unigram_counts.at(cW2) + (1 - lambda2) *
                  unigram_counts.at(cW3) /
                  unigram_counts.at(n_count_total));
          }
          if (isfinite(val))
            (*data)[nGiven] = val;
          else
            (*data)[nGiven] = .000000000001;
        }
      }
    }

    // Pass sounds to tag_list.
    for (auto it = sounds.begin(); it != sounds.end(); ++it) {
      tag_list->push_back(*it);
    }
  }
  return true;
}

// Quick test for bigram version.
#if DO_MAIN
int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "No filename given." << endl;
    return 0;
  } 
  string filename = argv[1];
  map<Notation, double> data;
  vector<string> tag_list;
  TagGrammarFinder::GetTagGrammarFromOrganizedRows(filename, &data, &tag_list);
  cout << "Data:\n";
  for (auto it = data.begin(); it != data.end(); ++it) {
    cout << it->first << " " << it->second << endl;
  }
  cout << "Tag list:\n";
  for (auto it = tag_list.begin(); it != tag_list.end(); ++it) {
    cout << *it << ",";
  }
  cout << endl;
  return 0;
}
#endif
