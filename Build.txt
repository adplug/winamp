Configuration
-------------
Before you can compile anything, create a file called config.bat in the
source directory. It's best to copy the template below out of this file
and modify it to fit your setup.

The plugin installation procedure uses the "bindir" directory to find
out where your Winamp2 plugin directory is located. If you prefer to
install the plugin manually, you can just leave the path blank. This will
prevent automatic installation.

Setup
-----
Start MSVC and create a new workspace. Then load the project file
in_adlib.dsp from the source subdirecotry into it and you're finished.

Optionally, if you got the AdPlug core library sources installed, you
can additionally load that project file (adplug.dsp) into your
workspace (use the "Add project to workspace..." function from the
"Project" menu) and create a dependency from in_adlib to adplug (i.e.
in_adlib depends on adplug) and the next time you change something on
the core library, the plugin will automatically get recompiled with it.
Use the "Dependencies..." entry from the "Project" menu to do so.

config.bat Template
-------------------
rem Path to Winamp2 plugin directory
set bindir=C:\Program Files\Winamp\Plugins
