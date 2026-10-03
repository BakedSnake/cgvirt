#!/usr/bin/perl

package IMGDownload;

use strict;
use warnings;
use WWW::Curl::Easy;

sub select_image {
    my $os_choice = $_[0];
    my $interactive = $_[1];
    my @os_images = qw( Ubuntu Debian OpenSuse );
    my %os_img_urls = (
        #1 => "https://cloud-images.ubuntu.com/focal/current/focal-server-cloudimg-amd64.img",
        1 => "https://cloud-images.ubuntu.com/noble/20260926/noble-server-cloudimg-amd64.img",
        2 => "https://chuangtzu.ftp.acc.umu.se/images/cloud/trixie/20260914-2601/debian-13-generic-amd64-20260914-2601.qcow2",
        3 => "https://download.opensuse.org/distribution/openSUSE-stable/appliances/Leap-16.0-Minimal-VM.x86_64-Cloud.qcow2",
        4 => "https://distfiles.gentoo.org/releases/arm64/autobuilds/current-di-arm64-console/di-arm64-console-20260913T234554Z.qcow2",
        5 => "https://distfiles.gentoo.org/releases/amd64/autobuilds/current-di-amd64-console/di-amd64-console-20260927T170058Z.qcow2"
    );

    print(STDOUT "\n>>> Choose OS:\n");
    printf(STDOUT "  1. Ubuntu\n  2. Debian\n  3. OpenSuse\n  4. Gentoo ( arm64 )\n  5. Gentoo ( amd64 )\n\nYour answer: ");
    if ($interactive or $os_choice eq "") {
        $os_choice = 2;
        $os_choice = <STDIN>;
        chomp $os_choice;
    } else { print(STDOUT "$os_choice\n"); }

    if ($os_choice == "1") {
        print(STDOUT "\nDownloading:\n  Ubuntu\n  - $os_img_urls{1}\n\n");
    } elsif ($os_choice == "2") {
        print(STDOUT "\nDownloading:\n  Debian\n  - $os_img_urls{2}\n\n");
    } elsif ($os_choice == "3") {
        print(STDOUT "\nDownloading:\n  OpenSuse\n  - $os_img_urls{3}\n\n");
    } elsif ($os_choice == "4") {
        print(STDOUT "\nDownloading:\n  Gentoo ( arm64 )\n  - $os_img_urls{4}\n\n");
    } else {
        print(STDOUT "\nDownloading:\n  Gentoo ( amd64 )\n  - $os_img_urls{5}\n\n");
    }

    return ($os_choice, %os_img_urls);
}

sub get_disk_image {
    my $os_image_url = $_[0];
    my $memory;
    my $curl = WWW::Curl::Easy->new;
    $curl->setopt(CURLOPT_HEADER, 0);
    $curl->setopt(CURLOPT_URL, $os_image_url);
    $curl->setopt(CURLOPT_FOLLOWLOCATION, 1);
    $curl->setopt(CURLOPT_NOPROGRESS, 0);
    $curl->setopt(CURLOPT_PROGRESSDATA, $memory);

    my $res;
    $curl->setopt(CURLOPT_WRITEDATA, \$res);
    my $ret = $curl->perform;
    
    my $rc;
    if ($ret == 0) {
        $rc = $curl->getinfo(CURLINFO_HTTP_CODE);
        print(STDOUT ">>> Transfer Ok!\n");
        print(STDOUT ">>> Received response: $rc\n\n");
    
        open IMG, "> os_img.qcow2" or die "Could not open file: $!";
        print(IMG "$res");
        close IMG;
    } else {
        print(STDERR ">>> An error happened: $ret ".$curl->strerror($ret)." ".$curl->errbuf."\n");
    }
}

1;
