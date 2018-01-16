make clean
make
echo -e '\033[31m ╔══════════════════════════════╗'
echo -n ' ║ '
echo -n `date`
echo    ' ║ '
echo -e '\033[31m ╚══════════════════════════════╝'
echo -e '\033[0m'
cat testfile.txt | ./hw4_mm_test > outputfile.txt
if cmp -s outputfile.txt solution.txt ; then
    echo "same"
else
    diff outputfile.txt solution.txt
fi
