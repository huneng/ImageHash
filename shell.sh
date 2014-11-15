rm *html
count=0;
tmp=0;
index=0;

for img in `ls -v img/`
do
    
    let tmp=count%100;
    if [ $tmp -eq 0 ]
    then
        let index=index+1;
        echo "<html><body bgcolor=\"#DDDDDD\"><table>" > index$index.html
    fi

    echo "<tr><td><img src=img/$img width=\"480\"/><p>$img</p></td><tr>" >> index$index.html

    if [ $tmp -eq 99 ]
    then
        echo "</table></body></html>" >>index$index.html
    fi
    let count=count+1;  
done

let tmp=count%100

if [ $tmp -eq 99 ]
then
    echo "</table></body></html>" >>index$index.html
fi


