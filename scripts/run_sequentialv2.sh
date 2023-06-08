# ./driver -g  -w 16 -l  -b 1000000 -r 1000000 -R 3 -t 4 -c 0
properties="/home/hkatehar/gfe_driver/graph22/datasets/graph500-22.properties"
log="/DS/DSG/nobackup/update_files/graph500-22-1.0.graphlog"
output_file="/DS/DSG/work/newlogs/3_v2/01_w8-128_b1M-100M.txt"


for b in 100000000 50000000 10000000 5000000 1000000 
do
    # for w in 48 64 96 128
    for w in 8 12 16 24 32 
    do
        R=$(($b*300/100000000))
        echo $R 
        ./driver -g $properties -w $w -l $log -b $b -R $R -t 4 | tee -a $output_file
        echo "END" >> $output_file
    done
done



