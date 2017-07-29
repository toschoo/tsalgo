# ========================================================================
# (c) Tobias Schoofs, 2015 -- 2016
# ========================================================================
# Find an issue with the AVL tree algorithm
# ========================================================================
# Creates a random permutations of lists of numbers,
# runs treei and report whether there was an error.
# ========================================================================

if [ $# -lt 1 ]
then
	n=10
else
	n=$1
fi

n2=$(($n/2))

echo "inserting $n, deleting $n2"

printf "0000"
for i in {1..1000}
do

	#shuffle
	ins=$(tools/shuffle $n)
	del=$(tools/shuffle $n2)

	tools/treei $ins $del > /dev/null 2>&1

	if [ $? -ne 0 ]
	then
		printf "\nerror with %s\n and %s\n" $ins $del 
		break
	else
		printf "\b\b\b\b%04d" $i
	fi
done
printf "\n"
