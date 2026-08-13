#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <random>

class Logic 
{
public:
	Logic();
	~Logic();

	std::string chooseComment(std::string title, std::string artist);
	bool containsSubstring(std::string_view fullString, std::string_view, std::string_view stringToFind2 = "");
	int generateRandomNum(int min, int max);

private:
	std::vector<std::vector<std::string>> m_trackList;
	std::vector<std::string> m_currentTrack = { "", "" };
	std::vector<std::string> m_prevTrack = { "", "" };
	nlohmann::json m_commentList = {};

	std::random_device m_rd;
	std::mt19937 m_gen;
};