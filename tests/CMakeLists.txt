include(ECMAddTests)

ecm_add_test(applicationicontest.cpp applicationicontest.qrc
    TEST_NAME applicationicontest
    LINK_LIBRARIES Qt6::Test KF6::IconThemes
    NAME_PREFIX "opusfm-"
    GUI
)
add_test(NAME opusfm-installer COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/installtest.sh ${PROJECT_SOURCE_DIR}/install.sh)

set(opusfm_desktop_file ${PROJECT_BINARY_DIR}/src/app/org.kde.opusfilemanager.desktop)
set(opusfm_metainfo_file ${PROJECT_BINARY_DIR}/src/app/org.kde.opusfilemanager.metainfo.xml)
ecm_add_test(appidentitytest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
)
add_dependencies(appidentitytest opusfm-metadata)
ecm_add_test(brandingtest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
)
target_compile_definitions(brandingtest PRIVATE OPUSFM_LOGO_FILE="${PROJECT_SOURCE_DIR}/src/app/branding/plasma-opus-logo.svg")
ecm_add_test(startuppresentationtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)
target_compile_definitions(appidentitytest PRIVATE
    OPUSFM_PROJECT_VERSION="${PROJECT_VERSION}"
    OPUSFM_DESKTOP_FILE="${opusfm_desktop_file}"
    OPUSFM_METAINFO_FILE="${opusfm_metainfo_file}"
)
find_program(APPSTREAMCLI_EXECUTABLE appstreamcli)
if(APPSTREAMCLI_EXECUTABLE)
    add_test(NAME opusfm-appstream-validate COMMAND ${APPSTREAMCLI_EXECUTABLE} validate --no-net --pedantic --explain ${opusfm_metainfo_file})
endif()
find_program(DESKTOP_FILE_VALIDATE_EXECUTABLE desktop-file-validate)
if(DESKTOP_FILE_VALIDATE_EXECUTABLE)
    add_test(NAME opusfm-desktop-file-validate COMMAND ${DESKTOP_FILE_VALIDATE_EXECUTABLE} ${opusfm_desktop_file})
endif()

set(translation_test_catalog ${CMAKE_CURRENT_BINARY_DIR}/translation-locale/fa/LC_MESSAGES/opus-file-manager.mo)
add_custom_command(
    OUTPUT ${translation_test_catalog}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/translation-locale/fa/LC_MESSAGES
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${translation_test_catalog} ${CMAKE_CURRENT_SOURCE_DIR}/data/translation/opus-file-manager.po
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/translation/opus-file-manager.po
    VERBATIM
)
add_custom_target(translation-test-catalog DEPENDS ${translation_test_catalog})
set(translation_production_catalog ${CMAKE_CURRENT_BINARY_DIR}/production-locale/fa/LC_MESSAGES/opus-file-manager.mo)
add_custom_command(
    OUTPUT ${translation_production_catalog}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/production-locale/fa/LC_MESSAGES
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${translation_production_catalog} ${PROJECT_SOURCE_DIR}/po/fa/opus-file-manager.po
    DEPENDS ${PROJECT_SOURCE_DIR}/po/fa/opus-file-manager.po
    VERBATIM
)
add_custom_target(translation-production-catalog DEPENDS ${translation_production_catalog})
ecm_add_test(translationtest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
)
add_dependencies(translationtest translation-test-catalog translation-production-catalog opus-file-manager)
target_compile_definitions(translationtest PRIVATE
    OPUSFM_TEST_LOCALE_DIR="${CMAKE_CURRENT_BINARY_DIR}/translation-locale"
    OPUSFM_PRODUCTION_LOCALE_DIR="${CMAKE_CURRENT_BINARY_DIR}/production-locale"
    OPUSFM_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    OPUSFM_EXECUTABLE="$<TARGET_FILE:opus-file-manager>"
)

ecm_add_test(informationtranslationtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)
add_dependencies(informationtranslationtest translation-test-catalog)
target_compile_definitions(informationtranslationtest PRIVATE
    OPUSFM_TEST_LOCALE_DIR="${CMAKE_CURRENT_BINARY_DIR}/translation-locale"
)

ecm_add_tests(
    activityassociationstest.cpp
    activityusagetest.cpp
    adminmodetest.cpp
    appearancedefaultstest.cpp
    appfontstest.cpp
    apppackagestest.cpp
    archivestest.cpp
    arktest.cpp
    atuintest.cpp
    batchrenametest.cpp
    batchrenamepresetstest.cpp
    checksumstest.cpp
    cloudactivitytest.cpp
    cloudsynctest.cpp
    cloudsyncrunnertest.cpp
    commandpalettetest.cpp
    customcommandstest.cpp
    diskimagestest.cpp
    favoritesmodeltest.cpp
    filesplittertest.cpp
    filetypeplacestest.cpp
    foldermetadatatest.cpp
    foldercolorstest.cpp
    foldergatestest.cpp
    foldercolorrulestest.cpp
    foldercomparisontest.cpp
    foldersynctest.cpp
    foldersyncprofilestest.cpp
    flatviewtest.cpp
    hiddenfolderstest.cpp
    ignorelisttest.cpp
    indexedsearchtest.cpp
    indexstatustest.cpp
    kdeconnectsharetest.cpp
    lansharetest.cpp
    mailhandofftest.cpp
    mailmessagetest.cpp
    mediabackendtest.cpp
    onlineaccountstest.cpp
    keybindingstest.cpp
    keyboardhintstest.cpp
    keymapfiletest.cpp
    kwinscripttest.cpp
    mounttabletest.cpp
    libraryscantest.cpp
    libraryarchivetest.cpp
    linkednavigationtest.cpp
    localizeddatestest.cpp
    livesearchtest.cpp
    namefiltertest.cpp
    densitytest.cpp
    draftstoretest.cpp
    documentformatstest.cpp
    duplicatefindertest.cpp
    editorproviderstest.cpp
    edittoolstest.cpp
    executablestest.cpp
    filemanagerrequesttest.cpp
    filepublishtest.cpp
    imageeditingtest.cpp
    imageconversiontest.cpp
    imagedetailstest.cpp
    markdownexporttest.cpp
    metadataeditortest.cpp
    metadataimporttest.cpp
    minimapbandstest.cpp
    navigationhistorytest.cpp
    navigationshortcutstest.cpp
    nextcloudaccounttest.cpp
    nextcloudassetstest.cpp
    nextcloudclienttest.cpp
    nextclouddesktoptest.cpp
    nextcloudorganizetest.cpp
    nextcloudphotostest.cpp
    nextcloudsharestest.cpp
    notevaulttest.cpp
    officeappstest.cpp
    operationjobtest.cpp
    previewcontenttest.cpp
    rclonetest.cpp
    remotemountstest.cpp
    remoteprofilestest.cpp
    rsyncsynctest.cpp
    runnermatchestest.cpp
    servicemenustest.cpp
    sharedsearchtest.cpp
    shortcuthintstest.cpp
    shortcutstoragetest.cpp
    sessiontest.cpp
    savedpathfilterstest.cpp
    savedsearchestest.cpp
    searchcollectiontest.cpp
    searchhistorytest.cpp
    searchlocationstest.cpp
    searchquerytest.cpp
    searchrankingtest.cpp
    tabcolorstest.cpp
    startuptest.cpp
    stylesheetstest.cpp
    symlinkstest.cpp
    symboliciconstest.cpp
    tabmodeltest.cpp
    taglibrarytest.cpp
    terminalproviderstest.cpp
    themeprofilestest.cpp
    treemaptest.cpp
    transferqueuetest.cpp
    traypolicytest.cpp
    vaultsettingstest.cpp
    viewsettingstest.cpp
    wikilinkstest.cpp
    workspaceappearancetest.cpp
    workspacecatalogtest.cpp
    workspacerestoretest.cpp
    workspacestartuptest.cpp
    vimnavigationtest.cpp
    yazimodetest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test KF6::ConfigCore
    NAME_PREFIX "opusfm-"
)
# The test reads the ui.rc files it strips.
target_link_libraries(shortcutstoragetest Qt6::Xml)
if(AccountsQt6_FOUND)
    target_compile_definitions(onlineaccountstest PRIVATE OPUSFM_HAVE_ACCOUNTS)
    target_include_directories(onlineaccountstest SYSTEM PRIVATE ${ACCOUNTSQT_INCLUDE_DIRS})
    target_link_libraries(onlineaccountstest ${ACCOUNTSQT_LIBRARIES} Qt6::Xml)
endif()

ecm_add_test(mcpservertest.cpp
    LINK_LIBRARIES opusfm_mcp Qt6::Test
    NAME_PREFIX "opusfm-"
)

ecm_add_test(mcpsettingstest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test KF6::ConfigCore
    NAME_PREFIX "opusfm-"
)
add_dependencies(mcpsettingstest opus-file-manager-mcp)
target_compile_definitions(mcpsettingstest PRIVATE OPUSFM_MCP_EXECUTABLE="$<TARGET_FILE:opus-file-manager-mcp>")

ecm_add_test(vimnavigationuitest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(shortcuthintsuitest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(yazimodeuitest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(foldergateuitest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(hiddenfoldersuitest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(fileencryptiontest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
)

ecm_add_test(fileencryptiondialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(plasmavaultstest.cpp
    LINK_LIBRARIES opusfm_core Qt6::DBus Qt6::Test
    NAME_PREFIX "opusfm-"
)
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session)
if(DBUS_RUN_SESSION_EXECUTABLE)
    add_test(NAME opusfm-plasmavaults-dbus COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} $<TARGET_FILE:plasmavaultstest> talksToPlasmaVault)
endif()

ecm_add_test(plasmavaultmenutest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(galleryeditingtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(workspacedialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(workspacelocationstest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(batchrenamedialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(customcommanddialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(tagginguitest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(metadataeditdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(foldersyncdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(cloudactivitydialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(cloudsyncdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(transferqueuedialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(editorpaneltest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(adminmodeuitest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(previeweditingtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(listingexporttest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

# The keyboard map writes PDF references, which need a GUI application.
ecm_add_test(keyboardmaptest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(contentcharttest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(duplicatefinderdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(searchwidgettest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(doubleshifttest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(filesplitdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(symlinkdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(terminalpaneltest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(terminallaunchertest.cpp
    LINK_LIBRARIES opusfm_app Qt6::DBus Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session)
if(DBUS_RUN_SESSION_EXECUTABLE)
    add_test(NAME opusfm-terminallauncher-dbus COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} $<TARGET_FILE:terminallaunchertest> opensYakuakeSessionsOnTheBus)
    set_tests_properties(opusfm-terminallauncher-dbus PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()

ecm_add_test(fontpreviewtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(mailpreviewtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(mediaviewtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

if(OPUSFM_HAVE_MPV)
    ecm_add_test(mpvplayertest.cpp
        LINK_LIBRARIES opusfm_core Qt6::Test
        NAME_PREFIX "opusfm-"
    )
endif()

ecm_add_test(remotepreviewtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test Qt6::Network
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(mainwindowtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(activitycontrollertest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(applicationpackagedialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(indexstatusdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(kdeconnectsharedialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(sendbyemaildialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(integrationsettingstest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)
if(AccountsQt6_FOUND)
    target_compile_definitions(integrationsettingstest PRIVATE OPUSFM_HAVE_ACCOUNTS)
    target_include_directories(integrationsettingstest SYSTEM PRIVATE ${ACCOUNTSQT_INCLUDE_DIRS})
    target_link_libraries(integrationsettingstest ${ACCOUNTSQT_LIBRARIES})
endif()

ecm_add_test(lansharedialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(onlineaccountsmenutest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(mountmanagerdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(remotemountdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(nextcloudaccountsdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(nextcloudpreviewtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(nextcloudorganizedialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(nextcloudphotosdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(nextcloudsharesdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(nextclouddesktopmenutest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(remoteprofilesdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(kwinintegrationtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(kwinscriptjstest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test Qt6::Qml
    NAME_PREFIX "opusfm-"
)
target_compile_definitions(kwinscriptjstest PRIVATE
    OPUSFM_KWIN_SCRIPT="${PROJECT_SOURCE_DIR}/src/kwin/opusfilemanager/contents/code/main.js"
    OPUSFM_KWIN_METADATA="${PROJECT_SOURCE_DIR}/src/kwin/opusfilemanager/metadata.json"
)

ecm_add_test(opusrunnertest.cpp
    LINK_LIBRARIES opusfm_runner Qt6::Test
    NAME_PREFIX "opusfm-"
)

ecm_add_test(desktoptraytest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)
# The test sends window-manager close requests through QWindowSystemInterface.
find_package(Qt6 ${QT_MIN_VERSION} QUIET COMPONENTS GuiPrivate)
target_link_libraries(desktoptraytest Qt6::GuiPrivate)

ecm_add_test(filemanagerservicetest.cpp
    LINK_LIBRARIES opusfm_app Qt6::DBus Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session)
if(DBUS_RUN_SESSION_EXECUTABLE)
    add_test(NAME opusfm-filemanagerservice-dbus COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} $<TARGET_FILE:filemanagerservicetest> exportsTheInterfaceOnTheBus claimsTheNameOnAPrivateBus)
    set_tests_properties(opusfm-filemanagerservice-dbus PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;OPUSFM_PRIVATE_SESSION_BUS=1")
endif()

ecm_add_test(filechoosertest.cpp
    LINK_LIBRARIES opusfm_core Qt6::DBus Qt6::Test
    NAME_PREFIX "opusfm-"
)
ecm_add_test(defaultfilemanagertest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
)
ecm_add_test(portalsetuptest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
)
ecm_add_test(chooserdialogtest.cpp
    LINK_LIBRARIES opusfm_portal Qt6::DBus Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)
if(DBUS_RUN_SESSION_EXECUTABLE)
    add_test(NAME opusfm-chooserdialog-dbus COMMAND ${DBUS_RUN_SESSION_EXECUTABLE} $<TARGET_FILE:chooserdialogtest> answersRequestsOnTheBus remembersTheLastFolderOnTheBus)
    set_tests_properties(opusfm-chooserdialog-dbus PROPERTIES ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()

ecm_add_test(settingsbackuptest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
)

ecm_add_test(
    settingsdialogtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(combinedpaneltest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)
ecm_add_test(colorschemestest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(settingstest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(filtertest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(tabstest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(archivebrowsingtest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test KF6::Archive
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(archivewritertest.cpp
    LINK_LIBRARIES opusfm_core Qt6::Test KF6::Archive
    NAME_PREFIX "opusfm-"
)

ecm_add_test(flattest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

ecm_add_test(symbolicappiconstest.cpp
    LINK_LIBRARIES opusfm_app Qt6::Test
    NAME_PREFIX "opusfm-"
    GUI
)

if(OPUSFM_RUST_PROTOTYPE)
    ecm_add_test(rustsearchquerytest.cpp
        LINK_LIBRARIES opusfm_core opus_searchquery Qt6::Test Qt6::Concurrent
        NAME_PREFIX "opusfm-"
    )
    add_test(NAME opusfm-rust-searchquery-cargo
        COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR}/cargo-test
            ${Rust_CARGO_CACHED} test --offline --manifest-path ${PROJECT_SOURCE_DIR}/rust/searchquery/Cargo.toml
    )
endif()
