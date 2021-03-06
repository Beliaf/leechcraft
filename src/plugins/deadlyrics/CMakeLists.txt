SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	deadlyrics.cpp
	core.cpp
	searcher.cpp
	lyricwikisearcher.cpp
	findproxy.cpp
	lyricscache.cpp
	)
SET (HEADERS
	deadlyrics.h
	core.h
	searcher.h
	lyricwikisearcher.h
	findproxy.h
	lyricscache.h
	)
SET (TRANSLATIONS
	leechcraft_deadlyrics_ru_RU.ts
	leechcraft_deadlyrics_uk_UA.ts
	)
SET (RESOURCES resources.qrc)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_deadlyrics_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_deadlyrics_uk_UA.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_deadlyrics SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_deadlyrics
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_deadlyrics DESTINATION ${LC_PLUGINS_DEST})
