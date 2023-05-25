properties="/home/hkatehar/gfe_driver/graph22/datasets/graph500-22.properties"
log="/DS/DSG/nobackup/update_files/graph500-22-1.0.graphlog"
output_file="/DS/DSG/work/newlogs/3_and_4/file.txt"

b=100000
# for b in 1000 10000 100000 1000000 10000000
# for b in 100 100000000
for w in 16 24 32 40 48 56 64
do
    for t in 3 4
    do
        ./driver -g $properties -w $w -l $log -b $b -t $t -r 500000 -R 5 | tee -a $output_file
        echo "END" >> $output_file
    done
done


