properties="/home/hkatehar/gfe_driver/graph22/datasets/graph500-22.properties"
output_file="/DS/DSG/work/newlogs/without_updates/1-64.txt"

for w in {1..64}
# for w in {16..18}
do
    ./driver -g $properties -w $w | tee -a $output_file
    echo "END" >> $output_file
done


