import sys
from parse import compile

if __name__ == '__main__':
    result = ''
    n = 0

    with open(sys.argv[1], 'r') as f:
        p = compile("#include <{}>")

        for line in f.readlines():
            include = p.parse(line.strip())

            if not include:
                result += line
            else:
                include = include[0]
                if include.endswith('.hpp') or include.endswith('.cpp'):
                    result += '#include <wpp/{}>\n'.format(include)
                    n += 1
                else:
                    result += line

    print('Successfully amended {} includes in {}, writing output...'.format(n, sys.argv[1]), end='')
    with open(sys.argv[1], 'w') as f:
        f.write(result)

    print(' done.')
