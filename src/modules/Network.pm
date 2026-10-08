#!/usr/bin/perl

package Network;
use strict;
use warnings;

my $host_iface;

my $dosu = VMConfig::get_sudo();
sub check_iptables {
    open MASQ, "$dosu iptables -t nat -L -n |" or die "$!";
    while (my $line = <MASQ>) {
        if ($line =~ m/MASQUERADE/) {
            print(STDOUT "$line");
        }
    }

    close(MASQ);
}

sub accept_forwarding {
    open FORWARD, "$dosu iptables -I FORWARD -i virbr0 -o $host_iface -j ACCEPT |" or die "$!";
    while (my $line = <FORWARD>) {
        print(STDOUT "$line");
    }

    close(FORWARD);
    open FORWARD, "$dosu iptables -I FORWARD -i $host_iface -o virbr0 -m state --state RELATED,ESTABLISHED -j ACCEPT |" or die "$!";
    while (my $line = <FORWARD>) {
        print(STDOUT "$line");
    }

    close(FORWARD);
}

sub sst {
}

sub setup_postrouting {
    # for now using default qemu network
    open POST, "$dosu iptables -t nat -A POSTROUTING -s 192.168.122.0/24 -o $host_iface -j MASQUERADE |" or die "$!";
    while (my $line = <FORWARD>) {
        print(STDOUT "$line");
    }

    close(POST);
}

1;
