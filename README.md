EPaper 

This tool allows printing documents to a virtual printer for reading on an Inkplate 10 device.

The use case I had in mind was cooking with recipes from the internet.  Printing to paper for 
a recipe that is going to be used once seemed wasteful and reading the recipe from a phone/laptop
is aggravating due to power-management.

This tool consists of a sketch to be installed on the Inkplate and a service to run on a webserver.
The server needs to be configured with a CUPS pdf printer that is shared with the internal network as well as a 
webserver that is at least available on the internal network. Install the pdftoepaper.sh script and ensure that it
is run at startup. There are some config values at the top of the script that will need to be changed.

Copy config.h.in to config.h and edit it with your wifi and server details and flash it to the inkplate.
Install a FAT-formatted SDcard into the inkplate.

When waking from sleep hold down whichever touchpads you intend to use until the loading circle appears.

Hold 1 to go back a page
Hold 3 to go forward a page
Hold 2 to download a new document from the printer

