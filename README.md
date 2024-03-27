# SeniorDesign

# Install the following vscode extensions
--required: 
* PlatformIO IDE.
* Python.
* c/c++ build tools (all of the microsoft ones).

if you have python you can install the others by running the python script 'install_vscode_extension.py' from the vscode terminal. (still testing :) )

--optional:
* cmake (we don't needed it for our project (platformIO takes care of everything), but if you want to use vscode for c/c++ dev in general you would need it).

# After installing all extensions:
At this step, you have 2 options:
1. load the project by file/open folder (navigate to where you cloned the repo)
2.  load the project by using PlatformIO:
    * In the left most bar on vscode, you can find the icon for PlatformIO (like an alien).
    * If this is the first time using PlatformIO, you will have 2 options. Choose "Pick a folder" and navigate to wherever you cloned the repo)
    * If you have used PlatformIO before (or just closed the 'PIO Home') you will have a section to your left called "Quick Access". These are all the options
    for PlatformIO. You can choose 'Open' and locate the repo.

In both cases, once you load the folder containing the platformio.ini file vscode should prepare your environment. (load libraries, create the make files etc)
This might take a few minutes since it will download the files for the board we are using, plus the libraries for the sensors. 
Everytime we add a new library, that file will update automatically.


# Adding Libraries to projects using PlaformIO
If you want to try something new and require a new library you will need to add it. You have quite a few options I'll cover 2.

1. Using GUI-like system
    * navigate to the GUI interface as described during step 2 in "After installing all extentions".
    * locate the 'libraries' options.
    * add the name of the library you require and open it
    * click the blue button to add to the project
    * under "select prokject" you should have a drop list including our project. Choose the one you're working on.
    * Click add

2. Editing directly the plaformio.ini
If you know the name of the library you require, you only need to add it to the file like follow: <br />
lib_deps = <br />
    &emsp;&emsp;lib1<br />
   &emsp;&emsp;lib2<br />
   &emsp;&emsp;...<br />
   &emsp;&emsp;libn<br />


example:<br />
lib_deps = <br />
	&emsp;&emsp;adafruit/DHT sensor library@^1.4.6<br />
	&emsp;&emsp;adafruit/Adafruit Unified Sensor@^1.1.14<br />

