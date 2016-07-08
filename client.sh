rec -t raw -b 16 -c 1 -e s -r 44100 - | ./i1i2i3_phone_tcp 127.0.0.1 50000 | play -t raw -b 16 -c 1 -e s -r 44100 -
