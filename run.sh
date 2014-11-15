g++ main.cpp -o hash `pkg-config --cflags --libs  opencv`
videoName=$1

[ ! -f hash ] && exit
    ./hash $videoName #> ${videoName%.*}.txt
