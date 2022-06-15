PROJECT_DIR=/home/$(shell whoami)/libraries/projects
INSTALL_DIR=/home/$(shell whoami)/libraries/install
EVENT_DRIVEN_FLAGS=-DBUILD_HARDWAREIO=ON -DBUILD_PROCESSING=ON -DENABLE_vCorner=ON -DENABLE_vPreProcess=ON -DENABLE_vFramer=ON -DVLIB_CLOCK_PERIOD_NS=80 -DVLIB_CODEC_TYPE=CODEC_304x240_24 -DVLIB_DEPRECATED=ON -DVLIB_TIMER_BITS=30 

all: sources install-YCM install-yarp install-event-driven 
	mkdir -p $(INSTALL_DIR)

sources:
	mkdir -p $(PROJECT_DIR)
	if [ -d $(PROJECT_DIR)/YCM ]; then \
		echo "YCM already exists"; \
	else \
		git clone https://github.com/robotology/YCM.git $(PROJECT_DIR)/YCM; \
	fi;
	if [ -d $(PROJECT_DIR)/yarp ]; then \
		echo "yarp already exists"; \
	else \
		git clone https://github.com/robotology/yarp.git $(PROJECT_DIR)/yarp; \
    fi;
	if [ -d $(PROJECT_DIR)/event-driven ]; then \
       		echo "event-driven already exists"; \
    else \
		git clone https://github.com/robotology/event-driven.git $(PROJECT_DIR)/event-driven; \
    fi;

install-YCM:
	cmake -B$(PROJECT_DIR)/YCM/build/ -H$(PROJECT_DIR)/YCM/ -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR)
	make -C $(PROJECT_DIR)/YCM/build/ install -j$$(nproc)

install-yarp:
	cmake -B$(PROJECT_DIR)/yarp/build/ -H$(PROJECT_DIR)/yarp/ -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) -DYCM_DIR=$(INSTALL_DIR) -DENABLE_yarpmod_usbCamera=ON
	make -C $(PROJECT_DIR)/yarp/build/ install -j$$(nproc)

install-event-driven:
	cmake -B$(PROJECT_DIR)/event-driven/build/ -H$(PROJECT_DIR)/event-driven/ -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) -DYCM_DIR=$(INSTALL_DIR) -DYARP_DIR=$(INSTALL_DIR) -DBUILD_HARDWAREIO=ON -DENABLE_IMUCalibDumper=ON -DENABLE_zynqgrabber=ON $(EVENT_DRIVEN_FLAGS)
	make -C $(PROJECT_DIR)/event-driven/build/ install -j$$(nproc)

install-convolution:
	cmake -B$(PROJECT_DIR)/convolution/build -Hsrc -DYCM_DIR=$(INSTALL_DIR) -DYARP_DIR=$(INSTALL_DIR) -Devent_driven_DIR=$(INSTALL_DIR) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) -DVIS=ON -DLOG=0 -DPROFILE=OFF
	make -C $(PROJECT_DIR)/convolution/build install -j$$(nproc) 

dependencies:
	sudo apt-get install libace-dev
	sudo apt install qml-module-qt-labs-folderlistmodel qml-module-qt-labs-settings
	sudo apt install libv4l-dev
	sudo apt-get install qtbase5-dev qtdeclarative5-dev qtmultimedia5-dev qml-module-qtquick2 qml-module-qtquick-window2 qml-module-qtmultimedia qml-module-qtquick-dialogs qml-module-qtquick-controls

clean-install:
	rm -rf $(INSTALL_DIR)

clean-build:
	rm -rf $(PROJECT_DIR)/YCM/build/ $(PROJECT_DIR)/yarp/build/ $(PROJECT_DIR)/event-driven/build/ 
	rm -rf $(PROJECT_DIR)/convolution/build/
	echo 'built projects clean'

cleanall:
	rm -rf $(PROJECT_DIR)
	rm -rf $(INSTALL_DIR)

.SILENT:
	sources
