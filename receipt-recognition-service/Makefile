build-ReceiptRecognitionService:
	rm -rf build
	mkdir build
	cd build;																																			\
	cmake .. 																																			\
	-DCMAKE_TOOLCHAIN_FILE=/usr/toolchains/aarch64-al2-linux-gnu/toolchain.cmake 	\
	-DCMAKE_BUILD_TYPE=$(CONFIG)																									\
	-DDEBUG_FUNCS=$(DEBUG_FUNCS);																									\
	make;																																					\
	cp -r bin $(ARTIFACTS_DIR);																										\
	cp -P bootstrap $(ARTIFACTS_DIR)

