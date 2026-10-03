#!/usr/bin/perl

package VMConfig;
use strict;
use warnings;

our $username = $ENV{LOGNAME};
our $config_file = "$ENV{HOME}/.config/cgvirt/cgvirt.conf";

our %virtual_machine = (
    arch        => "x86_64",
    boot        => "uefi",
    cpu         => "2",
    graphics    => "spice",
    model       => "virtio",
    net         => "default",
    ram         => "2096",
    ssh_key     => "",
    user        => "$username",
    vm_dir      => ""
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
    open CONFIG, "< $config_file" or die "Could not open file: $!";
    while (my $line = <CONFIG>) {
        foreach my $key (keys %virtual_machine) {
            if ($line =~ qr/^$key/) {
                my @parts;
                if ($line =~ qr/ = /) {
                    @parts = split(" = ", $line);
                } else {
                    @parts = split("=", $line);
                }
    
                chomp $parts[1];
                $virtual_machine{$key} = $parts[1];
            }
        }
    }

    close CONFIG;
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
