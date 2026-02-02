# gsus

gsus saves all.

The application is a systemd service that two-way syncs a local folder with a remote Google Drive.
It syncs all files locally and does not povide any on-demand access.

# Dependencies
## Build
```bash
build-essential
cmake
libsystemd-dev
```

## Development
```bash
vscode
```

# Getting started
## Build and install
```bash
mkdir -p build && cd build
cmake ..
make
sudo cmake --install .
```

## Enable service
```bash
sudo systemctl daemon-reload
sudo systemctl enable --now gsus.service
```

## Debugging
```bash
sudo systemctl status gsus.service -l
sudo journalctl -u gsus.service -n 200 --no-pager
```
