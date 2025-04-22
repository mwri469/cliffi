# cliffi

Cliffi is a **C**ommand **L**ine **Interface** **f**or **Fi**nancial securities.

It is currently under development, see **project updates** to see the current progress and objectives.

## Building instructions

### Requirements

OpenSSL is required for this project to build:
```bash
sudo apt-get install libssl-dev
sudo apt-get install libpsl-dev
```

### Build

From project root dir:

```bash
mkdir build
cd build
cmake ..
make -j
```

## Running

To run the program, just run ./diffi from the build folder
