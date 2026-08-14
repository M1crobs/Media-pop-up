#include "logic.h"
#include <spdlog/spdlog.h>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <vector>

Logic::Logic()
{
    try {
        std::ifstream file("commentlist.json");
        m_commentList = nlohmann::json::parse(file);
    }
    catch (const nlohmann::json::parse_error* e) {
        spdlog::error("Error while parsing commentlist.json");
    }
    catch (...) {
        spdlog::error("An unkown exception occured");
    }
}

Logic::~Logic() 
{
}

std::string Logic::chooseComment(std::string title, std::string artist) {
    m_prevTrack = m_currentTrack; // Store the current track in a separate var..
	m_currentTrack = { title, artist }; // ..and then rewrite the current one
	m_trackList.push_back(m_currentTrack);

    // iterate through every artist in json
    for (auto it = m_commentList.begin(); it != m_commentList.end(); it++) {
        auto trackList = it.value();

        // if current artist matches
        if (containsSubstring(artist, it.key())) {
            spdlog::info("Found matching artist name in json: {}", it.key());

            // iterate through artist's tracklist
            for (auto i = trackList.begin(); i != trackList.end(); i++) {

                // if current title matches
                if (containsSubstring(title, i.key())) {
                    spdlog::info("Found matching title in json: {}", i.key());

                    // extract comment and erase quotes (.dump() keeps them)
                    std::string comment = i.value().dump();
                    std::erase(comment, '"');

                    // if there are semicolons, then the title has several possible comments
                    // find the position of it and separate strings before and after
                    std::vector<std::string> possibleComments;
                    auto semicolonPos = std::find(comment.begin(), comment.end(), ';');
                    while (semicolonPos != comment.end()) {

                        // separate the part before semicolon
                        possibleComments.push_back(std::string(comment.begin(), semicolonPos));
                        spdlog::debug("Separated left part: {}", std::string(comment.begin(), semicolonPos));

                        // cut the left part from the initial string
                        comment = std::string(semicolonPos + 1, comment.end());
                        spdlog::debug("Got new comment string: {}", comment);

                        // look for another semicolon in the remaining string
                        semicolonPos = std::find(comment.begin(), comment.end(), ';');

                        if (semicolonPos == comment.end()) {
                            possibleComments.push_back(comment);
                            spdlog::debug("Separated left part: {}", comment);
                        }
                    }

                    if (!possibleComments.empty()) {
                        for (std::string s : possibleComments) {
                            spdlog::debug("Possible comment: {}", s);
                         }
                        int randomNum = generateRandomNum(0, possibleComments.size() - 1);
                        spdlog::debug("Generatod random number: {}", randomNum);
                        return possibleComments[randomNum];
                    }
                    return comment;
                }
            }
        }
    }

    // Base case (no matches found)
    return m_commentList["default"]["default"];
}

bool Logic::containsSubstring(std::string_view fullString, std::string_view stringToFind, std::string_view stringToFind2) {
    if (stringToFind.empty()) return true;

    // Look for the first target
    auto res = std::ranges::search(fullString, stringToFind, [](unsigned char a, unsigned char b) {
        return std::tolower(a) == std::tolower(b);
        });

    // If not found the first target, look for the second one
    if (res.empty() && !stringToFind2.empty()) {
        res = std::ranges::search(fullString, stringToFind2, [](unsigned char a, unsigned char b) {
            return std::tolower(a) == std::tolower(b);
            });
    }

    return !res.empty();
}

int Logic::generateRandomNum(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(m_gen);
}