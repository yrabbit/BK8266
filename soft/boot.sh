#! /bin/bash

cd boot-2apps
make || exit
cd ..

cd EmuAPP
make || exit
cd ..

cd WiFiAPP
make || exit
cd ..

cd help
./mkhelp || exit
cd ..
