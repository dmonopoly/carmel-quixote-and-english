cipher_file='Ciphers/eng.cypher.quoted.txt'
clang++ -stdlib=libc++ -std=c++11 BigramSparseMain.cc EMViterbiPackage/Notation.cc CypherReader.cc TagGrammarFinder.cc
./a.out LanguageModels/eng.bigram.counts.txt $cipher_file
carmel --train-cascade -HJ -X .99999 -: -! 5 $cipher_file cipher.wfsa cipher.fst 
carmel --project-right --project-identity-fsa -HJ cipher.wfsa > cipher.wfsa.noe 
awk 'NF>0' $cipher_file > cipher.data.noe 
cat cipher.data.noe | carmel -qbsriWIEk 1 cipher.wfsa.noe cipher.fst.trained
