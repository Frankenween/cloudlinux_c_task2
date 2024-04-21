# A simple ls-like utility
This utility can list files in a directory and all its subdirectories.

Some parameters are available:
- `--no-req`: do not list subdirectories recursively.
- `--quote-all`: quote all names.
- `--no-quote`: quote no names, even is they contain spaces.
- `--all`: list all files, including hidden ones(starting with ".") and "." and ".." directories
- `--almost-all`: list all files except files with names "." and ".."
- `--types`: write file type after its name. Types will be colored!
- `--root=path`: list file in given directory. It can be absolute or relative.