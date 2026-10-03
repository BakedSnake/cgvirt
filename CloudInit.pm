#!/usr/bin/perl

package CloudInit;
use strict;
use warnings;

sub get_pubkey {
    my $u_key; 
    open KEY, "< $VMConfig::virtual_machine{ssh_key}" or die "$!";
    while (my $line = <KEY>) {
        $u_key = "$line\n";
    }

    close KEY;
    return $u_key;
}

sub configure {
    my ($username, $hostname, $u_key) = @_;
    open META, "> meta-data" or die "$!";
    print(META "instance-id: $hostname\nlocal-hostname: $hostname\n");
    close META;
    
    open USERD, "> user-data" or die "$!";
    print(USERD "#cloud-config

users:
  - name: $VMConfig::virtual_machine{user}
    lock_passwd: False
    password: $hostname
    sudo: ['ALL=(ALL) NOPASSWD:ALL']
    groups: sudo
    shell: /bin/bash
    ssh_authorized_keys:
      - $u_key 

chpasswd:
    list: |
        $hostname:$hostname
    expire: False
ssh_pwauth: True
    \n");
    close USERD;
}

sub create_cidata {
    open ISO, "mkisofs -output cidata.iso -V cidata -r -J user-data meta-data |" or die "$!";
    while (my $line= <ISO>) {
        print STDOUT "  $line";
    }
    close ISO;
}

1;
