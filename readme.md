<div align="center">
  <h1>Lõi VNExos Bản Nguyên</h1>
  <p><em>VNExos Apeiron Core</em></p>
</div>

### ✨ Cài đặt môi trường
- Cài đặt QEMU và kiến trúc chip bổ sung
```bash
sudo pacman -S qemu-base qemu-system-aarch64 qemu-system-riscv
```

- Cài đặt gói công cụ hiển thị đồ họa
```bash
sudo pacman -S qemu-ui-gtk qemu-ui-sdl qemu-audio-alsa qemu-audio-pa
```

- Cài đặt bộ firmware UEFI
```bash
sudo pacman -S edk2-ovmf edk2-aarch64 edk2-riscv64
```