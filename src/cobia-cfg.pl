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
my $netconf;

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
    $vm_env = "$VMConfig::default_settings{vm_dir}/$vm_name";
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
    while (my ($key, $val) = each %VMConfig::default_settings) {
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

sub write_vm_yaml {
    my @vm = (
        arch        => $VMConfig::default_settings{arch},
        boot        => $VMConfig::default_settings{boot},
        cpu         => $VMConfig::default_settings{cpu},
        graphics    => $VMConfig::default_settings{graphics},
        model       => $VMConfig::default_settings{model},
        name        => $vm_name,
        net         => $VMConfig::default_settings{net},
        ram         => $VMConfig::default_settings{ram},
        ssh_key     => $VMConfig::default_settings{ssh_key},
        user        => $VMConfig::default_settings{user},
        vm_dir      => $vm_env
    );

    my $vm_yaml = YAML::Tiny->new;
    $vm_yaml->[0]->{domain} = { @vm };
    my $filename = "$vm_name".".yml";
    $vm_yaml->write( $filename );
}

sub create_virtual_machine {
    print(STDOUT "\n>>> Installing virtual machine.\n");
    my $virt_str = "cobia -n $vm_name"
    . " --arch $VMConfig::default_settings{arch}"
    . " --cpu $VMConfig::default_settings{cpu}"
    . " --disk $vm_env/$diskname"
    . " --cd $vm_env/cidata.iso"
    . " --net $VMConfig::default_settings{net}"
    . " --model $VMConfig::default_settings{model}"
    . " --graphics $VMConfig::default_settings{graphics}"
    . " --memory $VMConfig::default_settings{ram}"
    . " --create |";
    
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
        open IP, "cobia -i $vm_name |" or die "$!";
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
    IMGDownload::get_disk_image($vm_image_urls{$vm_image_choice});
    create_vm_disk();
    
    ## Cloud Init Config
    $ssh_key = CloudInit::get_pubkey();
    CloudInit::configure($username,$hostname,$ssh_key);
    CloudInit::create_cidata();

    ## Create VM
    write_vm_yaml();
    create_virtual_machine();
    get_ip_address();
    return 1;
}

prepare();
GetOptions(
    "arch=s"        => \$VMConfig::default_settings{arch},
    "boot=s"        => \$VMConfig::default_settings{boot},
    "cpu=s"         => \$VMConfig::default_settings{cpu},
    "graphics=s"    => \$VMConfig::default_settings{graphics},
    "hostname=s"    => \$hostname,
    "model=s"       => \$VMConfig::default_settings{model},
    "name=s"        => \$vm_name,
    "net=s"         => \$VMConfig::default_settings{net},
    "os=s"          => \$vm_image_choice,
    "ram=s"         => \$VMConfig::default_settings{ram},
    "size=s"        => \$disksize,
    "sshkey=s"      => \$VMConfig::default_settings{ssh_key},
    "username=s"    => \$VMConfig::default_settings{user},

    "create"        => \$create,
    "debug"         => \$debug,
    "delete"        => \$delete,
    "netconf"       => \$netconf,

    "interactive=s" => \$interactive,
);

if ($create or $netconf) {
    main();
}
