Created by Frank Fodera

This program is a simple 3D house with two floors. It contains a door that can open by the user going up to it and pressing X, a set of stairs to go up. You can press enter for a menu that will show all different keys that have functionality. There is also a small easter egg where you can find a picture of me.


In order to run this program please run the OpenGL3D.exe file in this folder. 

Also if it does not run due to side by side configuration then you can either view the screenshots or compile the code as debug and follow these instructions:

Place the glut32.dll to -> C:\Windows\System  (You can find this file inside of the project folder)

Open the OpenGL3D.sln file in this folder and compile it as debug. 




If you get some linking errors about glut you can also try these instructions:

Install the 3 files required to run GLUT, you can get them from GLUT for Windows: http://www.xmission.com/~nate/glut.html 
Install Glut into the following directories: 
glut32.dll -> C:\Windows\System or C:\WinNT\System
glut32.lib -> C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\Lib
glut32.h -> C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\Include\gl
You must now modify search paths so that VC can find these folders.  Start Visual Studio and from the menu choose Tools | Options.  Scroll down to VC++ Directories and add the path 
C:\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\include
to the include folder which contains gl/glut.h.  


