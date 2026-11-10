#!/bin/sh
# Installs the build dependencies of Gwenhywfar on a GitHub-hosted Ubuntu
# or macOS runner. The arguments name the GUIs to build (for example
# "gtk3 qt5"); the packages for those GUIs are installed as well.
set -eu

guis="$*"

case "$(uname -s)" in
  Linux)
    pkgs="autoconf automake libtool gettext pkg-config
          libgnutls28-dev libgcrypt20-dev libgpg-error-dev"
    for g in $guis; do
      case "$g" in
        none) ;;
        gtk2) pkgs="$pkgs libgtk2.0-dev" ;;
        gtk3) pkgs="$pkgs libgtk-3-dev" ;;
        qt5)  pkgs="$pkgs qtbase5-dev" ;;
        qt6)  pkgs="$pkgs qt6-base-dev" ;;
        *) echo "No Ubuntu packages known for GUI '$g'" >&2; exit 1 ;;
      esac
    done
    sudo apt-get update -q
    # shellcheck disable=SC2086
    sudo DEBIAN_FRONTEND=noninteractive \
      apt-get install -y -q --no-install-recommends $pkgs
    ;;
  Darwin)
    pkgs="autoconf automake libtool gettext pkgconf
          gnutls libgcrypt libgpg-error"
    for g in $guis; do
      case "$g" in
        none | cocoa) ;;
        *) echo "No Homebrew packages known for GUI '$g'" >&2; exit 1 ;;
      esac
    done
    # shellcheck disable=SC2086
    brew install --quiet $pkgs
    # Makefile.cvs calls "libtoolize", which Homebrew installs as
    # "glibtoolize" with the plain name in libexec/gnubin.
    echo "$(brew --prefix libtool)/libexec/gnubin" >>"$GITHUB_PATH"
    # Homebrew's gettext keeps gettext.m4 (AM_GNU_GETTEXT) outside the
    # search path of aclocal.
    echo "ACLOCAL_PATH=$(brew --prefix gettext)/share/gettext/m4${ACLOCAL_PATH:+:$ACLOCAL_PATH}" >>"$GITHUB_ENV"
    ;;
  *)
    echo "Unsupported system: $(uname -s)" >&2
    exit 1
    ;;
esac
