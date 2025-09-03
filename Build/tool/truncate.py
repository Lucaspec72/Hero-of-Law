#!/usr/bin/env python3
import os

def truncate_zeros_simple(filename, alignment=16):

    # Find last non-zero byte
    with open(filename, 'rb') as f:
        f.seek(0, 2)  # Go to end
        size = f.tell()
        
        if size == 0:
            return
        
        # Read backwards to find last non-zero byte
        last_nonzero = 0
        chunk_size = 1024
        
        for pos in range(size, 0, -chunk_size):
            start = max(0, pos - chunk_size)
            f.seek(start)
            chunk = f.read(pos - start)
            
            # Find last non-zero in chunk
            for i in range(len(chunk) - 1, -1, -1):
                if chunk[i] != 0:
                    last_nonzero = start + i + 1
                    break
            
            if last_nonzero > 0:
                break
    
    # Calculate aligned size
    if last_nonzero == 0:
        new_size = alignment
    else:
        new_size = ((last_nonzero - 1) // alignment + 1) * alignment
    
    # Truncate if smaller than original
    if new_size < size:
        with open(filename, 'r+b') as f:
            f.truncate(new_size)
        print(f"Truncated {filename}: {size} -> {new_size} bytes")

# Example usage
if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1:
        for filename in sys.argv[1:]:
            truncate_zeros_simple(filename)
    else:
        print("Usage: python script.py <file1> [file2] ...")
