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
1. msdc reporting uncorrect info if emmc disk image is not exists
