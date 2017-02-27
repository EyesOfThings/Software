# Google Play Skeleton App - README

## Embed EoT App into the Android Application
1. Copy your "*.elf" binary into the "./app/src/main/assets" directory.
    	* Please make sure that only one elf binary is placed in the assets directory.
    	* Do not forget to strip all unnecessary data (like debugging infos) from the elf binary.
          <MV_TOOLS_DIR>/<MV_TOOLS_VERSION>/<platform>/sparc-myriad-elf-4.8.2/sparc-myriad-elf/bin/strip --strip-all <elf-binary>
2. Place your description text about your EoT App in the description.txt file.
3. If your app need additional files copy them into the "./app/src/main/assets/sdcard" directory. 
		Folders and files that are placed in the "sdcard" folder will be mapped to the /mnt/sdcard/ directory of the EoT device.