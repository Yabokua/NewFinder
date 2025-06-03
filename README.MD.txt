# NewFinder

**NewFinder** is a cross-platform Qt-based application designed to retrieve and display current news using the MediaStack API. You can replace it with your own by registering at https://mediastack.com.

## Features

- Get news by country, keywords
- Choose language news
- Add keywords to favorites
- Flexible choice of interface color theme
- Follow links to original sources
- Save settings and favorite keywords between sessions

## Technologies and dependencies

- Qt 6.x (or Qt 5.15 with Qt Widgets and Qt Network support)
- CMake 3.15 or higher
- OpenSSL (required for HTTPS requests)
- Compiler with C++17 support

- Qt JSON (processing API responses)
- Qt Settings (local storage)
- MediaStack API

## Resource file (resources.qrc)

Contains design styles (QSS), images and icons
/icons - contains images (icons)
/styles - contains css styles