import os, time
import subprocess

fout = open('results.csv', 'w')
fout.write("")
fout.close()

flags = [
    'NO_QSEARCH_EVASION',
]

cpp_first_names = [
    'bits',
    'eval',
    'hash',
    'movegen',
    'order',
    'pos',
    'search',
    'timer',
    'tt',
    'types',
    'uci'
    ]

def list_of_affected_files():
    result = []
    for filename in os.listdir(os.getcwd()):
        if filename.endswith('.cpp') or filename.endswith('.h'):
            with open(filename) as file:
                for line in file:
                    if (line.startswith("#ifdef") or line.startswith("#ifndef")):
                        result.append(filename)
                        break
    return result

def add_entry(stdout, enabled_flags):
    stdout = stdout.decode()
    total_points = 0
    for score in stdout.split(','):
        if score.isdigit():
            total_points += int(score)
    fout = open('results.csv', 'a')
    fout.write(('|'.join(enabled_flags) if len(enabled_flags) != 0 else 'NONE') + ',' + stdout + str(total_points) + '\n')
    fout.close()

def get_itr_flags(i):
    result = []
    bini = bin(i)[2:].zfill(len(flags))
    for j in range(len(flags)):
        if bini[j] == '1':
            result.append(flags[j])
    return result

subprocess.run('make clean', shell=True)
subprocess.run('make objects', shell=True)

assert(len(flags) <= 32)

i = 0
while i < 2**len(flags):
    files_to_recompile = list_of_affected_files()
    itr_flags = get_itr_flags(i)
    for file in files_to_recompile:
        first_name = file.split('.')[0]
        cmd = 'clang++ -c -Ofast ' + first_name + '.cpp -o ' + first_name + '.o -D CSV_OUTPUT' + (' -D ' + " -D ".join(itr_flags) if i != 0 else '')
        print("recompiling " + first_name)
        subprocess.run(cmd, shell=True)
    cmd = 'clang++ -Ofast -pthread ' + '.o '.join(cpp_first_names) + '.o ' + 'unit_tests.cpp -o unit_tests.exe -D CSV_OUTPUT' + (' -D ' + " -D ".join(itr_flags) if i != 0 else '')
    print(cmd)
    subprocess.run(cmd, shell=True)
    proc = subprocess.run('unit_tests.exe', shell=True, capture_output=True)
    add_entry(proc.stdout, itr_flags)
    i += 1
