#!/bin/bash

mkdir -p build/generic
pushd build/generic

cmake -DCMAKE_BUILD_TYPE=Release ../.. && make
if [ $? -ne 0 ]; then
	echo "Could not build. Check the build log for details."
	popd
	exit 1
fi
sudo su -c "make install && gtk-update-icon-cache /usr/share/icons/hicolor/ &>/dev/null && update-desktop-database &>/dev/null"
echo ""
echo "Installed successfully!"
echo ""

popd
