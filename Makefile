# PREFIX overrides the 'prefix' option of Meson's function project() on 'ninja install'.
PREFIX = ${HOME}/PAV

BUILD_RELEASE = bin/release
BUILD_DEBUG   = bin/debug
FILE_RELEASE  = ${BUILD_RELEASE}/build.ninja
FILE_DEBUG    = ${BUILD_DEBUG}/build.ninja

.PHONY:	help release debug all clean_release clean_debug clean doc

help:
	@echo '----------------------------------------------------------------'
	@echo 'Usage:'
	@echo '  make release       : create "bin-&-lib" of the release version'
	@echo '  make debug         : create "bin-&-lib" of the debug version '
	@echo '  make all           : make debug and release'
	@echo ' '
	@echo '  make clean_release : remove the "release" intermediate files'
	@echo '  make clean_debug   : remove the "debug"   intermediate files'
	@echo '  make clean         : make clean_debug and clean_release'
	@echo ' '
	@echo '  make doc           : generate the documentation of the project'
	@echo '----------------------------------------------------------------'

release: ${FILE_RELEASE}
	ninja install -C ${BUILD_RELEASE}

${FILE_RELEASE}:
	meson --buildtype=release --prefix=${PREFIX} --libdir=lib ${BUILD_RELEASE}

clean_release:
	\rm -rf ${BUILD_RELEASE}

debug: ${FILE_DEBUG}
	ninja install -C ${BUILD_DEBUG}

${FILE_DEBUG}:
	meson --buildtype=debug --prefix=${PREFIX} --libdir=lib ${BUILD_DEBUG}

clean_debug:
	\rm -rf ${BUILD_DEBUG}

all: release debug

clean: clean_release clean_debug

doc:
	ninja doc -C ${BUILD_RELEASE}
