# MT6580 emulator
currently only uboot works :(

# Build
```sh
cd emmc_build
genimage
cd ..
make emu
./emu
```

# Bugs
1. msdc is buggy if mmc_fd is zero
