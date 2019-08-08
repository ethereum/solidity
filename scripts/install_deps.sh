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
        DISTRO=$(sed -n -e 's/^NAME="\?\([^"]*\)"\?$/\1/p' /etc/os-release)
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
            10.13)
                echo "Installing solidity dependencies on macOS 10.13 High Sierra."
                ;;
            10.14)
                echo "Installing solidity dependencies on macOS 10.14 Mojave."
                ;;
            *)
                echo "Unsupported macOS version."
                echo "We only support Mavericks, Yosemite, El Capitan, Sierra, High Sierra and Mojave."
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
        echo "Drop us a message at https://gitter.im/ethereum/solidity-dev."
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

            Arch*|ManjaroLinux)
                #Arch
                echo "Installing solidity dependencies on Arch Linux."

                # All our dependencies can be found in the Arch Linux official repositories.
                # See https://wiki.archlinux.org/index.php/Official_repositories
                sudo pacman -Syu \
                    base-devel \
                    boost \
                    cmake \
                    git \
                    cvc4
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
                apk add boost-dev boost-static build-base cmake git

                ;;

#------------------------------------------------------------------------------
# Debian
#------------------------------------------------------------------------------

            Debian*)
                #Debian
                . /etc/os-release
                install_z3=""
                case $VERSION_ID in
                    7)
                        #wheezy
                        echo "Installing solidity dependencies on Debian Wheezy (7.x)."
                        echo "ERROR - 'install_deps.sh' doesn't have Debian Wheezy support yet."
                        echo "See http://solidity.readthedocs.io/en/latest/installing-solidity.html for manual instructions."
                        echo "If you would like to get 'install_deps.sh' working for Debian Wheezy, that would be fantastic."
                        echo "Drop us a message at https://gitter.im/ethereum/solidity-dev."
                        echo "See also https://github.com/ethereum/webthree-umbrella/issues/495 where we are working through Alpine support."
                        exit 1
                        ;;
                    8)
                        #jessie
                        echo "Installing solidity dependencies on Debian Jesse (8.x)."
                        ;;
                    9)
                        #stretch
                        echo "Installing solidity dependencies on Debian Stretch (9.x)."
                        install_z3="libz3-dev"
                        ;;
                    10)
                        #buster
                        echo "Installing solidity dependencies on Debian Buster (10.x)."
                        install_z3="libz3-dev"
                        ;;
                    *)
                        #other Debian
                        echo "Installing solidity dependencies on unknown Debian version."
                        echo "ERROR - This might not work, but we are trying anyway."
                        echo "Drop us a message at https://gitter.im/ethereum/solidity-dev"
                        install_z3="libz3-dev"
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
                    unzip \
                    "$install_z3"


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
                    boost-static \
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
#------------------------------------------------------------------------------

            Ubuntu|LinuxMint)
                #LinuxMint is a distro on top of Ubuntu.
                #Ubuntu
                install_z3=""
                case $(lsb_release -cs) in
                    trusty|qiana|rebecca|rafaela|rosa)
                        echo "Installing solidity dependencies on Ubuntu Trusty Tahr (14.04)."
                        echo "Or, you may also be running Linux Mint Qiana / Rebecca / Rafaela / Rosa (base: Ubuntu Trusty Tahr (14.04).)"
                        ;;
                    utopic)
                        echo "Installing solidity dependencies on Ubuntu Utopic Unicorn (14.10)."
                        ;;
                    vivid)
                        echo "Installing solidity dependencies on Ubuntu Vivid Vervet (15.04)."
                        ;;
                    wily)
                        echo "Installing solidity dependencies on Ubuntu Wily Werewolf (15.10)."
                        ;;
                    xenial|sarah|serena|sonya|sylvia)
                        echo "Installing solidity dependencies on Ubuntu Xenial Xerus (16.04)."
                        echo "Or, you may also be running Linux Mint Sarah / Serena / Sonya / Sylvia (base: Ubuntu Xenial Xerus (16.04).)"
                        install_z3="libz3-dev"
                        ;;
                    yakkety)
                        echo "Installing solidity dependencies on Ubuntu Yakkety Yak (16.10)."
                        install_z3="libz3-dev"
                        ;;
                    zesty)
                        echo "Installing solidity dependencies on Ubuntu Zesty (17.04)."
                        install_z3="libz3-dev"
                        ;;
                    artful)
                        echo "Installing solidity dependencies on Ubuntu Artful (17.10)."
                        install_z3="libz3-dev"
                        ;;
                    bionic)
                        echo "Installing solidity dependencies on Ubuntu Bionic (18.04)."
                        install_z3="libz3-dev"
                        ;;
                    betsy)
                        #do not try anything for betsy.
                        echo "Linux Mint Betsy is not supported at the moment as it runs off of Debian."
                        echo "We only support Sylvia, Sonya, Serena, Sarah, Rosa, Rafaela, Rebecca, and Qiana."
                        echo "See http://solidity.readthedocs.io/en/latest/installing-solidity.html for manual instructions."
                        echo "If you would like to get your distro working, that would be fantastic."
                        echo "Drop us a message at https://gitter.im/ethereum/solidity-dev."
                        exit 1
                        ;;
                    *)
                        #other Ubuntu
                        echo "ERROR - Unknown or unsupported Ubuntu version (" $(lsb_release -cs) ")"
                        echo "ERROR - This might not work, but we are trying anyway."
                        echo "Please drop us a message at https://gitter.im/ethereum/solidity-dev."
                        echo "We only support Trusty, Utopic, Vivid, Wily, Xenial, Yakkety, Zesty, Artful and Bionic."
                        install_z3="libz3-dev"
                        ;;
                esac

                sudo apt-get -y update
                sudo apt-get -y install \
                    build-essential \
                    cmake \
                    git \
                    libboost-all-dev \
                    "$install_z3"
                if [ "$CI" = true ]; then
                    # install Z3 from PPA if the distribution does not provide it
                    if ! dpkg -l libz3-dev > /dev/null 2>&1
                    then
                        sudo apt-add-repository -y ppa:hvr/z3
                        sudo apt-get -y update
                        sudo apt-get -y install libz3-dev
                    fi
                fi
                ;;

#------------------------------------------------------------------------------
# CentOS
# CentOS needs some more testing. This is the general idea of packages
# needed, but some tweaking/improvements can definitely happen
#------------------------------------------------------------------------------
            CentOS*)
                echo "Attention: CentOS 7 is currently not supported!";
                read -p "This script will heavily modify your system in order to allow for compilation of Solidity. Are you sure? [Y/N]" -n 1 -r
                if [[ $REPLY =~ ^[Yy]$ ]]; then
                    # Make Sure we have the EPEL repos
                    sudo yum -y install epel-release
                    # Get g++ 4.8
                    sudo rpm --import http://linuxsoft.cern.ch/cern/slc6X/i386/RPM-GPG-KEY-cern
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
                    sudo wget https://bintray.com/vicendominguez/CentOS6/rpm -O /etc/yum.repos.d/bintray-vicendominguez-CentOS6.repo
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
                echo "Drop us a message at https://gitter.im/ethereum/solidity-dev."
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
        echo "Drop us a message at https://gitter.im/ethereum/solidity-dev."
        ;;
esac
