#!/usr/bin/perl

use strict;
use warnings;

use Getopt::Long;
use POSIX qw(setuid waitpid);
use File::Path qw( make_path );

use Cobia::VMConfig;
use Cobia::CloudInit;
use Cobia::IMGDownload;
use Cobia::Network;

my $os_release  = VMConfig::get_os_release();
my $username    = $VMConfig::username;
my $config_file = $VMConfig::config_file;

my $uid;
my $gid;
my $vm_env;
my $vm_os;
my $ssh_key;

my $dosu;
my $interactive;
my $debug;
my $netconf;

my $apply;
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
    if ($netconf) {
        if (Network::fw_enabled()) {
            if (! Network::fw_check_dns()) {
                Network::fw_allow_dns();
                print(STDOUT "added dns rule to firewall\n");
            }

            if (! Network::fw_check_dhcp()) {
                Network::fw_allow_dhcp();
                print(STDOUT "added dhcp rule to firewall\n");
            }
        }

        if (! Network::iptables_rules_present()) {
            Network::accept_forwarding();
            Network::setup_postrouting();
            print(STDOUT "iptables were updated\n");
        }

        exit(0);
    }
    
    if ($VMConfig::vm_image_choice) {
        if ($VMConfig::vm_image_choice == 1) {
            $vm_os = "alpine";
        } elsif ($VMConfig::vm_image_choice == 2) {
            $vm_os = "debian13";
        } elsif ($VMConfig::vm_image_choice == 3) {
            $vm_os = "gentoo";
        } elsif ($VMConfig::vm_image_choice == 4) {
            $vm_os = "gentoo";
        } elsif ($VMConfig::vm_image_choice == 5) {
            $vm_os = "opensuse16.0";
        } elsif ($VMConfig::vm_image_choice == 6) {
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
        $VMConfig::domain_settings{name} = <STDIN>;
        chomp $VMConfig::domain_settings{name};
        print(STDOUT "\nVM Hostname: ");
        $VMConfig::domain_settings{hostname} = <STDIN>;
        chomp $VMConfig::domain_settings{hostname};
    } else {
        if ($VMConfig::domain_settings{name} eq "") {
            my $date;
            open DATE, "date -u +%s |" or die "$!";
            while (my $line = <DATE>) {
                $date = $line;
            }

            chomp $date;
            $VMConfig::domain_settings{name} = "$vm_os"."-$date";
            close DATE;
        }

        $VMConfig::domain_settings{hostname} = $VMConfig::domain_settings{name};
    }
    
    ## Create VM env
    $vm_env = "$VMConfig::domain_settings{vm_dir}/$VMConfig::domain_settings{name}";
    my @vm_path = make_path("$vm_env", {
        verbose => 0,
        mode => 0777,
    }) unless ( -d $vm_env );
    
    $uid = getpwnam("$VMConfig::username");
    $gid = getgrnam("libvirt");
    chown($uid, $gid, $vm_env);

    ## Display Configuration
    if ($debug) {
        print(STDOUT ">>> Using the following configuration:\n");
        print(STDOUT "  name => $VMConfig::domain_settings{name}\n");
        while (my ($key, $val) = each %VMConfig::domain_settings) {
            print(STDOUT "  $key => $val\n");
        }
    }

    ## Select Cloud image OS
    $VMConfig::vm_image_choice = IMGDownload::select_image($interactive);
}

sub create_vm_disk {
    my $diskname = "$VMConfig::domain_settings{name}.qcow2";
    my $disksize = $VMConfig::domain_settings{disksize};
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

sub write_vm_yaml {
    $VMConfig::domain_settings{cd} = "$vm_env/$VMConfig::domain_settings{cd}";
    $VMConfig::domain_settings{disk} = "$vm_env/$VMConfig::domain_settings{name}.qcow2";
    $VMConfig::domain_settings{vm_dir} = "$vm_env";

    my @vm = (
        os          => {
            efi         => "false",
            arch        => $VMConfig::domain_settings{arch},
            machine     => $VMConfig::domain_settings{machine},
            features    => {
                secure_boot => $VMConfig::domain_settings{secboot},
            },
            bootdev     => $VMConfig::domain_settings{bootdev},

        },
        boot        => $VMConfig::domain_settings{boot},
        cpu         => $VMConfig::domain_settings{cpu},
        devices     => {
            cd      => $VMConfig::domain_settings{cd},
            disk    => {
                source  => $VMConfig::domain_settings{disk},
                size    => $VMConfig::domain_settings{disksize}
            },
            graphics=> $VMConfig::domain_settings{graphics},
            net     => $VMConfig::domain_settings{net},
        },
        model       => $VMConfig::domain_settings{model},
        name        => $VMConfig::domain_settings{name},
        ram         => $VMConfig::domain_settings{ram},
        ssh_key     => $VMConfig::domain_settings{ssh_key},
        user        => $VMConfig::domain_settings{user},
        vm_dir      => $VMConfig::domain_settings{vm_dir}
    );

    my $vm_yaml = YAML::Tiny->new;
    $vm_yaml->[0]->{domain} = { @vm };
    my $filename = "$VMConfig::domain_settings{name}".".yml";
    $vm_yaml->write( $filename );
}

sub create_virtual_machine {
    print(STDOUT "\n>>> Installing virtual machine.\n");
    my $virt_str = "cobia -c $vm_env/$VMConfig::domain_settings{name}.yml |";

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
        open IP, "cobia -i $VMConfig::domain_settings{name} |" or die "$!";
        while (my $line = <IP>) {
            if ($line =~ m/IP/) {
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
    IMGDownload::get_disk_image();
    create_vm_disk();
    
    ## Cloud Init Config
    $ssh_key = CloudInit::get_pubkey();
    CloudInit::configure($username,$VMConfig::domain_settings{hostname},$ssh_key);
    CloudInit::create_cidata();

    ## Create VM
    if ($apply) {
        write_vm_yaml();
    }

    if ($create) {
        write_vm_yaml();
        create_virtual_machine();
        get_ip_address();
    }

    return 1;
}

prepare();
GetOptions(
    "arch=s"        => \$VMConfig::domain_settings{arch},
    "boot=s"        => \$VMConfig::domain_settings{boot},
    "bootdev=s"     => \$VMConfig::domain_settings{bootdev},
    "cpu=s"         => \$VMConfig::domain_settings{cpu},
    "graphics=s"    => \$VMConfig::domain_settings{graphics},
    "hostname=s"    => \$VMConfig::domain_settings{hostname},
    "machine=s"     => \$VMConfig::domain_settings{machine},
    "model=s"       => \$VMConfig::domain_settings{model},
    "name=s"        => \$VMConfig::domain_settings{name},
    "net=s"         => \$VMConfig::domain_settings{net},
    "os=s"          => \$VMConfig::vm_image_choice,
    "ram=s"         => \$VMConfig::domain_settings{ram},
    "secboot=s"     => \$VMConfig::domain_settings{secboot},
    "size=s"        => \$VMConfig::domain_settings{disksize},
    "sshkey=s"      => \$VMConfig::domain_settings{ssh_key},
    "username=s"    => \$VMConfig::domain_settings{user},

    "create"        => \$create,
    "debug"         => \$debug,
    "delete"        => \$delete,
    "netconf"       => \$netconf,
    "apply"         => \$apply,

    "interactive=s" => \$interactive,
);

if ($apply or $create or $netconf) {
    main();
}
