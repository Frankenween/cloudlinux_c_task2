# A simple ls-like utility
This utility can list files in current working directory and all subdirectories.
It currently can't work with listing directories provided via program arguments.

Some parameters are available:
- `--no-req`: do not list subdirectories recursively.
- `--quote-all`: quote all names.
- `--no-quote`: quote no names, even is they contain spaces.
- `--all`: list all files, including hidden ones(starting with ".") and "." and ".." directories
- `--almost-all`: list all files except files with names "." and ".."
- `--types`: write file type after its name. Types will be colored!