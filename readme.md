### CG's Virtual Machine Manager / Installer
Quickly whip up a Cloud OS virtual machine with qemu and libvirt.

#### Dependencies
- qemu
- libvirt
- mkisofs (cdr-tools)

#### Install
Default location is **/usr/local/bin**. ( The current make file is broken and needs to be updated. ).
```bash
$ make
$ make install
$ make uninstall
$ make clean

```

#### Configuration
Can be configured on **~/.config/cgvirt/cgvirt.conf**.
```vimwiki
- vms_dir -> Directory to install virtual machines
- user -> Name of  VM user that will be created on install
- ssh_key -> Full path to your chosen ssh .pub key

```

Default values for vm install.
```bash
ram = 2048
cpu = 4
net = default
model = virtio
graphics = spice
boot = uefi
arch = x86_64

```

Note: These options assume you have a virtual network named default, and that it is enabled.

#### Usage
```bash
$ cobia-cfg --interactive --create
$ cobia-cfg --os=1 --create             # Create Debian vm with all default values
$ cobia -l domains                      # List domains
$ cobia -l networks                     # List networks
$ cobia -i <vm name>                    # Show vm info

```

