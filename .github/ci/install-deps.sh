#!/bin/sh
# Installs the build dependencies of Gwenhywfar on a GitHub-hosted Ubuntu
# or macOS runner. The arguments name the GUIs to build (for example
# "gtk3 qt5"), and "openssl" for a build with --with-tls=openssl; the
# packages for those are installed as well. On macOS, "openssl" installs
# OpenSSL 3 and "openssl@4" OpenSSL 4.
set -eu

case "$(uname -s)" in
  Linux)
    pkgs="autoconf automake libtool gettext pkg-config
          libgnutls28-dev libgcrypt20-dev libgpg-error-dev"
    for g in "$@"; do
      case "$g" in
        none) ;;
        openssl) pkgs="$pkgs libssl-dev" ;;
        gtk2) pkgs="$pkgs libgtk2.0-dev" ;;
        gtk3) pkgs="$pkgs libgtk-3-dev" ;;
        qt5)  pkgs="$pkgs qtbase5-dev" ;;
        qt6)  pkgs="$pkgs qt6-base-dev" ;;
        *) echo "No Ubuntu packages known for '$g'" >&2; exit 1 ;;
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
    openssl=
    for g in "$@"; do
      case "$g" in
        none | cocoa) ;;
        openssl) openssl=openssl@3 ;;
        openssl@4) openssl=openssl@4 ;;
        *) echo "No Homebrew packages known for '$g'" >&2; exit 1 ;;
      esac
    done
    # shellcheck disable=SC2086
    brew install --quiet $pkgs $openssl
    # Makefile.cvs calls "libtoolize", which Homebrew installs as
    # "glibtoolize" with the plain name in libexec/gnubin.
    echo "$(brew --prefix libtool)/libexec/gnubin" >>"$GITHUB_PATH"
    # Homebrew's gettext keeps gettext.m4 (AM_GNU_GETTEXT) outside the
    # search path of aclocal.
    echo "ACLOCAL_PATH=$(brew --prefix gettext)/share/gettext/m4${ACLOCAL_PATH:+:$ACLOCAL_PATH}" >>"$GITHUB_ENV"
    # Homebrew keeps OpenSSL outside the search path of pkg-config.
    if test -n "$openssl"; then
      echo "PKG_CONFIG_PATH=$(brew --prefix "$openssl")/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" >>"$GITHUB_ENV"
    fi
    ;;
  *)
    echo "Unsupported system: $(uname -s)" >&2
    exit 1
    ;;
esac
