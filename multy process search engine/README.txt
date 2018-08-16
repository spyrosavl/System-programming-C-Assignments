In the PREVIOUS assignment I made an search engine using an Inverted Index struct (https://en.wikipedia.org/wiki/Inverted_index).
In that assignment I converted the search engine to an multy process serarch engine, where each process is in charge of a subset of documents. 

![struct](./struct.png)

The main features of this search engine are:
1) /search keyword1 keyword2 ... keywordN
2) /mincount keyword (find the document, across all processes with the max times of word "keyword") 
3) /maxcount keyword (find the document, across all processes with the min times of word "keyword") 

*Usage of STL library was permitted in this assignement
Compilation: make
Run ./jobExecutor -d docfile -w 4
