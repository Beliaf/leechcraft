SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

SET (SRCS
    aggregator.cpp
    core.cpp
    addfeed.cpp
    parserfactory.cpp
	rssparser.cpp
    rss20parser.cpp
	rss10parser.cpp
	rss091parser.cpp
	atomparser.cpp
    atom10parser.cpp
    atom03parser.cpp
    parser.cpp
    item.cpp
    channel.cpp
    feed.cpp
    xmlsettingsmanager.cpp
    channelsmodel.cpp
    channelsfiltermodel.cpp
	itembucket.cpp
	itemmodel.cpp
	regexpmatcherui.cpp
	singleregexp.cpp
	regexpmatchermanager.cpp
	itemsfiltermodel.cpp
	opmlparser.cpp
	opmlwriter.cpp
	importopml.cpp
	export.cpp
	sqlstoragebackend.cpp
	storagebackend.cpp
	importbinary.cpp
	feedsettings.cpp
	jobholderrepresentation.cpp
	itemswidget.cpp
	selectablebrowser.cpp
	itemslistmodel.cpp
	)
SET (HEADERS
    aggregator.h
    core.h
    addfeed.h
    item.h
    channel.h
    feed.h
    parserfactory.h
    parser.h
	rssparser.h
    rss20parser.h
	rss10parser.h
	rss091parser.h
	atomparser.h
    atom10parser.h
    atom03parser.h
    channelsmodel.h
    xmlsettingsmanager.h
    channelsfiltermodel.h
	itembucket.h
	itemmodel.h
	regexpmatcherui.h
	singleregexp.h
	regexpmatchermanager.h
	itemsfiltermodel.h
	opmlparser.h
	opmlwriter.h
	importopml.h
	export.h
	sqlstoragebackend.h
	storagebackend.h
	opmlitem.h
	importbinary.h
	feedsettings.h
	jobholderrepresentation.h
	itemswidget.h
	selectablebrowser.h
	itemslistmodel.h
	)
SET (FORMS
    mainwidget.ui
    addfeed.ui
	itembucket.ui
	regexpmatcherui.ui
	singleregexp.ui
	importopml.ui
	export.ui
	importbinary.ui
	feedsettings.ui
	itemswidget.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_aggregator_ru_RU.ts
	leechcraft_aggregator_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_aggregator_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_aggregator_uk_UA.qm
	)

ADD_DEFINITIONS(-DBOOST_DYN_LINK)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_aggregator SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
LINK_DIRECTORIES (${Boost_LIBRARY_DIR})
TARGET_LINK_LIBRARIES (leechcraft_aggregator
	${Boost_DATE_TIME_LIBRARY}
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_aggregator DESTINATION ${LC_PLUGINS_DEST})
