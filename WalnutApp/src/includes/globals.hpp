#pragma once

using json = nlohmann::json;

inline json selected_game;

inline json registered_games;

inline std::unordered_map<std::string, std::shared_ptr<Walnut::Image>> game_images;

inline std::shared_ptr<Walnut::Image> default_thumbnail;