all:
	$(MAKE) -C echolinux
	$(MAKE) -C echogui

install:
	$(MAKE) -C echolinux install
	$(MAKE) -C echogui install

clean:
	$(MAKE) -C echolinux clean
	$(MAKE) -C echogui clean

copy_defaults:
	$(MAKE) -C echolinux copy_defaults
