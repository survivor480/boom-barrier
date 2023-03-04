idf_component_register(SRCS main.c files/boom_barrier.c files/ethernet_connect.c files/http_server.c files/spiffs_create.c
                       INCLUDE_DIRS "."
                       EMBED_FILES webpage/static/css/main.css webpage/static/js/main.js webpage/static/icon/images.png webpage/index.html webpage/ota.html webpage/static/js/index_html.js)
