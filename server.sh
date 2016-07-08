rec -t raw -b 16 -c 1 -e s -r 44100 - 2> /dev/null | ./i1i2i3_phone_tcp 50000 | play -t raw -b 16 -c 1 -e s -r 44100 - 2> /dev/null
