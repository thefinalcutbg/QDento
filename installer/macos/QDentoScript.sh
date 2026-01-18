#!/bin/sh

#defining file paths
APP="$PWD/../files/QDento.app" #the compiled bundle
INSTALLER="$PWD/../compiled/qdento-macos.dmg"
QDEPLOY="$PWD/../../../Qt/6.10.0/macos/bin/macdeployqt" #macdeployqt location

#enforcing light theme:
defaults write $APP/Contents/Info.plist NSRequiresAquaSystemAppearance -bool yes 

#deploying the app
$QDEPLOY $APP -sign-for-notarization="Developer ID Application: Hristo Konstantinov"

codesign --options runtime -s "Developer ID Application: Hristo Konstantinov"  -f --timestamp $APP

#deleting the old dmg installer
rm -R $INSTALLER

#creating dmg installer using create-dmg
create-dmg \
  --volname "QDento Installer" \
  --volicon "icon.icns" \
  --background "background.png" \
  --window-pos 200 110 \
  --window-size 605 360 \
  --icon-size 90 \
  --icon "QDento.app" 160 150 \
  --app-drop-link 435 150 \
  --hide-extension "QDento.app" \
  $INSTALLER \
  "$PWD/../files/"

#signing the dmg
codesign --options runtime --timestamp -s "Developer ID Application: Hristo Konstantinov" $INSTALLER

#submitting for notarization
xcrun notarytool submit $INSTALLER --keychain-profile "DeployProfile" --wait

#applying notarization
xcrun stapler staple $INSTALLER

#verifying notarization
spctl --assess -vv --type install $INSTALLER

#deleting the original deployed file
rm -R $APP
