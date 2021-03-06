INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${QT_PHONON_INCLUDES}
	${LEECHCRAFT_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	)
SET (SRCS
	lmp.cpp
	core.cpp
	xmlsettingsmanager.cpp
	keyinterceptor.cpp
	videosettings.cpp
	entitychecker.cpp
	player.cpp
	)
SET (HEADERS
	lmp.h
	core.h
	xmlsettingsmanager.h
	keyinterceptor.h
	videosettings.h
	entitychecker.h
	player.h
	phonon.h
	)
SET (FORMS
    player.ui
	videosettings.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_lmp_ru_RU.ts
	leechcraft_lmp_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_lmp_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_lmp_uk_UA.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_lmp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_lmp
	${QT_LIBRARIES}
	${QT_PHONON_LIBS}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_lmp DESTINATION ${LC_PLUGINS_DEST})
