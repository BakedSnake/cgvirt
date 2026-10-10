#!/usr/bin/perl

package Network;
use strict;
use warnings;

my $host_iface;

my $dosu = VMConfig::get_sudo();
sub check_iptables {
    my $iptables_masq = 0;
    open MASQ, "$dosu iptables -t nat -L -n |" or die "$!";
    while (my $line = <MASQ>) {
        # for now matching default qemu network
        if ($line =~ m/MASQUERADE/ and $line =~ m/192.168.122.0/) {
            print(STDOUT "$line");
            $iptables_masq = 1;
            last;
        }
    }

    close(MASQ);
    return $iptables_masq;
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

sub setup_postrouting {
    # for now using default qemu network
    open POST, "$dosu iptables -t nat -A POSTROUTING -s 192.168.122.0/24 -o $host_iface -j MASQUERADE |" or die "$!";
    while (my $line = <FORWARD>) {
        print(STDOUT "$line");
    }

    close(POST);
}

sub fw_enabled {
    my $enabled = 1;
    open FW, "$dosu ufw status |" or die "$!";
    while (my $line = <FW>) {
        if ($line =~ m/Status: inactive/) {
            $enabled = 0;
            last;
        }
    }

    close(FW);
    return $enabled;
}

sub fw_check_dns {
    my $dns_status = 0;
    open FW, "$dosu ufw status |" or die "$!";
    while (my $line = <FW>) {
        if ($line =~ m/53 on virbr0/) {
            $dns_status = 1;
            last;
        }
    }

    close(FW);
    return $dns_status;
}

sub fw_allow_dns {
    open FW, "$dosu ufw allow in on virbr0 to any port 53 |" or die "$!";
    while (my $line = <FW>) {
        print(STDOUT "$line");
    }

    close(FW);
}

sub fw_check_dhcp {
    my $dhcp_status = 0;
    open FW, "$dosu ufw status |" or die "$!";
    while (my $line = <FW>) {
        if ($line =~ m|67/udp on virbr0|) {
            $dhcp_status = 1;
            last;
        }
    }

    close(FW);
    return $dhcp_status;
}

sub fw_allow_dhcp {
    open FW, "$dosu ufw allow in on virbr0 to any port 67 proto udp |" or die "$!";
    while (my $line = <FW>) {
        print(STDOUT "$line");
    }

    close(FW);
}

1;
