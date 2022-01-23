#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <map>
//#include <CryGame/SteamAchievementCustom.h>

class CESave {
private:
	std::map<std::string, std::string>* saveData = new std::map<std::string, std::string>();
public:
 	void openFile();
	void saveFile();
	void modifyElement(std::string, std::string);
	std::string checkElement(std::string);
	bool checkAchievement(const char*);
	void createFile(std::string filename);
	void createSettingsFile(std::string filename);
	void createAutoExec(bool initial);
	void reconstructCFG();
};