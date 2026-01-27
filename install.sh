mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build . --parallel
cmake --install .

systemctl --user daemon-reload
systemctl --user enable --now gsus.service