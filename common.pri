# Disable moc keywords that conflict with 3rd parties
CONFIG += no_keywords

# Detect debug or release build
CONFIG(debug, debug|release) {
    BUILD_MODE = debug
} else {
    BUILD_MODE = release
}

# The release/debug binary file name suffix
CONFIG(debug, debug|release) {
    BIN_SUFFIX = _g
} else {
    BIN_SUFFIX =
}

# Detect target platform
blackberry-armv7le* {
  	PLATFORM = armv7le
	CPU_DIR = armle-v7
}

blackberry-x86* {
    PLATFORM = x86
	CPU_DIR = x86
}

win32|macx|linux* {
    PLATFORM = host
    DEFINES += BB_TEST_BUILD
    CONFIG += test
}

isEmpty(PLATFORM) {
    error("Unsupported platform")
}

# Organize where build output goes
isEmpty(DESTDIR) {
        DESTDIR=$${PWD}/build/$${PLATFORM}/$${BUILD_MODE}
}
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR     = $${DESTDIR}/.moc
RCC_DIR     = $${DESTDIR}/.rcc
UI_DIR      = $${DESTDIR}/.ui
QTPLUGIN_DESTDIR =   $${DESTDIR}/qt4/plugins
QMLPLUGIN_DESTDIR =  $${DESTDIR}/qt4/imports

# Get optional overrides directory from environment variable
OVERRIDES_DIR = $$(QNX_TARGET_OVERRIDES)
!isEmpty(OVERRIDES_DIR) {
	!isEmpty(CPU_DIR) {
		# Search overrides directory for headers/libraries before devstage
		QMAKE_INCDIR = $${OVERRIDES_DIR}/usr/include $${QMAKE_INCDIR}
		QMAKE_LIBDIR = $${OVERRIDES_DIR}/$${CPU_DIR}/lib $${OVERRIDES_DIR}/$${CPU_DIR}/usr/lib $${QMAKE_LIBDIR}
		QMAKE_LFLAGS = -Wl,-rpath-link,$${OVERRIDES_DIR}/$${CPU_DIR}/lib -Wl,-rpath-link,$${OVERRIDES_DIR}/$${CPU_DIR}/usr/lib $${QMAKE_LFLAGS}
	}

    test {
        QMAKE_CXXFLAGS += -idirafter \'$${OVERRIDES_DIR}/usr/include\'
    }

}

# Search local directories for headers/libraries before overrides and devstage
QMAKE_INCDIR = $${PWD}/include $${PWD}/include/public/QtLocationSubset $${QMAKE_INCDIR}
QMAKE_LIBDIR = $${DESTDIR} $${QMAKE_LIBDIR}

# Tell qmake where to look when resolving dependencies on local header files
DEPENDPATH += \
    $${PWD}/include \
    $${PWD}/include/public/QtLocationSubset

# Treat all warnings as errors and hide all 
# symbols by default (i.e. require explicit export)
QMAKE_CXXFLAGS += -Werror -Wno-error=deprecated-declarations -fvisibility=hidden

# Force all symbols in regular object files to be resolved when 
# linking; this helps catch missing dependencies when building a
# shared library. Also, as a failsafe, link only "as needed" libs.
QMAKE_LFLAGS += -Wl,--no-undefined -Wl,--as-needed

# For test builds, search QNX headers last and
# force 32-bit compilation
test {
    QMAKE_CXXFLAGS += -m32 -idirafter \'$(QNX_TARGET)/usr/include\'
    QMAKE_LFLAGS += -m32
}

# For 'gcov' build target, compile without optimization for
# accurate line statistics.  'profile-arcs' and 'test-coverage'
# tell the compiler to generate additional information needed
# by gcov.
gcov {
    QMAKE_CXXFLAGS += -O0 -fprofile-arcs -ftest-coverage
}

# When deploying libraries, enable debug symbols for release 
# builds so they can be stripped and saved in a separate 
# file
release_with_debuginfo {
    QMAKE_CXXFLAGS_RELEASE += -g
}
