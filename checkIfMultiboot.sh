if grub-file --is-x86-multiboot image/kernel.elf; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi