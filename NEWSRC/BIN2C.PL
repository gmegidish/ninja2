
binmode (STDIN);

print "unsigned char my_crap[] = {\n";

while (16 == read (STDIN, $buffer, 16)) {
       print "\t";
       for ($i=0; $i<16; $i++) {
               printf "0x%02x, ", ord(substr ($buffer, $i, 1));
       }
       print "\n";
}

print "};\n";
