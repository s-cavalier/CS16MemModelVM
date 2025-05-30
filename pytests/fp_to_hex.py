import struct

def float32_to_hex(f):
    [i] = struct.unpack('>I', struct.pack('>f', f))
    return f"{i:08X}"  # 8 hex digits for 32 bits

def float64_to_hex(f):
    [d] = struct.unpack('>Q', struct.pack('>d', f))
    return f"{d:016X}"

if __name__ == "__main__":
    from sys import argv
    if len(argv) != 3:
        print("USAGE: python fp_to_hex.py <f: float, d: double, fd: both> <fp number>")
        exit(1)
    
    num = float(argv[2])

    if 'f' in argv[1]:
        print('SINGLE PRECISION:', float32_to_hex(num))
    if 'd' in argv[1]:
        print('DOUBLE PRECISION:', float64_to_hex(num))
