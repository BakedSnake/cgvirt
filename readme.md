### CG's Virtual Machine Manager / Installer
Quickly whip up a Cloud OS virtual machine with qemu and libvirt.

#### Dependencies
- qemu
- libvirt
- virt-manager
- mkisofs (cdr-tools)

#### Install
Default location is **/usr/local/bin**. ( The current make file is broken and needs to be updated. ).
```bash
$ make install # you may need root priviledges depending on $PREFIX.
$ make uninstall
```

#### Configuration
Can be configured on **~/.config/cgvirt/cgvirt.conf**.
```vimwiki
- vms_dir -> Directory to install virtual machines
- user -> Name of  VM user that will be created on install
- ssh_key -> Full path to your chosen ssh .pub key

```

Default values for virt-install.
```bash
ram = 2048
cpu = 4
net = default
model = virtio
graphics = spice
```

Note: These options assume you have a virtual network named default, and that it is enabled.

#### Usage
```bash
$ cgv --interactive --create

```

