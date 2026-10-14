### Cobia :: Virtual Machine Manager
Quickly whip up a Cloud OS virtual machine with qemu and libvirt.

#### Dependencies
- qemu
- libvirt
- mkisofs (cdr-tools)
- libxml2
- libyaml
- libwww-curl-perl
- libyaml-tiny-perl

#### Install
Default location is **/usr/local/bin**.
```bash
$ make
$ make install
$ make uninstall
$ make clean
```

#### Configuration
Can be configured on **~/.config/cobia/cobia.conf**.
```vimwiki
- vms_dir -> Directory to install virtual machines
- user -> Name of  VM user that will be created on install
- ssh_key -> Full path to your chosen ssh .pub key
```

Default values for vm install.
```yaml
- name: Default Settings
  arch: "x86_64"
  boot: "uefi"
  cpu: "2"
  graphics: "spice"
  model: "virtio"
  net: "default"
  ram: "2048"
```

Note: These options assume you have a virtual network named default, and that it is enabled.

#### Usage
```bash
$ cobia-cfg --interactive --create
$ cobia-cfg --os=1 --create             # Create Debian vm with all default values
$ cobia-cfg --os=1 --apply              # Create Debian domain yaml file
$ cobia -c /path/to/domain/file.yml     # Create vm from domain yaml file
$ cobia -l domains                      # List domains
$ cobia -l networks                     # List networks
$ cobia -i <vm name>                    # Show vm info
```

