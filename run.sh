#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR"
if [ ! -f "$DIR/build/PythonLauncher" ]; then
    echo "ビルド済みバイナリが見つかりません。ビルドを実行します..."
    mkdir -p build && cd build && cmake .. && make -j$(nproc) && cd ..
fi
exec "$DIR/build/PythonLauncher" "$@"
