QtLocationSubset
================

BlackBerry 10 Specific QtLocationSubset API

The QtLocationSubset library provides support for positioning and geocoding/reverse geocoding.
It is based on the Qt Mobility 1.2 QtLocation module. It lacks support for landmarks, mapping
and routing. The following enhancements have been made:

-	To support the BlackBerry 10 geocoding/reverse geocoding I/O, the text() and setText() methods
    from Qt 5 were backported to the QGeoAddress class.
-	Support for features of the BlackBerry 10 Location Manager not supported by the QtLocation
    API was added through the Qt property system applied to the BlackBerry 10 specific subclass
    of QGeoPositionInfoSource.
-	Similarly, the BlackBerry 10 related ability to set the  level of specificity (address, city,
    country) for a geocode request is supported utilizing Qt’s property system with subclassed 
    QGeoSearchManagerEngine.
-	Declared metatypes for all of the QGeo* value type classes (backported from Qt 5).
