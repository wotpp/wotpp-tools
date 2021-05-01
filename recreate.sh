mkdir -p tmp
cd tmp

function print_done {
	printf " done.\n"
}

if [ ! -d "wotpp" ]; then
	printf "Cloning wot++...\n"
	git clone https://github.com/Jackojc/wotpp.git wotpp
else
	printf "Pulling wot++...\n"
	cd wotpp
	git pull origin
	cd ..
fi

mkdir -p src
mkdir -p include/wpp

cd wotpp/src/
printf "Finding source files..."
find . -type f -name '*.cpp' -exec cp --parents "{}" ../../src/ \;
print_done
printf "Finding header files..."
find . -type f -name '*.hpp' -exec cp --parents "{}" ../../include/wpp \;
print_done

cd ../../
printf "Removing main.cpp..."
rm src/main.cpp
print_done

cd ../
printf "Copying files..."
cp tmp/src/ . -r
cp tmp/include/ . -r
print_done
