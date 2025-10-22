#include "../include/WebServ.hpp"

void tests() {
    // Aquí puedes agregar llamadas a funciones de prueba si es necesario
    logInfo("%i", path_matches("/img/", "/img/pic.png")); // true
    logInfo("%i", path_matches("/img/", "/img/")); // true
    logInfo("%i", path_matches("/img/", "/img")); // true
    logInfo("%i", path_matches("/img/", "/imges")); // false
    logInfo("%i", path_matches("/img/", "/im")); // false
    logInfo("%i", path_matches("/img/", "/imggallery")); // false
    logInfo("%i", path_matches("/hola/que/", "/hola/que")); // true
    logInfo("%i", path_matches("/hola/que/", "/hola/que/")); // true
    logInfo("%i", path_matches("/hola/que/", "/hola/que/tal.html")); // true
    logInfo("%i", path_matches("/hola/que/", "/hola/quetal.html")); // false
    logInfo("%i", path_matches("/", "/")); // true
    logInfo("%i", path_matches("/", "/index.html")); // true
    logInfo("%i", path_matches("/", "/images/pic.png")); // true
}

int main(int argc, char **argv) 
{
    //tests(); // Descomenta para ejecutar pruebas
    std::string config_path;
    ReadConfig config_reader;
    std::vector<ServerUnit> serverGroup;
    ServerManager serverManager;

    if (argc > 2) {
        std::cerr << USAGE << std::endl;
        return (ERROR);
    }
    
    try {
        if (argc == 1)
            config_path = DEFAULT_CONFIG_FILE;
        else
            config_path = argv[1];
        
        config_reader.createServerGroup(config_path);
        logDebug("🍉 All servers validated successfully");
        serverGroup = config_reader.getServers();
        logDebug("🍉 Config file %s parsed successfully", config_path.c_str());
        serverManager.setup(serverGroup);
        serverManager.init();

    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return (ERROR);
    }
    
    return (SUCCESS);
}
