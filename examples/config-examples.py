# Run this to generate Makefile, then run 'make'
EXAMPLES = ['minimum']
OUTPUT = 'Makefile'

ofh = open(OUTPUT, 'w')

ofh.write('all\t: ' + ' '.join(EXAMPLES) + '\n\n')

ofh.write('clean\t:\n')
ofh.write('\trm -f *.o\n')
ofh.write('\trm ' + ' '.join(EXAMPLES) + '\n')

for e in EXAMPLES:
    ofh.write(e + '.o\t: ' + e + '.cpp\n')
    ofh.write('\tg++ -std=c++11 -I../src -c ' + e + '.cpp\n\n')
    ofh.write(e + '\t: ' + e + '.o\n')
    ofh.write('\t../libtool --tag=CXX --mode=link g++ -o ' + e + ' ' + e + '.o ../libiroha.la')

print('Generated Makefile. Please run \'make\'')
