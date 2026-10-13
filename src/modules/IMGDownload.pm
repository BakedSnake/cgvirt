#!/usr/bin/perl

package IMGDownload;

use strict;
use warnings;
use WWW::Curl::Easy;
use YAML::Tiny;
use Cobia::VMConfig;

sub select_image {
    my $interactive = $_[0];

    VMConfig::get_os_images_urls();
    print(STDOUT "\n>>> Choose OS:\n");
    my $count = 0;
    for my $name (@VMConfig::os_img_names) {
        if ($count != 0) {
            printf(STDOUT "  ${count}. $name\n");
        }

        $count++;
    }

    if ($interactive or $VMConfig::vm_image_choice eq "") {
        print(STDOUT "\n  Select OS image: ");
        $VMConfig::vm_image_choice = <STDIN>;
        chomp $VMConfig::vm_image_choice;
    } else {
        print(STDOUT "\n  Your choice: $VMConfig::vm_image_choice\n\n");
    }
}

sub get_disk_image {
    my $memory;
    my $curl = WWW::Curl::Easy->new;
    my $url = $VMConfig::os_img_urls[$VMConfig::vm_image_choice];

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
        print(STDOUT "\n>>> Transfer Ok!\n");
        print(STDOUT ">>> Received response: $rc\n\n");
    
        open IMG, "> os_img.qcow2" or die "Could not open file: $!";
        print(IMG "$res");
        close IMG;
    } else {
        print(STDERR ">>> An error happened: $ret ".$curl->strerror($ret)." ".$curl->errbuf."\n");
    }
}

1;
