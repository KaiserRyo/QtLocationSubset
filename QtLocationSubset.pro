include(common.pri)

# Build library and tests
TEMPLATE = subdirs
SUBDIRS = src

# Use friendly make target name
src.target = lib

# Check if tests should be built
contains(DEFINES, BB_TEST_BUILD) {

    # Recurse into tests directory
    SUBDIRS += tests

    # Make tests dependent on library and use friendly make target name
    tests.depends = src
    tests.target = tests
} else {
    # Build the QML plugin.  It requires that the library be built first.
    SUBDIRS += plugins
    plugins.depends = src
}