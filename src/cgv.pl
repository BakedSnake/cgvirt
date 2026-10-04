#!/usr/bin/perl

use strict;
use warnings;

use Getopt::Long;
use POSIX qw(setuid waitpid);
use File::Path qw( make_path );

use CGVirt::VMConfig;
use CGVirt::CloudInit;
use CGVirt::IMGDownload;

my $os_release  = VMConfig::get_os_release();
my $username    = $VMConfig::username;
my $config_file = $VMConfig::config_file;

my $uid;
my $gid;
my $vm_env;
my $vm_name;
my $vm_os;
my $vm_image_choice = 1;
my %vm_image_urls;
my $hostname;
my $diskname;
my $disksize = 40;
my $ssh_key;

my $dosu;
my $interactive;
my $debug;

my $create;
my $delete;

sub prepare {
    if ($debug) {
        print(STDOUT ">>> Configuration\n");
        print(STDOUT "OS: $os_release\n");
        print(STDOUT "USER: $username\n");
        print(STDOUT "CONFIG: $config_file\n\n");
    }
    
    VMConfig::parse_config();
}

sub configure {
    if ($vm_image_choice) {
        if ($vm_image_choice == 1) {
            $vm_os = "debian13";
        } elsif ($vm_image_choice == 2) {
            $vm_os = "gentoo";
        } elsif ($vm_image_choice == 3) {
            $vm_os = "gentoo";
        } elsif ($vm_image_choice == 4) {
            $vm_os = "opensuse16.0";
        } elsif ($vm_image_choice == 5) {
            $vm_os = "ubuntu24.04";
        } else {
            $vm_os = "ubuntu20.04";
        }
    } else {
        print(STDERR "OS image not specified!\n");
        print(STDERR "Please either use interactive, or specify using the --os flag.\n");
    }

    if ($interactive) {
        print(STDOUT "\nVM Name: ");
        $vm_name = <STDIN>;
        chomp $vm_name;
        print(STDOUT "\nVM Hostname: ");
        $hostname = <STDIN>;
        chomp $hostname;
    } else {
        if (! $vm_name) {
            my $date;
            open DATE, "date -u +%s |" or die "$!";
            while (my $line = <DATE>) {
                $date = $line;
            }

            chomp $date;
            $vm_name = "$vm_os"."-$date";
            close DATE;
        }

        $hostname = $vm_name;
    }
    
    ## Create VM env
    $vm_env = "$VMConfig::virtual_machine{vm_dir}/$vm_name";
    my @vm_path = make_path("$vm_env", {
        verbose => 0,
        mode => 0777,
    }) unless ( -d $vm_env );
    
    $uid = getpwnam("$VMConfig::username");
    $gid = getgrnam("libvirt");
    chown($uid, $gid, $vm_env);

    ## Display Configuration
    print(STDOUT ">>> Using the following configuration:\n");
    print(STDOUT "  name => $vm_name\n");
    while (my ($key, $val) = each %VMConfig::virtual_machine) {
        print(STDOUT "  $key => $val\n");
    }

    ## Select Cloud image OS
    ($vm_image_choice, %vm_image_urls) = IMGDownload::select_image($vm_image_choice,$interactive);
}

sub create_vm_disk {
    $diskname = "$vm_name.qcow2";
    print(STDOUT ">>> Enter Disk Size ( in GBs ): ");
    if ($interactive or $disksize eq "") {
        $disksize = <STDIN>;
        chomp $disksize;
    } else { print(STDOUT "$disksize"."G\n"); }
    $disksize = $disksize."G";
    
    print(STDOUT ">>> Resizing disk.\n");
    open QEMU, "qemu-img create -b os_img.qcow2 -f qcow2 -F qcow2 \"$diskname\" $disksize |" or die "$!";
    while (my $line= <QEMU>) {
        my @diskopts = split(" ", $line);
        print(STDOUT " $diskopts[0] $diskopts[1]\n");
        shift(@diskopts);
        shift(@diskopts);
        foreach my $diskopt (@diskopts) {
            print(STDOUT " - $diskopt\n");
        }
    }
    close QEMU;
    print(STDOUT "\n>>> Image file created successfully.\n");
    print(STDOUT ">>> Updating permissions.\n");
    chown($uid, $gid, $diskname);
}

sub create_virtual_machine {
    print(STDOUT "\n>>> Installing virtual machine.\n");
    my $virt_str = "$dosu virt-install --name $vm_name"
    . " --arch=$VMConfig::virtual_machine{arch}"
    . " --boot=$VMConfig::virtual_machine{boot}"
    . " --ram=$VMConfig::virtual_machine{ram}"
    . " --vcpus=$VMConfig::virtual_machine{cpu}"
    . " --disk path=$diskname,format=qcow2"
    . " --disk path=cidata.iso,device=cdrom"
    . " --os-variant=$vm_os"
    . " --network network=$VMConfig::virtual_machine{net},model=$VMConfig::virtual_machine{model}"
    . " --graphics $VMConfig::virtual_machine{graphics}"
    . " --noautoconsole --import |";
    
    open VIRT, $virt_str or die "$!";
    while (my $line= <VIRT>) {
        print(STDOUT "$line");
    }
    close VIRT;
}

sub get_ip_address {
    print(STDOUT "\n>>> Waiting for network...\n\n");
    my $found = 0;
    while (!$found) {
        sleep 5;
        open IP, "$dosu virsh domifaddr $vm_name |" or die "$!";
        while (my $line = <IP>) {
            if ($line =~ m/ipv4/) {
                print(STDOUT " Name     MAC address         Protocol   Address\n");
                print(STDOUT "-------------------------------------------------------------\n");
                print(STDOUT "$line\n");
                $found = 1;
            }
        }

        close IP;
    }

    print(STDOUT ">>> Done!\n");
}

sub main {
    configure();

    ## Get Cloud image
    chdir($vm_env);
    IMGDownload::get_disk_image($vm_image_urls{$vm_image_choice});
    create_vm_disk();
    
    ## Cloud Init Config
    $ssh_key = CloudInit::get_pubkey();
    CloudInit::configure($username,$hostname,$ssh_key);
    CloudInit::create_cidata();

    ## Create VM
    $dosu = VMConfig::get_sudo();
    create_virtual_machine();
    get_ip_address();
    return 1;
}

prepare();
GetOptions(
    "arch=s"        => \$VMConfig::virtual_machine{arch},
    "boot=s"        => \$VMConfig::virtual_machine{boot},
    "cpu=s"         => \$VMConfig::virtual_machine{cpu},
    "graphics=s"    => \$VMConfig::virtual_machine{graphics},
    "hostname=s"    => \$hostname,
    "model=s"       => \$VMConfig::virtual_machine{model},
    "name=s"        => \$vm_name,
    "net=s"         => \$VMConfig::virtual_machine{net},
    "os=s"          => \$vm_image_choice,
    "ram=s"         => \$VMConfig::virtual_machine{ram},
    "size=s"        => \$disksize,
    "sshkey=s"      => \$VMConfig::virtual_machine{ssh_key},
    "username=s"    => \$VMConfig::virtual_machine{user},

    "create"        => \$create,
    "debug"         => \$debug,
    "delete"        => \$delete,
    "interactive=s" => \$interactive,
);

if ($create) {
    main();
}
