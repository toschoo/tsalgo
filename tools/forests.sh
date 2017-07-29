# ========================================================================
# (c) Tobias Schoofs, 2015 -- 2016
# ========================================================================
# Create at print 10 random trees with number of elements
# indicated by user (default: 10).
# ========================================================================

if [ $# -lt 1 ]
then
	n=10
else
	n=$1
fi

n2=$(($n/2))

echo "inserting $n, deleting $n2"

for i in {1..10}
do

	#shuffle
	ins=$(tools/shuffle $n)
	del=$(tools/shuffle $n2)

	tools/treei $ins $del

	if [ $? -ne 0 ]
	then
		printf "\nerror with %s\n and %s\n" $ins $del 
		break
	fi
done
printf "\n"
