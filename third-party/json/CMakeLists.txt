# Add third party json library: https://github.com/gabime/spdlog.git



add_library(json INTERFACE)
add_library(nlohmann::json ALIAS json)
message("****** ${PROJECT_SOURCE_DIR}/json/include *****")

target_include_directories(
        json INTERFACE ${PROJECT_SOURCE_DIR}/json/include
)

target_link_libraries(json INTERFACE)
