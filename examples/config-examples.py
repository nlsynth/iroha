# Run this to generate Makefile, then run 'make'
EXAMPLES = {
    'minimum' : {'minimum'},
    'array' : {'array', 'example_common'},
    'embed' : {'embed', 'example_common'},
    'ext_io' : {'ext_io', 'example_common'},
    'loop' : {'loop', 'example_common'},
    'sub_module' : {'sub_module', 'example_common'},
}
EXAMPLE_OBJS = ['example_common', 'array', 'embed', 'ext_io', 'loop',
                'sub_module', 'minimum']
OUTPUT = 'Makefile'

ofh = open(OUTPUT, 'w')

ofh.write('all\t: ' + ' '.join(EXAMPLES) + '\n\n')

ofh.write('clean\t:\n')
ofh.write('\trm -f *.o\n')
ofh.write('\trm -f ' + ' '.join(EXAMPLES) + '\n')

for e in EXAMPLE_OBJS:
    ofh.write(e + '.o\t: ' + e + '.cpp\n')
    ofh.write('\tg++ -std=c++11 -Wall -g -I../src -c ' + e + '.cpp\n\n')

for k, v in EXAMPLES.iteritems():
    objs = []
    for o in v:
        objs.append(o + '.o')
    obj_lst = ' '.join(objs)
    ofh.write(k + '\t: ../libiroha.la ' + obj_lst + '\n')
    ofh.write('\t../libtool --tag=CXX --mode=link g++ -o ' + k + ' ' + obj_lst + ' ../libiroha.la\n')

print('Generated Makefile. Please run \'make\'')
