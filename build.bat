mkdir build
cls

"wasi-sdk/bin/clang++" ^
  -Wl,--no-entry ^
  -nostartfiles ^
  -fno-exceptions ^
  -Wl,--strip-all ^
  -Wno-c99-designator ^
  -fvisibility=hidden ^
  -Ofast ^
  -std=c++20 ^
  --sysroot="wasi-sdk/share/wasi-sysroot" ^
  "src/*.cpp" -o "build/bot.wasm"

  @REM -mmultivalue ^
  @REM -Xclang -target-abi -Xclang experimental-mv ^

node "yare.io-wasm/wasm2yareio" "build/bot.wasm"