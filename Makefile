# To configure exlaunch, edit config.mk.
include $(shell pwd)/config.mk

all:
	cmake -DSMO_DEBUG=$(SMO_DEBUG) -DLOGGER_IP=$(LOGGER_IP) -DFTP_USER=$(FTP_USERNAME) \
			-DFTP_IP=$(FTP_IP) -DFTP_PASS=$(FTP_PASSWORD) -DRYU_PATH=$(RYU_PATH) \
			-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain.cmake -S . -B build -G Ninja
	cmake --build build --target subsdk_npdm -j 10
	$(MAKE) deploy

deploy:
	cmake --build build --target install -j 10

clean:
	rm -r build

start_logging_server:
	python misc/scripts/tcpServer.py