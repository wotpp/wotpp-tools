for d in $(ls -d *); do
	CC=clang++ make -C $d
done
