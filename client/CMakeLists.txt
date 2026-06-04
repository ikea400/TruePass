cmake_minimum_required(VERSION 3.22)
project (TruePass-client LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(qt.cmake)

find_package(PkgConfig REQUIRED)
pkg_check_modules(THORVG REQUIRED IMPORTED_TARGET thorvg-1)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
        Network
        Svg
        Concurrent
        Test
)
qt_standard_project_setup()

set(PROJECT_SOURCES
    img/Icon.ico
    src/app/AppController.h
    src/app/AppController.cpp
    src/app/main.cpp
    src/core/ClientAuth.cpp
    src/core/ClientAuth.h
    src/core/MasterKeyManager.cpp
    src/core/MasterKeyManager.h
    src/core/TpmProvider.h
    src/core/WinTpmProvider.cpp
    src/core/WinTpmProvider.h
    src/core/LinuxTpmProvider.h
    src/core/VaultManager.cpp
    src/core/VaultManager.h
    src/core/Vault.cpp
    src/core/Vault.h
    src/core/VaultItem.h
    src/core/VaultItem.cpp
    src/core/QDefaultException.h
    src/core/TotpContext.cpp
    src/core/TotpContext.h
    src/core/Base32Decoder.h
    src/core/utils.h
    src/core/QSyncTimer.h
    src/core/EnvelopeCodec.cpp
    src/core/EnvelopeCodec.h
    src/core/BufferReader.h
    src/core/BufferWriter.h
    src/core/wordlist.h
    src/core/PasswordGenerator.cpp
    src/core/PasswordGenerator.h
    src/core/FaviconManager.cpp
    src/core/FaviconManager.h
    src/core/DeviceBinder.cpp
    src/core/DeviceBinder.h
    src/core/CardValidator.cpp
    src/core/CardValidator.h
    src/core/IconProvider.cpp
    src/core/IconProvider.h
    src/ui/MainWelcomePage/MainWelcomePage.cpp
    src/ui/MainWelcomePage/MainWelcomePage.h
    src/ui/MainWelcomePage/MainWelcomePage.ui
    src/ui/MainVaultPage/MainVaultPage.cpp
    src/ui/MainVaultPage/MainVaultPage.h
    src/ui/MainVaultPage/MainVaultPage.ui
    src/ui/LoginWidget/LoginWidget.cpp
    src/ui/LoginWidget/LoginWidget.h
    src/ui/LoginWidget/LoginWidget.ui
    src/ui/RegisterWidget/RegisterWidget.cpp
    src/ui/RegisterWidget/RegisterWidget.h
    src/ui/RegisterWidget/RegisterWidget.ui
    src/ui/AddVaultDialog/AddVaultDialog.cpp
    src/ui/AddVaultDialog/AddVaultDialog.h
    src/ui/AddVaultDialog/AddVaultDialog.ui
    src/ui/AddItemDialog/AddItemDialog.cpp
    src/ui/AddItemDialog/AddItemDialog.h
    src/ui/AddItemDialog/AddItemDialog.ui
    src/ui/EditLoginItem/EditLoginItem.cpp
    src/ui/EditLoginItem/EditLoginItem.h
    src/ui/EditLoginItem/EditLoginItem.ui
    src/ui/ViewLoginItem/ViewLoginItem.cpp
    src/ui/ViewLoginItem/ViewLoginItem.h
    src/ui/ViewLoginItem/ViewLoginItem.ui
    src/ui/ItemInfoWidget/ItemInfoWidget.cpp
    src/ui/ItemInfoWidget/ItemInfoWidget.h
    src/ui/ItemInfoWidget/ItemInfoWidget.ui
    src/ui/GeneratePasswordDialog/GeneratePasswordDialog.cpp
    src/ui/GeneratePasswordDialog/GeneratePasswordDialog.h
    src/ui/GeneratePasswordDialog/GeneratePasswordDialog.ui
    src/ui/EditCardItem/EditCardItem.cpp
    src/ui/EditCardItem/EditCardItem.h
    src/ui/EditCardItem/EditCardItem.ui
    src/ui/EditItemDialog/EditItemDialog.cpp
    src/ui/EditItemDialog/EditItemDialog.h
    src/ui/EditItemDialog/EditItemDialog.ui
    src/ui/ViewCardItem/ViewCardItem.cpp
    src/ui/ViewCardItem/ViewCardItem.h
    src/ui/ViewCardItem/ViewCardItem.ui
    src/ui/Components/PasswordRevealButton.cpp
    src/ui/Components/PasswordRevealButton.h
    src/ui/Components/LoadingButton.cpp
    src/ui/Components/LoadingButton.h
    src/ui/Components/CopyButton.cpp
    src/ui/Components/CopyButton.h
    src/ui/Components/LottieAnimation.cpp
    src/ui/Components/LottieAnimation.h
    src/ui/Components/TotpLoadingWidget.cpp
    src/ui/Components/TotpLoadingWidget.h
    src/ui/Components/TotpLabel.cpp
    src/ui/Components/TotpLabel.h
    src/ui/Components/TotpCopyButton.cpp
    src/ui/Components/TotpCopyButton.h
    src/ui/Components/LoadingWidget.cpp
    src/ui/Components/LoadingWidget.h
    src/ui/Components/HiddenLabel.cpp
    src/ui/Components/HiddenLabel.h
    src/ui/Components/KeyCaptureWidget.cpp
    src/ui/Components/KeyCaptureWidget.h
    src/ui/Components/LabeledSlider.cpp
    src/ui/Components/LabeledSlider.h
    src/ui/Components/CappedPlainTextEdit.cpp
    src/ui/Components/CappedPlainTextEdit.h
    src/ui/Components/CardLineEdit.cpp
    src/ui/Components/CardLineEdit.h
    src/network/ServerSession.cpp
    src/network/ServerSession.h
    src/model/VaultListModel.cpp
    src/model/VaultListModel.h
    src/model/VaultModel.h
    src/model/VaultItemModel.h
    src/model/LoginItemDetailModel.h
    src/model/VaultItemSummaryModel.h
    src/model/VaultItemListModel.cpp
    src/model/VaultItemListModel.h
    src/model/VaultItemListProxy.cpp
    src/model/VaultItemListProxy.h
    src/model/CardItemDetailModel.h
    src/dto/LoginItemDto.h
    src/dto/VaultItemMetadataDto.h
    src/dto/VaultItemDetailsBase.h
)

qt_add_resources(PROJECT_SOURCES resource.qrc)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ncrypt.lib
        crypt32.lib)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Svg
        Qt6::Concurrent
        Qt6::Test
        TruePass-common
        PkgConfig::THORVG
)
