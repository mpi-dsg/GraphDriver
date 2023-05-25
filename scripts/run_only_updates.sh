properties="/home/hkatehar/gfe_driver/graph22/datasets/graph500-22.properties"
log="/DS/DSG/nobackup/update_files/graph500-22-1.0.graphlog"
output_file="/DS/DSG/work/newlogs/2_updates_only/1-64.txt"

# for b in 1000 10000 100000 1000000 10000000
for b in 100 100000000
# for w in {16..18}
do
    ./driver -g $properties -w 16 -l $log -b $b -t 2 | tee -a $output_file
    echo "END" >> $output_file
done


