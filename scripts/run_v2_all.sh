# ./driver -g  -w 16 -l  -b 1000000 -r 1000000 -R 3 -t 4 -c 0
properties="/home/hkatehar/gfe_driver/graph22/datasets/graph500-22.properties"
log="/DS/DSG/nobackup/update_files/graph500-22-1.0.graphlog"
output_file="/DS/DSG/work/newlogs/3_and_4_v2/w128_R0-10.txt"


b=1000000
for w in 128
do
    for R in 0 1 2 3 4 5 6 7 8 9 10
    do
        for t in 3 4
        do
            ./driver -g $properties -w $w -l $log -b $b -R $R -t $t | tee -a $output_file
            echo "END" >> $output_file
        done
    done
done



