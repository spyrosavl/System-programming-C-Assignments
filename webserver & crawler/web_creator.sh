#!/usr/bin/env bash
function createPage {
    echo -e "<!DOCTYPE html>\n<html>\n\t<body>"
}
function closingTags {
	echo -e "\t</body>\n</html>"
}

root_directory=$1
text_file=$2
no_of_websites=$3
pages_per_website=$4


if [ "$#" -ne 4 ]; then
    echo "usage: ./web_creator.sh root_directory text_file no_of_websites pages_per_website"
    exit
fi

if [ !  -d $root_directory  ]; then
    echo "$1 doesn't exist."
    exit
fi

if [ ! -r $text_file ]; then
    echo "\"$text_file\" doesn't exist or is not readable"
    exit
fi

if !([[ $no_of_websites =~ ^[\-0-9]+$ ]] && (( no_of_websites > 0))); then
	echo "no_of_websites should be positive integer"
	exit
fi

if !([[ $pages_per_website =~ ^[\-0-9]+$ ]] && (( pages_per_website > 0))); then
	echo "pages_per_website should be positive integer"
	exit
fi

if [[ $(wc -l <$text_file) -lt 10000 ]]; then
	echo "\"$text_file\" should have more than 10000 lines"
    exit
fi

$(sed -i '/^\s*$/d' $text_file)

if [[ $root_directory != '/' ]]; then
    rm -rf $root_directory/*
fi
sleep 1

declare -A websites
pages_count=0
for (( i=0; i<$no_of_websites; i++)) 
do
    declare -A array
    site="/site$i"
    for (( j=0; j<$pages_per_website; j++)) do
            ((pageID = RANDOM))
            if [[ -z ${array[$pageID]} ]]; then
                page="$site/page_$pageID.html"
                array[$pageID]="exists"
                pages+=($page)
            else
            	((j--))
            fi
    done
    websites[$site]=${pages[@]}
    allpages+=(${pages[@]})
    pages=()
done


no_of_lines=$(wc -l <$text_file)
tmp=$(($no_of_lines-2000))
f=$((pages_per_website / 2 + 1))
q=$((no_of_websites / 2 + 1))

declare -A pageHasIncoming


for site in "${!websites[@]}" 
do
	mkdir "$root_directory$site"
    echo "Creating website \"$site\""
    pages=${websites[$site]}
    for page in ${pages[@]}
    do
		touch "$root_directory$page"
        createPage >> "$root_directory$page"

		k=$(shuf -i 1-$tmp -n 1)
	    m=$(shuf -i 1000-2000 -n 1)
    	
	    insideLinks+=( $(echo "${websites[$site]}" | tr ' ' '\n'  | shuf | head -n "$f" | tr '\n' ' ' ) )
	    outboundLinks+=( $(echo "${allpages[@]}" | tr ' ' '\n'  | shuf | head -n "$q" | tr '\n' ' ' ) )
	    allLinks=( "${insideLinks[@]}" "${outboundLinks[@]}" )

	    begin_from_line=$k
	    lines_to_copy=$(($m/($f+$q)))

		echo -e "\tCreating page $page with $lines_to_copy lines starting at line $begin_from_line"

		for line in $(seq $begin_from_line $((lines_to_copy+begin_from_line)));
        do
            current_link=$(echo "${allLinks[@]}"  | tr ' ' '\n' | shuf | head -n 1)
            line_string=$(sed "${line}q;d" $text_file)
            html_tag=$(echo -e "\t\t<a href=\"$current_link\" >$line_string</a><br/>")
            echo "$html_tag" >> "$root_directory$page"
            echo -e "\t\tAdding link to $current_link"
            pageHasIncoming[$current_link]=true
            #echo -e "<br/>\n" >> "$page"
        done

        closingTags >> "$root_directory$page"

	    outboundLinks=()
	    insideLinks=()
	done
done

if [[ "$(($no_of_websites*$pages_per_website))" == "${#pageHasIncoming[@]}" ]]; then
    echo -e "\n\nAll pages have at least one incoming link"
else
    echo -e "\n\nAt least one page doesn't have any incoming link"
fi