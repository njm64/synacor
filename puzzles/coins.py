# Solver for the coins puzzle

import itertools

for p in itertools.permutations([2,3,5,7,9]):
    a = p[0]
    b = p[1]
    c = p[2]
    d = p[3]
    e = p[4]
    s = a + b * pow(c, 2) + pow(d, 3) - e
    if s == 399:
        print(a, b, c, d, e)
        break

# 9 2 5 7 3
# blue, red, shiny, concave, corroded    
