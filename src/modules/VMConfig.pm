#!/usr/bin/perl

package VMConfig;
use strict;
use warnings;

use YAML::Tiny;

our $username = $ENV{LOGNAME};
our $config_file = "$ENV{HOME}/.config/cobia/cobia.yml";

our @os_img_urls;
our @os_img_names;
our $vm_image_choice = 1;

my $yaml = YAML::Tiny->read("$config_file");
my $cfg = $yaml->[0];

our %domain_settings = (
    arch => "x86_64",
    boot => "uefi",
    bootdev => "hd",
    cd => "cidata.iso",
    cpu => "2",
    disk => "",
    disksize => "40",
    efi => "false",
    graphics => "spice",
    hostname => "",
    machine => "q35",
    model => "virtio",
    name => "",
    net => "default",
    ram => "2096",
    secboot => "no",
);

sub get_os_release {
    my $os_release;
    my $os_release_file = "/etc/os-release";

    open OS_RELEASE, "< $os_release_file" or die "Could not open file: $!";
    while (my $line = <OS_RELEASE>) {
        if ( $line =~ m/^ID/ ) {
            my @parts = split("=", $line);
            $os_release = substr($parts[1], 1, -2);
        }
    }

    close OS_RELEASE;
    return $os_release;
}

sub parse_config {
    $domain_settings{ssh_key}     = $cfg->[1]->{ssh_key};
    $domain_settings{user}        = $cfg->[1]->{user};
    $domain_settings{vm_dir}      = $cfg->[0]->{vm_dir};
}

sub get_os_images_urls {
    my $imgs = $cfg->[2]->{images};
    my $len = $cfg->[2]->{count};

    if ($len > 0) {
        for (my $i = 0; $i < $len; $i++) {
            $os_img_urls[$i+1] = $imgs->[$i]->{url};
            $os_img_names[$i+1] = $imgs->[$i]->{name};
        }
    }
}

sub get_sudo {
    my $dosu;
    my @sudo_paths = qw( /usr/bin/doas /usr/local/bin/doas /usr/bin/sudo );
    
    my $i = 0;
    while ($i < 3) {
        if (-X $sudo_paths[$i]) {
            $dosu = $sudo_paths[$i];
            $i = 3;
        }
    
        ++$i;
    }

    return $dosu;
}
