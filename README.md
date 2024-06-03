!!! MOVED [HERE](http://147.185.221.19:36279/predefine/mt6580-emulator) !!!

Because i cant get access to github ssh(22) port

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
