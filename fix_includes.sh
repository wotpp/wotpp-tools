for f in $(find src/ -type f -name "*.cpp" ) $(find include/ -type f -name "*.hpp"); do
	python include.py $f
done
