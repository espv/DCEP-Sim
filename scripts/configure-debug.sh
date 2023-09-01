#!/bin/bash
python3 ns3 clean && python3 ns3 configure --disable-werror --enable-examples --enable-tests --build-profile debug --enable-modules "dcep;stream-processing" && python3 ns3 build
