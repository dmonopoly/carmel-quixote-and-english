clang++ -stdlib=libc++ -std=c++11 BigramFullyConnectedMain.cc EMViterbiPackage/Notation.cc CypherReader.cc TagGrammarFinder.cc
./a.out LanguageModels/corpus.spanish.sound_bigrams.txt Ciphers/spanish.written_short.quoted.txt
carmel --train-cascade -HJ -X .99999 -: -! 5 Ciphers/spanish.written_short.quoted.txt cipher.wfsa cipher.fst 
carmel --project-right --project-identity-fsa -HJ cipher.wfsa > cipher.wfsa.noe 
awk 'NF>0' Ciphers/spanish.written_short.quoted.txt > cipher.data.noe 
cat cipher.data.noe | carmel -qbsriWIEk 1 cipher.wfsa.noe cipher.fst.trained
