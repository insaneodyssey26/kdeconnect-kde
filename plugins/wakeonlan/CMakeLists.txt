kdeconnect_add_plugin(kdeconnect_wakeonlan SOURCES wakeonlanplugin.cpp)
target_link_libraries(kdeconnect_wakeonlan kdeconnectcore Qt::Core Qt::Network)
