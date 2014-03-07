# cipher_file='Ciphers/spanish.written_short.quoted.txt'
cipher_file='Ciphers/spanish.written_one_third.txt'
clang++ -stdlib=libc++ -std=c++11 TrigramSparseMain.cc EMViterbiPackage/Notation.cc CypherReader.cc TagGrammarFinder.cc
./a.out LanguageModels/corpus.spanish.trigrams.txt $cipher_file
carmel --train-cascade -HJ -X .99999 $cipher_file cipher.wfsa cipher.fst 
carmel --project-right --project-identity-fsa -HJ cipher.wfsa > cipher.wfsa.noe # cipher.wfsa = cipher.wfsa.trained (! specified in cipher.wfsa writer)
awk 'NF>0' $cipher_file > cipher.data.noe 
cat cipher.data.noe | carmel -qbsriWIEk 1 cipher.wfsa.noe cipher.fst.trained
