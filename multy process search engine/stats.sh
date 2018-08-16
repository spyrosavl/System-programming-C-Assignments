#!/bin/bash
lines=$(grep -F 'search:' log/*)
maxKey=""
minkey=""
maxCount=0
minCount=999999
declare -A queryCount

while read -r line; do
  key=$(echo $line | cut -d " " -f 7)
  count=$(echo $line | cut -d " " -f9- | wc -w)
  let queryCount[$key]+=$count
  #echo $key, ${queryCount[$key]}
done <<< $lines

for i in "${!queryCount[@]}"
do
  if (( ${queryCount[$i]} > $maxCount )); then
    maxCount=${queryCount[$i]}
    maxKey=$i
  fi
  if (( ${queryCount[$i]} < $minCount )); then
    minCount=${queryCount[$i]}
    minkey=$i
  fi
done

echo "Total number of keywords searched:" ${#queryCount[@]}
echo "Keyword most frequently found: $maxKey [totalNumFilesFound: $maxCount]:"
echo "Keyword least frequently found: $minkey [totalNumFilesFound: $minCount]:"