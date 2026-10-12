#!/usr/bin/perl

package IMGDownload;

use strict;
use warnings;
use WWW::Curl::Easy;
use YAML::Tiny;
use Cobia::VMConfig;

sub select_image {
    my $interactive = $_[1];

    VMConfig::get_os_images_urls();
    print(STDOUT "\n>>> Choose OS:\n");
    printf(STDOUT "  1. Debian\n  2. Gentoo ( arm64 )\n  3. Gentoo ( amd64 )\n  4. OpenSuse\n  5. Ubuntu Noble\n  6. Ubuntu Focal\n\n  Your answer: ");
    if ($interactive or $VMConfig::vm_image_choice eq "") {
        $VMConfig::vm_image_choice = <STDIN>;
        chomp $VMConfig::vm_image_choice;
    } else { print(STDOUT "$VMConfig::vm_image_choice\n"); }

    if ($VMConfig::vm_image_choice == "1") {
        print(STDOUT "\n>>> Downloading: Debian Trixie\n\n  - $VMConfig::os_img_urls[1]\n\n");
    } elsif ($VMConfig::vm_image_choice == "2") {
        print(STDOUT "\n>>> Downloading: Gentoo ( arm64 )\n\n  - $VMConfig::os_img_urls[2]\n\n");
    } elsif ($VMConfig::vm_image_choice == "3") {
        print(STDOUT "\n>>> Downloading: Gentoo ( amd64 )\n\n  - $VMConfig::os_img_urls[3]\n\n");
    } elsif ($VMConfig::vm_image_choice == "4") {
        print(STDOUT "\n>>> Downloading: OpenSuse Leap 16\n\n  - $VMConfig::os_img_urls[4]\n\n");
    } elsif ($VMConfig::vm_image_choice == "5") {
        print(STDOUT "\n>>> Downloading: Ubuntu Noble\n\n  - $VMConfig::os_img_urls[5]\n\n");
    } else {
        print(STDOUT "\n>>> Downloading: Ubuntu Focal\n\n  - $VMConfig::os_img_urls[6]\n\n");
    }
}

sub get_disk_image {
    my $memory;
    my $curl = WWW::Curl::Easy->new;
    my $url = $VMConfig::os_img_urls[$VMConfig::vm_image_choice]

    $curl->setopt(CURLOPT_HEADER, 0);
    $curl->setopt(CURLOPT_URL, $url);
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
