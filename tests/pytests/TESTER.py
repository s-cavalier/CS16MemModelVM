from sys import argv
from os import fork, execv, execvp, pipe, dup2, close, read, wait
from collections import deque
from math import isclose

def read_pipe(fd):
    output = deque[str]()
    while buffer := read(fd, 128):
        output.extend(buffer.decode().splitlines())
    return output

if __name__ == "__main__":
    if len(argv) != 2:
        print("USAGE: python TESTER.py <filename>")
        exit(1)
    
    # Run spim
    readfd, writefd = pipe()
    
    pid = fork()
    
    if pid == 0:
        close(readfd)
        
        dup2(writefd, 1)    # replace stdout with writefd
        close(writefd)
        
        execvp('spim', ['spim', '-f', f'tests/{argv[1]}.asm'])
        assert(False)       # Should never hit here
    
    close(writefd)
    spim_output = read_pipe(readfd)
    close(readfd)
    wait()
    
    for _ in range(6):
        spim_output.popleft()   # clean up intro text
    
    readfd, writefd = pipe()
    
    # Run spimulator
    pid = fork()
    if pid == 0:
        close(readfd)
        
        dup2(writefd, 1)
        close(writefd)
        
        execv('spimulator', ['spimulator', f'bin/{argv[1]}', '3.14159'])
        assert(False)
    
    close(writefd)
    spimulator_output = read_pipe(readfd)
    close(readfd)
    wait()
    
    print('SPIM:', spim_output)
    print('SPIMULATOR:', spimulator_output)
    print('Passed?', 'True' if isclose(float(spim_output[0]), float(spimulator_output[0]), rel_tol=1e-5) else 'False')
    