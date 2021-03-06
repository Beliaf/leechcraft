SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTWEBKIT TRUE)
SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	poshuku.cpp
	xmlsettingsmanager.cpp
	core.cpp
	browserwidget.cpp
	customwebview.cpp
	progresslineedit.cpp
	favoritesmodel.cpp
	addtofavoritesdialog.cpp
	filtermodel.cpp
	favoritesdelegate.cpp
	favoritestreeview.cpp
	customwebpage.cpp
	restoresessiondialog.cpp
	historymodel.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	urlcompletionmodel.cpp
	finddialog.cpp
	screenshotsavedialog.cpp
	cookieseditdialog.cpp
	cookieseditmodel.cpp
	sourceviewer.cpp
	htmlhighlighter.cpp
	xbelparser.cpp
	xbelgenerator.cpp
	pluginmanager.cpp
	historyfiltermodel.cpp
	proxyobject.cpp
	jsproxy.cpp
	notification.cpp
	passwordremember.cpp
	pageformsdata.cpp
	linkhistory.cpp
	cookiesfilter.cpp
	)
SET (HEADERS
	poshuku.h
	xmlsettingsmanager.h
	core.h
	browserwidget.h
	customwebview.h
	progresslineedit.h
	favoritesmodel.h
	addtofavoritesdialog.h
	filtermodel.h
	favoritesdelegate.h
	favoritestreeview.h
	customwebpage.h
	restoresessiondialog.h
	historymodel.h
	storagebackend.h
	sqlstoragebackend.h
	urlcompletionmodel.h
	finddialog.h
	screenshotsavedialog.h
	cookieseditdialog.h
	cookieseditmodel.h
	sourceviewer.h
	htmlhighlighter.h
	xbelparser.h
	xbelgenerator.h
	pluginmanager.h
	historyfiltermodel.h
	proxyobject.h
	jsproxy.h
	notification.h
	passwordremember.h
	pageformsdata.h
	linkhistory.h
	cookiesfilter.h
	interfaces/pluginbase.h
	)
SET (FORMS
	poshuku.ui
	browserwidget.ui
	addtofavoritesdialog.ui
	restoresessiondialog.ui
	finddialog.ui
	screenshotsavedialog.ui
	cookieseditdialog.ui
	sourceviewer.ui
	passwordremember.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_poshuku_ru_RU.ts
	leechcraft_poshuku_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_uk_UA.qm
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
ADD_LIBRARY (leechcraft_poshuku SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_poshuku DESTINATION ${LC_PLUGINS_DEST})

SET (POSHUKU_INCLUDE_DIR ${CURRENT_SOURCE_DIR})
ADD_SUBDIRECTORY (plugins/fua)
ADD_SUBDIRECTORY (plugins/cleanweb)
