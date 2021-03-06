SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

SET (SRCS
	cstp.cpp
	core.cpp
	task.cpp
	hook.cpp
	addtask.cpp
	xmlsettingsmanager.cpp
	mainviewdelegate.cpp
	representationmodel.cpp
	morphfile.cpp
	)
SET (HEADERS
	cstp.h
	core.h
	task.h
	hook.h
	addtask.h
	xmlsettingsmanager.h
	mainviewdelegate.h
	representationmodel.h
	morphfile.h
	)
SET (FORMS
	addtask.ui
	)
SET (RESOURCES
	resources.qrc
	)
SET (TRANSLATIONS
	leechcraft_cstp_ru_RU.ts
	leechcraft_cstp_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_cstp_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_cstp_uk_UA.qm
	)
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_cstp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS})
LINK_DIRECTORIES (${Boost_LIBRARY_DIR})
TARGET_LINK_LIBRARIES (leechcraft_cstp
                        ${QT_LIBRARIES}
                        plugininterface
                        xmlsettingsdialog)
INSTALL (TARGETS leechcraft_cstp DESTINATION ${LC_PLUGINS_DEST})
