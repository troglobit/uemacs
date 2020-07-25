a:cp68 -i a: %1.c %1.i
a:c068 %1.i %1.1 %1.2 %1.3 -f
a:rm %1.i
a:c168 %1.1 %1.2 %1.s
a:rm %1.1
a:rm %1.2
a:as68 -l -u %1.s
a:rm %1.s
