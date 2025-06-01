#!/usr/bin/env python3
import sys

def parse_memory_line(line_iter, first_line):
    """
    Given an iterator over lines and the first line containing 'MEMORY:',
    extract the memory tokens (address:byte pairs) from that line or the next.
    Returns a list of (address_int, byte_int) tuples.
    """
    line = first_line.strip()
    # Strip off the "MEMORY:" prefix if present
    if line.startswith("MEMORY:"):
        rest = line[len("MEMORY:"):].strip()
        if rest:
            tokens = rest.split()
        else:
            # The pairs are on the next line
            try:
                next_line = next(line_iter)
            except StopIteration:
                return []
            tokens = next_line.strip().split()
    else:
        # If first_line didn’t actually start with "MEMORY:", treat it as the memory data
        tokens = line.split()

    mem_entries = []
    for tok in tokens:
        if ":" not in tok:
            continue
        addr_str, byte_str = tok.split(":", 1)
        try:
            addr = int(addr_str, 16)
            byte = int(byte_str, 16)
        except ValueError:
            # Skip tokens that don’t parse as hex
            continue
        mem_entries.append((addr, byte & 0xFF))
    return mem_entries

def hex_dump(mem_dict):
    """
    Given a dict mapping addresses (ints) to byte values (ints 0–255),
    produce a hex dump where each line is:
      <address> <word@addr> <word@addr+4> <word@addr+8> <word@addr+12>
    in big-endian order, iterating downwards until all memory is printed.
    """
    if not mem_dict:
        return ""

    addrs = sorted(mem_dict.keys())
    min_addr = addrs[0] & ~0x3      # round down to 4-byte boundary
    max_addr = addrs[-1]
    # Round up max_addr to include the last full word
    max_word_end = (max_addr | 0x3)

    lines = []
    # Step by 16 bytes per row (4 words × 4 bytes each)
    row_start = min_addr
    while row_start <= max_word_end:
        words = []
        for offset in (0, 4, 8, 12):
            base = row_start + offset
            # Gather 4 bytes at base, base+1, base+2, base+3
            b0 = mem_dict.get(base + 0, 0)
            b1 = mem_dict.get(base + 1, 0)
            b2 = mem_dict.get(base + 2, 0)
            b3 = mem_dict.get(base + 3, 0)
            word = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3
            words.append(f"0x{word:08x}")
        line = f"0x{row_start:08x} " + " ".join(words)
        lines.append(line)
        row_start += 16

    return "\n".join(lines)

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <debug_file>", file=sys.stderr)
        sys.exit(1)

    debug_file = sys.argv[1]
    try:
        with open(debug_file, "r") as f:
            lines = f.readlines()
    except IOError as e:
        print(f"Error opening file {debug_file}: {e}", file=sys.stderr)
        sys.exit(1)

    line_iter = iter(lines)
    cycle_index = 0

    for raw_line in line_iter:
        if raw_line.strip().startswith("MEMORY:"):
            # Parse this memory block
            mem_entries = parse_memory_line(line_iter, raw_line)
            if not mem_entries:
                continue

            # Build a dict of address → byte
            mem_dict = {addr: byte for addr, byte in mem_entries}

            dump = hex_dump(mem_dict)
            print(f"# Memory dump, cycle {cycle_index}")
            print(dump)
            print()  # blank line between cycles
            cycle_index += 1

if __name__ == "__main__":
    main()
