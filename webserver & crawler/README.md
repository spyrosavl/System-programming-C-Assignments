This assignemt consists of 3 sub assignmets
1)A bash script wich creates random html pages
2)A c++ web server
3)A c++ web crawler

*Usage of STL library was permitted in this assignement
Compilation:
    make
Web Creator:
    ./web_creator.sh output datasets/random_words.txt 2 2
Server:
    ./myhttpd -p 8080 -c 9090 -t 4 -d output/
Crawler:
    ./mycrawler -h localhost -p 8080 -c 6060 -d crawler_output -t 4 /site0/page_23805.html
