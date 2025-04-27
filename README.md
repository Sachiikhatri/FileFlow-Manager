# FileFlow-Manager 🗄️

A secure and efficient command-line utility for file and directory management in Unix/Linux systems.

## Overview

FileWrangler is a powerful file management tool designed to help developers and system administrators perform common file operations with enhanced security and efficiency. Built with the Unix philosophy in mind, it provides a set of focused commands that can be easily integrated into scripts or used directly from the terminal.

## Features

- **File Listing**: Recursively traverse directories and list all files
- **Extension Filtering**: Find and process files with specific extensions (.c, .txt, .pdf)
- **Statistical Operations**: Count files, directories, or calculate total file sizes
- **File Operations**: Safely copy or move files between directories
- **Selective Deletion**: Remove files matching specific extensions
- **Security-Focused**: All operations restricted to user's home directory
- **Error Handling**: Comprehensive error detection and reporting
- **Directory Creation**: Automatically creates directories when needed

## Requirements

- Unix/Linux operating system
- GCC compiler
- Standard C libraries

## Installation

1. Clone the repository:
   ```
   git clone https://github.com/yourusername/filewrangler.git
   ```

2. Navigate to the project directory:
   ```
   cd filewrangler
   ```

3. Compile the program:
   ```
   gcc -o filewrangler a1_sachi_khatri_110184013.c -D_GNU_SOURCE
   ```

4. Make the program executable:
   ```
   chmod +x filewrangler
   ```

5. (Optional) Add to your PATH for system-wide access:
   ```
   sudo cp filewrangler /usr/local/bin/
   ```

## Usage

```
./filewrangler [COMMAND] [SOURCE_DIR] [ADDITIONAL_ARGS]
```

### Available Commands:

| Command | Syntax | Description |
|---------|--------|-------------|
| `-ls` | `-ls <root_dir>` | List all files recursively |
| `-ext` | `-ext <root_dir> <file_extension>` | List files with specific extension |
| `-fc` | `-fc <root_dir>` | Count total number of files |
| `-dc` | `-dc <root_dir>` | Count total number of directories |
| `-fs` | `-fs <root_dir>` | Calculate total size of all files |
| `-cp` | `-cp <source_dir> <destination_dir> [file_extension]` | Copy files (excludes specified extension if provided) |
| `-mv` | `-mv <source_dir> <destination_dir>` | Move files and directories |
| `-del` | `-del <root_dir> <file_extension>` | Delete files with specific extension |

### Examples:

```bash
# List all files in ~/Documents
./filewrangler -ls ~/Documents

# Find all C files in ~/Projects
./filewrangler -ext ~/Projects .c

# Count files in ~/Downloads
./filewrangler -fc ~/Downloads

# Count directories in ~/Music
./filewrangler -dc ~/Music

# Calculate total size of files in ~/Videos
./filewrangler -fs ~/Videos

# Copy all files except .pdf from ~/Work to ~/Backup
./filewrangler -cp ~/Work ~/Backup .pdf

# Move all files from ~/Temp to ~/Archive
./filewrangler -mv ~/Temp ~/Archive

# Delete all .txt files in ~/Trash
./filewrangler -del ~/Trash .txt
```

## Security Features

- **Path Validation**: All operations are restricted to the user's home directory
- **Safe File Handling**: Files are copied in chunks with error detection
- **Extension Validation**: Only allows common file types (.c, .txt, .pdf)
- **Memory Safety**: Careful memory allocation and deallocation
- **Error Recovery**: Cleanup of partial operations on failure

## Implementation Details

FileWrangler uses the `nftw()` (new file tree walk) function to traverse directory structures efficiently. This approach allows for depth-first traversal with minimal memory footprint while providing detailed information about each file encountered.

Key implementation features:
- Recursive directory creation with proper permissions
- Careful path handling to prevent buffer overflows
- Proper error handling and reporting
- Memory-efficient file copying using buffers

## Exit Codes

- `0`: Operation completed successfully
- `1`: Error occurred during operation

## Limitations

- Currently only supports three file extensions (.c, .txt, .pdf)
- Operations restricted to user's home directory
- No wildcard pattern matching for file selection

## Troubleshooting

### Common Issues:

1. **Permission Denied**: Ensure you have read/write permissions for the directories
2. **Path Not Found**: Verify that source directories exist
3. **Memory Allocation Failed**: System may be low on memory

## Future Enhancements

- Support for additional file extensions
- Pattern matching with wildcards
- File content searching
- Preservation of file attributes during copy/move
- Recursive delete with confirmation
- Progress indicators for large operations

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Author

- Sachi Khatri

## Acknowledgments

- Inspired by classic Unix utilities like `find`, `cp`, and `mv`
  
