SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
)
SET (SRCS
	networkmonitor.cpp
	requestmodel.cpp
	headermodel.cpp
	)
SET (HEADERS
	networkmonitor.h
	requestmodel.h
	headermodel.h
	)
SET (FORMS
	networkmonitor.ui
	)
SET (RESOURCES networkmonitorresources.qrc)
SET (TRANSLATIONS
	leechcraft_networkmonitor_ru_RU.ts
	leechcraft_networkmonitor_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_networkmonitor_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_networkmonitor_uk_UA.qm
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
ADD_LIBRARY (leechcraft_networkmonitor SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_networkmonitor
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_networkmonitor DESTINATION ${LC_PLUGINS_DEST})
