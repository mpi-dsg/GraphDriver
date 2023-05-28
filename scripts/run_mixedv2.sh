properties="/home/hkatehar/gfe_driver/graph22/datasets/graph500-22.properties"
log="/DS/DSG/nobackup/update_files/graph500-22-1.0.graphlog"
output_file="/DS/DSG/work/newlogs/3_v2/w8-128_c1-31.txt"

b=1000000

for w in 8 12 16 24 32 48 64 96 128
do
    for c in 1 8 16 24 31
    do
        ./driver -g $properties -w $w -l $log -b $b -r 1000000 -R 3 -t 3 -c $c | tee -a $output_file
        echo "END" >> $output_file
    done
done


