import os
import struct
import sys
from pathlib import Path
import re

def align_to_32(value):
    """Align a value to the next 32-byte boundary"""
    return (value + 31) & ~31

def sanitize_define_name(name):
    """Convert filename to a valid C define name"""
    # Remove file extension
    name = Path(name).stem
    # Replace spaces and special characters with underscores
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name)
    # Ensure it doesn't start with a number
    if name and name[0].isdigit():
        name = '_' + name
    # Convert to uppercase
    return name.upper()

def sanitize_path_for_define(path):
    """Convert a folder path to a valid C define name component"""
    # Convert path to string and normalize separators
    path_str = str(path)
    # Replace path separators and special characters with underscores
    sanitized = re.sub(r'[^a-zA-Z0-9_]', '_', path_str)
    # Remove multiple consecutive underscores
    sanitized = re.sub(r'_+', '_', sanitized)
    # Remove leading/trailing underscores
    sanitized = sanitized.strip('_')
    
   
    # Convert to uppercase
    return sanitized.upper()

def concatenate_files_simple(folder_path, output_file):
    """
    Simple concatenation of files in numerical order without headers.
    Individual files are padded to be divisible by 32 bytes.
    Creates a .h file with offset defines.
    
    Args:
        folder_path: Path to folder containing numbered files
        output_file: Output file path
    """
    folder = Path(folder_path)
    
    if not folder.exists() or not folder.is_dir():
        print(f"Error: {folder_path} is not a valid directory")
        return
    
    # Get all files and sort them numerically by filename
    files = []
    for file_path in folder.iterdir():
        if file_path.is_file() and file_path.name != "__noheader__":
            try:
                # Extract number and explanation from filename (format: number - explanation.extension)
                stem = file_path.stem  # filename without extension
                if ' - ' in stem:
                    number_str, explanation = stem.split(' - ', 1)
                    file_num = int(number_str)
                    files.append((file_num, file_path, explanation))
                else:
                    print(f"Warning: Skipping file {file_path.name} - doesn't match 'number - explanation.extension' format")
                    continue
            except ValueError:
                print(f"Warning: Skipping file {file_path.name} - number part is not valid")
                continue
    
    # Sort by file number
    files.sort(key=lambda x: x[0])
    
    if not files:
        print("Error: No properly formatted files found in directory")
        return
    
    # Calculate offsets and prepare header content
    current_offset = 0
    header_defines = []
    
    # Write the output file and collect offset information
    try:
        with open(output_file, 'wb') as outf:
            for file_num, file_path, explanation in files:
                print(f"  Adding {file_num}: {file_path.name} at offset {current_offset}")
                
                # Create define name from the explanation part (filename without "number - " prefix)
                define_name = sanitize_define_name(explanation + '.' + file_path.suffix[1:] if file_path.suffix else explanation)
                header_defines.append(f"#define OFFSET_{define_name} 0x{current_offset:X}")
                
                # Copy file content
                with open(file_path, 'rb') as inf:
                    data = inf.read()
                    outf.write(data)
                
                # Add padding to make this file's size divisible by 32
                original_size = len(data)
                padded_size = align_to_32(original_size)
                padding_needed = padded_size - original_size
                if padding_needed > 0:
                    outf.write(b'\x00' * padding_needed)
                
                # Update offset for next file
                current_offset += padded_size
        
        print(f"Successfully created {output_file} (simple concatenation)")
        
        # Create the header file
        header_file = Path(output_file).with_suffix('.h')
        
        # Create include guard name that incorporates both folder path and output filename
        parent_path = os.path.dirname(folder_path)
        folder_part = sanitize_path_for_define(os.path.basename(parent_path))
        
        filename_part = sanitize_define_name(Path(output_file).name)
        guard_name = f"_{folder_part}_{filename_part}_H"
        
        with open(header_file, 'w') as hf:
            hf.write(f"// Auto-generated header file for {Path(output_file).name}\n")
            hf.write(f"// Generated from folder: {folder_path}\n\n")
            hf.write(f"#ifndef {guard_name}\n")
            hf.write(f"#define {guard_name}\n\n")
            
            for define in header_defines:
                hf.write(define + '\n')
            
            hf.write(f"\n#endif // {guard_name}\n")
        
        print(f"Successfully created header file: {header_file}\n")

    except Exception as e:
        print(f"Error writing output file: {e}")

def concatenate_files_with_header(folder_path, output_file):
    """
    Concatenate files in numerical order with a header containing offsets and sizes.
    Individual files are padded to be divisible by 32 bytes.
    Files are 0-indexed (first file has id 0).
    
    Args:
        folder_path: Path to folder containing numbered files
        output_file: Output file path
    """
    folder = Path(folder_path)
    
    if not folder.exists() or not folder.is_dir():
        print(f"Error: {folder_path} is not a valid directory")
        return
    
    # Get all files and sort them numerically by filename
    files = []
    for file_path in folder.iterdir():
        if file_path.is_file() and file_path.name != "__noheader__":
            try:
                # Extract number and explanation from filename (format: number - explanation.extension)
                stem = file_path.stem  # filename without extension
                if ' - ' in stem:
                    number_str, explanation = stem.split(' - ', 1)
                    file_num = int(number_str)
                    files.append((file_num, file_path, explanation))
                else:
                    print(f"Warning: Skipping file {file_path.name} - doesn't match 'number - explanation.extension' format")
                    continue
            except ValueError:
                print(f"Warning: Skipping file {file_path.name} - number part is not valid")
                continue
    
    # Sort by file number
    files.sort(key=lambda x: x[0])
    
    if not files:
        print("Error: No properly formatted files found in directory")
        return
    
    # Create a mapping for reference lookup
    file_map = {num: (path, explanation) for num, path, explanation in files}
    
    # Determine the maximum ID to know how many header slots we need
    max_id = max(num for num, _, _ in files)

    # Process files and determine which are references
    # Create header entries for ALL slots from 0 to max_id
    header_entries = []  # Will contain (entry_num, target_entry_num, file_path, explanation, is_empty)
    unique_files = {}    # Maps target_entry_num to (file_path, original_size, padded_size)
    
    for entry_num in range(0, max_id + 1):
        if entry_num in file_map:
            file_path, explanation = file_map[entry_num]
            
            # Check if explanation is just a number (reference to another entry)
            try:
                ref_num = int(explanation)
                if ref_num in file_map:
                    # This entry references another entry
                    ref_path, ref_explanation = file_map[ref_num]
                    header_entries.append((entry_num, ref_num, ref_path, ref_explanation, False))
                    print(f"  {entry_num}: {file_path.name} -> references entry {ref_num} ({ref_path.name})")
                    
                    # Make sure the referenced file is in our unique files list
                    if ref_num not in unique_files:
                        original_size = ref_path.stat().st_size
                        padded_size = align_to_32(original_size)
                        unique_files[ref_num] = (ref_path, original_size, padded_size)
                else:
                    print(f"Warning: File {file_path.name} references non-existent entry {ref_num}, treating as normal file")
                    header_entries.append((entry_num, entry_num, file_path, explanation, False))
                    original_size = file_path.stat().st_size
                    padded_size = align_to_32(original_size)
                    unique_files[entry_num] = (file_path, original_size, padded_size)
            except ValueError:
                # Explanation is not a number, treat as normal file
                header_entries.append((entry_num, entry_num, file_path, explanation, False))
                original_size = file_path.stat().st_size
                padded_size = align_to_32(original_size)
                unique_files[entry_num] = (file_path, original_size, padded_size)
        else:
            # Empty slot - fill with zeros
            header_entries.append((entry_num, None, None, "empty", True))

    # Calculate header size
    # 4 bytes for numOfEntries + 8 bytes per header entry (4 for offset + 4 for padded size)
    header_size = 4 + (len(header_entries) * 8)
    
    # Calculate file offsets (only for unique files)
    current_offset = align_to_32(header_size)
    file_offsets = {}  # Maps target_entry_num to offset
    
    for target_num in sorted(unique_files.keys()):
        file_path, original_size, padded_size = unique_files[target_num]
        file_offsets[target_num] = current_offset
        current_offset += padded_size
    
    # Build header entries with their offsets and sizes
    header_data = []
    for entry_num, target_num, file_path, explanation, is_empty in header_entries:
        if is_empty:
            # Empty slot gets zero offset and zero size
            header_data.append((0, 0))
        else:
            offset = file_offsets[target_num]
            _, _, padded_size = unique_files[target_num]
            header_data.append((offset, padded_size))
    
    # Write the output file
    try:
        with open(output_file, 'wb') as outf:
            # Write header
            # Number of entries (big endian)
            outf.write(struct.pack('>I', len(header_entries)))
            
            # Write offset and padded size for each header entry (big endian)
            for offset, padded_size in header_data:
                outf.write(struct.pack('>I', offset))      # offset
                outf.write(struct.pack('>I', padded_size)) # padded size
            
            # Pad to align the first file to 32 bytes
            current_pos = outf.tell()
            if unique_files:
                first_file_offset = min(file_offsets.values())
                padding_needed = first_file_offset - current_pos
                if padding_needed > 0:
                    outf.write(b'\x00' * padding_needed)
            
            # Write unique file contents with padding
            for target_num in sorted(unique_files.keys()):
                file_path, original_size, padded_size = unique_files[target_num]
                expected_offset = file_offsets[target_num]
                current_pos = outf.tell()
                
                # Verify we're at the expected offset
                if current_pos != expected_offset:
                    print(f"Warning: Expected offset {expected_offset}, but at {current_pos}")
                
                # Copy file content
                with open(file_path, 'rb') as inf:
                    data = inf.read()
                    outf.write(data)
                
                # Add padding to make this file's size divisible by 32
                padding_needed = padded_size - original_size
                if padding_needed > 0:
                    outf.write(b'\x00' * padding_needed)
        
        print(f"Successfully created {output_file}")
        print(f"Header size: {header_size} bytes, entries: {len(header_entries)}\n")

    except Exception as e:
        print(f"Error writing output file: {e}")

def main():
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_folder> <output_file>")
        print("Example: python script.py ./input_files output.bin")
        print("Files should be named in format: 'number - explanation.extension'")
        print("Files are 0-indexed (first file should be named '0 - description.ext')")
        print("If explanation is a number, that header entry will point to the referenced entry's file")
        print("Missing IDs will create empty header slots with zero offset and size")
        print("Create a '__noheader__' file in the folder to concatenate without headers")
        print("For headerless mode, a .h file with offset defines will be created")
        sys.exit(1)
    
    input_folder = sys.argv[1]
    output_file = sys.argv[2]
    
    print(f"Concatenating {input_folder}")
    
    # Check if __noheader__ file exists - try multiple approaches
    folder_path = Path(input_folder)
    noheader_file = folder_path / "__noheader__"
    
    # Debug: List all files in the directory
    print(f"Files in directory:")
    for item in folder_path.iterdir():
        print(f"  {item.name} ({'file' if item.is_file() else 'dir'})")
    
    # Check for __noheader__ file existence
    has_noheader = False
    for item in folder_path.iterdir():
        if item.is_file() and item.name == "__noheader__":
            has_noheader = True
            break
    
    if has_noheader:
        print("Found __noheader__ file - concatenating without headers")
        concatenate_files_simple(input_folder, output_file)
    else:
        print("No __noheader__ file found - concatenating with headers")
        concatenate_files_with_header(input_folder, output_file)

if __name__ == "__main__":
    main()
