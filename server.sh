rec -t raw -b 16 -c 1 -e s -r 44100 - | ./i1i2i3_phone_tcp 50000 2> test.dat > /dev/null 
