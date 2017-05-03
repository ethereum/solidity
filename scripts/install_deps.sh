#!/usr/bin/env sh

#------------------------------------------------------------------------------
# Shell script for installing pre-requisite packages for solidity on a
# variety of Linux and other UNIX-derived platforms.
#
# This is an "infrastucture-as-code" alternative to the manual build
# instructions pages which we previously maintained at:
# http://solidity.readthedocs.io/en/latest/installing-solidity.html
#
# The aim of this script is to simplify things down to the following basic
# flow for all supported operating systems:
#
# - git clone --recursive
# - ./scripts/install_deps.sh
# - cmake && make
#
# TODO - There is no support here yet for cross-builds in any form, only
# native builds.  Expanding the functionality here to cover the mobile,
# wearable and SBC platforms covered by doublethink and EthEmbedded would
# also bring in support for Android, iOS, watchOS, tvOS, Tizen, Sailfish,
# Maemo, MeeGo and Yocto.
#
# The documentation for solidity is hosted at:
#
# http://solidity.readthedocs.io/
#
# ------------------------------------------------------------------------------
# This file is part of solidity.
#
# solidity is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# solidity is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with solidity.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2016 solidity contributors.
#------------------------------------------------------------------------------

set -e

# Check for 'uname' and abort if it is not available.
uname -v > /dev/null 2>&1 || { echo >&2 "ERROR - solidity requires 'uname' to identify the platform."; exit 1; }

# See http://unix.stackexchange.com/questions/92199/how-can-i-reliably-get-the-operating-systems-name
detect_linux_distro() {
    if [ $(command -v lsb_release) ]; then
        DISTRO=$(lsb_release -is)
    elif [ -f /etc/os-release ]; then
        # extract 'foo' from NAME=foo, only on the line with NAME=foo
        DISTRO=$(sed -n -e 's/^NAME="\(.*\)\"/\1/p' /etc/os-release)
    elif [ -f /etc/centos-release ]; then
        DISTRO=CentOS
    else
        DISTRO=''
    fi
    echo $DISTRO
}

case $(uname -s) in

#------------------------------------------------------------------------------
# macOS
#------------------------------------------------------------------------------

    Darwin)
        case $(sw_vers -productVersion | awk -F . '{print $1"."$2}') in
            10.9)
                echo "Installing solidity dependencies on OS X 10.9 Mavericks."
                ;;
            10.10)
                echo "Installing solidity dependencies on OS X 10.10 Yosemite."
                ;;
            10.11)
                echo "Installing solidity dependencies on OS X 10.11 El Capitan."
                ;;
            10.12)
                echo "Installing solidity dependencies on macOS 10.12 Sierra."
                ;;
            *)
                echo "Unsupported macOS version."
                echo "We only support Mavericks, Yosemite and El Capitan, with work-in-progress on Sierra."
                exit 1
                ;;
        esac

        # Check for Homebrew install and abort if it is not installed.
        brew -v > /dev/null 2>&1 || { echo >&2 "ERROR - solidity requires a Homebrew install.  See http://brew.sh."; exit 1; }
        brew update
        brew install boost
        brew install cmake
        if [ "$CI" = true ]; then
            brew upgrade cmake
            brew tap ethereum/ethereum
            brew install cpp-ethereum
            brew linkapps cpp-ethereum
        else
            brew upgrade
        fi

        ;;

#------------------------------------------------------------------------------
# FreeBSD
#------------------------------------------------------------------------------

    FreeBSD)
        echo "Installing solidity dependencies on FreeBSD."
        echo "ERROR - 'install_deps.sh' doesn't have FreeBSD support yet."
        echo "Please let us know if you see this error message, and we can work out what is missing."
        echo "Drop us a message at https://gitter.im/ethereum/solidity."
        exit 1
        ;;

#------------------------------------------------------------------------------
# Linux
#------------------------------------------------------------------------------

    Linux)
        case $(detect_linux_distro) in

#------------------------------------------------------------------------------
# Arch Linux
#------------------------------------------------------------------------------

            Arch)
                #Arch
                echo "Installing solidity dependencies on Arch Linux."

                # All our dependencies can be found in the Arch Linux official repositories.
                # See https://wiki.archlinux.org/index.php/Official_repositories
                # Also adding ethereum-git to allow for testing with the `eth` client
                sudo pacman -Sy \
                    base-devel \
                    boost \
                    cmake \
                    git \
                    ethereum-git \
                ;;

#------------------------------------------------------------------------------
# Alpine Linux
#------------------------------------------------------------------------------

            "Alpine Linux")
                #Alpine
                echo "Installing solidity dependencies on Alpine Linux."

                # All our dependencies can be found in the Alpine Linux official repositories.
                # See https://pkgs.alpinelinux.org/

                apk update
                apk add boost-dev build-base cmake

                ;;

#------------------------------------------------------------------------------
# Debian
#------------------------------------------------------------------------------

            Debian)
                #Debian
                case $(lsb_release -cs) in
                    wheezy)
                        #wheezy
                        echo "Installing solidity dependencies on Debian Wheezy (7.x)."
                        echo "ERROR - 'install_deps.sh' doesn't have Debian Wheezy support yet."
                        echo "See http://solidity.readthedocs.io/en/latest/installing-solidity.html for manual instructions."
                        echo "If you would like to get 'install_deps.sh' working for Debian Wheezy, that would be fantastic."
                        echo "Drop us a message at https://gitter.im/ethereum/solidity."
                        echo "See also https://github.com/ethereum/webthree-umbrella/issues/495 where we are working through Alpine support."
                        exit 1
                        ;;
                    jessie)
                        #jessie
                        echo "Installing solidity dependencies on Debian Jesse (8.x)."
                        ;;
                    stretch)
                        #stretch
                        echo "Installing solidity dependencies on Debian Stretch (9.x)."
                        echo "ERROR - 'install_deps.sh' doesn't have Debian Stretch support yet."
                        echo "See http://solidity.readthedocs.io/en/latest/installing-solidity.html for manual instructions."
                        echo "If you would like to get 'install_deps.sh' working for Debian Stretch, that would be fantastic."
                        echo "Drop us a message at https://gitter.im/ethereum/solidity."
                        exit 1
                        ;;
                    *)
                        #other Debian
                        echo "Installing solidity dependencies on unknown Debian version."
                        echo "ERROR - Debian Jessie is the only Debian version which solidity has been tested on."
                        echo "If you are using a different release and would like to get 'install_deps.sh'"
                        echo "working for that release that would be fantastic."
                        echo "Drop us a message at https://gitter.im/ethereum/solidity."
                        exit 1
                        ;;
                esac

                # Install "normal packages"
                sudo apt-get -y update
                sudo apt-get -y install \
                    build-essential \
                    cmake \
                    g++ \
                    gcc \
                    git \
                    libboost-all-dev \
                    unzip

                ;;

#------------------------------------------------------------------------------
# Fedora
#------------------------------------------------------------------------------

            Fedora)
                #Fedora
                echo "Installing solidity dependencies on Fedora."

                # Install "normal packages"
                # See https://fedoraproject.org/wiki/Package_management_system.
                dnf install \
                    autoconf \
                    automake \
                    boost-devel \
                    cmake \
                    gcc \
                    gcc-c++ \
                    git \
                    libtool

                ;;

#------------------------------------------------------------------------------
# OpenSUSE
#------------------------------------------------------------------------------

            "openSUSE project")
                #openSUSE
                echo "Installing solidity dependencies on openSUSE."
                echo "ERROR - 'install_deps.sh' doesn't have openSUSE support yet."
                echo "See http://solidity.readthedocs.io/en/latest/installing-solidity.html for manual instructions."
                echo "If you would like to get 'install_deps.sh' working for openSUSE, that would be fantastic."
                echo "See https://github.com/ethereum/webthree-umbrella/issues/552."
                exit 1
                ;;

#------------------------------------------------------------------------------
# Ubuntu
#
# TODO - I wonder whether all of the Ubuntu-variants need some special
# treatment?
#
# TODO - We should also test this code on Ubuntu Server, Ubuntu Snappy Core
# and Ubuntu Phone.
#
# TODO - Our Ubuntu build is only working for amd64 and i386 processors.
# It would be good to add armel, armhf and arm64.
# See https://github.com/ethereum/webthree-umbrella/issues/228.
#------------------------------------------------------------------------------

            Ubuntu)
                #Ubuntu
                case $(lsb_release -cs) in
                    trusty)
                        #trusty
                        echo "Installing solidity dependencies on Ubuntu Trusty Tahr (14.04)."
                        ;;
                    utopic)
                        #utopic
                        echo "Installing solidity dependencies on Ubuntu Utopic Unicorn (14.10)."
                        ;;
                    vivid)
                        #vivid
                        echo "Installing solidity dependencies on Ubuntu Vivid Vervet (15.04)."
                        ;;
                    wily)
                        #wily
                        echo "Installing solidity dependencies on Ubuntu Wily Werewolf (15.10)."
                        ;;
                    xenial)
                        #xenial
                        echo "Installing solidity dependencies on Ubuntu Xenial Xerus (16.04)."
                        ;;
                    yakkety)
                        #yakkety
                        echo "Installing solidity dependencies on Ubuntu Yakkety Yak (16.10)."
                        echo ""
                        echo "NOTE - You are in unknown territory with this preview OS."
                        echo "We will need to update the Ethereum PPAs, work through build and runtime breaks, etc."
                        echo "See https://github.com/ethereum/webthree-umbrella/issues/624."
                        echo "If you would like to partner with us to work through these, that"
                        echo "would be fantastic.  Please just comment on that issue.  Thanks!"
                        ;;
                    *)
                        #other Ubuntu
                        echo "ERROR - Unknown or unsupported Ubuntu version (" $(lsb_release -cs) ")"
                        echo "We only support Trusty, Utopic, Vivid, Wily and Xenial, with work-in-progress on Yakkety."
                        exit 1
                        ;;
                esac

                sudo apt-get -y update
                sudo apt-get -y install \
                    build-essential \
                    cmake \
                    git \
                    libboost-all-dev
                if [ "$CI" = true ]; then
                    # Install 'eth', for use in the Solidity Tests-over-IPC.
                    # We will not use this 'eth', but its dependencies
                    sudo add-apt-repository -y ppa:ethereum/ethereum
                    sudo add-apt-repository -y ppa:ethereum/ethereum-dev
                    sudo apt-get -y update
                    sudo apt-get -y install eth
                fi
                ;;

#------------------------------------------------------------------------------
# CentOS
# CentOS needs some more testing. This is the general idea of packages
# needed, but some tweaking/improvements can definitely happen
#------------------------------------------------------------------------------
            CentOS)
                read -p "This script will heavily modify your system in order to allow for compilation of Solidity. Are you sure? [Y/N]" -n 1 -r
                if [[ $REPLY =~ ^[Yy]$ ]]; then
                    # Make Sure we have the EPEL repos
                    sudo yum -y install epel-release
                    # Get g++ 4.8
                    sudo rpm --import http://ftp.scientificlinux.org/linux/scientific/5x/x86_64/RPM-GPG-KEYs/RPM-GPG-KEY-cern
                    wget -O /etc/yum.repos.d/slc6-devtoolset.repo http://linuxsoft.cern.ch/cern/devtoolset/slc6-devtoolset.repo
                    sudo yum -y install devtoolset-2-gcc devtoolset-2-gcc-c++ devtoolset-2-binutils

                    # Enable the devtoolset2 usage so global gcc/g++ become the 4.8 one.
                    # As per https://gist.github.com/stephenturner/e3bc5cfacc2dc67eca8b, what you should do afterwards is
                    # to add this line:
                    # source /opt/rh/devtoolset-2/enable
                    # to your bashrc so that this happens automatically at login
                    scl enable devtoolset-2 bash

                    # Get cmake
                    sudo yum -y remove cmake
                    sudo yum -y install cmake3
                    sudo ln -s /usr/bin/cmake3 /usr/bin/cmake

                    # Get latest boost thanks to this guy: http://vicendominguez.blogspot.de/2014/04/boost-c-library-rpm-packages-for-centos.html
                    sudo yum -y remove boost-devel
                    sudo wget http://repo.enetres.net/enetres.repo -O /etc/yum.repos.d/enetres.repo
                    sudo yum install boost-devel
                else
                    echo "Aborted CentOS Solidity Dependency Installation";
                    exit 1
                fi

                ;;




            *)

#------------------------------------------------------------------------------
# Other (unknown) Linux
# Major and medium distros which we are missing would include Mint, CentOS,
# RHEL, Raspbian, Cygwin, OpenWrt, gNewSense, Trisquel and SteamOS.
#------------------------------------------------------------------------------

                #other Linux
                echo "ERROR - Unsupported or unidentified Linux distro."
                echo "See http://solidity.readthedocs.io/en/latest/installing-solidity.html for manual instructions."
                echo "If you would like to get your distro working, that would be fantastic."
                echo "Drop us a message at https://gitter.im/ethereum/solidity."
                exit 1
                ;;
        esac
        ;;

#------------------------------------------------------------------------------
# Other platform (not Linux, FreeBSD or macOS).
# Not sure what might end up here?
# Maybe OpenBSD, NetBSD, AIX, Solaris, HP-UX?
#------------------------------------------------------------------------------

    *)
        #other
        echo "ERROR - Unsupported or unidentified operating system."
        echo "See http://solidity.readthedocs.io/en/latest/installing-solidity.html for manual instructions."
        echo "If you would like to get your operating system working, that would be fantastic."
        echo "Drop us a message at https://gitter.im/ethereum/solidity."
        ;;
esac
