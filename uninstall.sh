#!/bin/bash

if [ ! -f build/generic/Makefile ]; then
	echo "Error: Could not find the build files."
	exit 1
fi

pushd build/generic

sudo su -c "make uninstall && gtk-update-icon-cache /usr/share/icons/hicolor/ &>/dev/null && update-desktop-database &>/dev/null" &&
	 echo "" && echo "Uninstalled successfully!" && echo ""

popd
