#pragma once
#include "stdAFx.h"
#include "CESave.h"
std::string location = "save.txt";
std::string setup = "game.cfg";
 
//opens file and builds hash map. Creates file if not present then closes it.
void CESave::openFile()
{
	std::string line;
	std::ifstream myfile;
	std::ifstream graphics;
	myfile.open(location);
	graphics.open(setup);
	if (!myfile) { //file doesnt exist
		std::cout << "creating save file";
		createFile(location);
		myfile.open(location);
	}
	if (!graphics) { //file doesnt exist
		std::cout << "creating save file";
		createSettingsFile(setup);
	}

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			size_t location = line.find(":");
			(*saveData)[line.substr(0, location)] = line.substr(location + 1, line.length());
		}
	}
	else {
		std::cout << "File cannnot be opened.";
		//exit game with windows pop up
	}
	myfile.close();
	// Initialize Steam
//	CSteamAchievements::g_SteamAchievements->initializeSteam();
}

//save file 
void CESave::saveFile()
{
	std::ofstream myfile(location);
	for (std::map<std::string, std::string>::iterator it = (*saveData).begin(); it != (*saveData).end(); it++) {
		myfile << it->first + ":" << it->second << "\n";
	}
	myfile.close();

}
 

void CESave::reconstructCFG() {
	std::ofstream myfile(setup);
	std::string renderer = (checkElement("render") == "0") ? "DX11" : "DX12";
	myfile << "sys_spec = 0\n";
	if (checkElement("preset") != "-1") {
		myfile << "sys_spec_shadows=" << checkElement("preset") << "\n";
		myfile << "sys_spec_objectDetails=" << checkElement("preset") << "\n";
		myfile << "sys_spec_textures=" << checkElement("preset") << "\n";
		myfile << "sys_spec_textureResolution=" << checkElement("preset") << "\n";
		myfile << "sys_spec_AntiAliasingMode=" << checkElement("preset") << "\n";
	}
	else {
		myfile << "sys_spec_shadows=" << checkElement("shadows") << "\n";
		myfile << "sys_spec_objectDetails=" << checkElement("objectdetails") << "\n";
		myfile << "sys_spec_textures=" << checkElement("textures") << "\n";
		myfile << "sys_spec_textureResolution=" << checkElement("textures") << "\n";
		myfile << "sys_spec_AntiAliasingMode=" << checkElement("antialiasing") << "\n";
	}
	myfile << "r_VolumetricClouds=1\n";
	myfile << "r_VolumetricCloudsStereoReprojection=0\n";
	myfile << "sys_vr_support=1\n";
	myfile.close();
	createAutoExec(true);
}

/*
modify specific key in save file.
*/
void CESave::modifyElement(std::string key, std::string value)
{
	if ((*saveData).find(key) != (*saveData).end()) { //key found
		(*saveData)[key] = value;
	}
	else {
		std::cout << "key not found";
	}
	//saveFile();
}


//return value of key in save file
std::string CESave::checkElement(std::string key)
{
	return (*saveData)[key];
}

//create file if not present.
void CESave::createFile(std::string filename)
{
	std::ofstream myfile(filename);
	myfile << "10hours" << ":0" << "\n";
	myfile << "20hours" << ":0" << "\n";
	myfile << "3hours" << ":0" << "\n";
	myfile << "5hours" << ":0" << "\n";
	myfile << "billboard1" << ":0" << "\n";
	myfile << "billboard2" << ":0" << "\n";
	myfile << "billboard3" << ":0" << "\n";
	myfile << "croquet" << ":0" << "\n";
	myfile << "pin1" << ":0" << "\n";
	myfile << "pin2" << ":0" << "\n";
	myfile << "pin3" << ":0" << "\n";
	myfile << "pin4" << ":0" << "\n";
	myfile << "poem1" << ":0" << "\n";
	myfile << "poem2" << ":0" << "\n";
	myfile << "poem3" << ":0" << "\n";
	myfile << "poem4" << ":0" << "\n";
	myfile << "returning" << ":0" << "\n";
	myfile << "statue1" << ":0" << "\n";
	myfile << "statue2" << ":0" << "\n";
	myfile << "statue3" << ":0" << "\n";
	myfile << "statue4" << ":0" << "\n";
	myfile << "statue5" << ":0" << "\n";
	myfile << "statue6" << ":0" << "\n";
	myfile << "statue7" << ":0" << "\n";
	myfile << "statue8" << ":0" << "\n";
	myfile << "statue9" << ":0" << "\n";
	myfile << "preset" << ":-1" << "\n";
	myfile << "shadows" << ":1" << "\n";
	myfile << "render" << ":0" << "\n";
	myfile << "objectdetails" << ":1" << "\n";
	myfile << "textures" << ":1" << "\n";
	myfile << "antialiasing" << ":1" << "\n";
	myfile.close();
}
void CESave::createSettingsFile(std::string filename) {
	std::ofstream myfile(filename);
	myfile << "s_ImplName = CryAudioImplFmod\n";
	myfile << "ca_SkeletonEffectsPlayInEngine = 1\n";
	myfile << "r_Driver =" << "\"DX11\"\n";
	myfile << "sys_spec = 5\n";
	myfile << "sys_spec_shadows= 2\n";
	myfile << "r_VolumetricClouds=1\n";
	myfile << "r_VolumetricCloudsStereoReprojection=0\n";
	myfile << "sys_vr_support=1\n";
	myfile.close();
	createAutoExec(false);
}
//creates autoexec file for dx11/dx12. If it does not exist, assumes that there is no file and creates a default one.
void CESave::createAutoExec(bool exists) {
	std::string filename = "autoexec.cfg";
	std::ofstream myfile(filename);
	std::string renderer = "DX11";
	if (exists) {
		renderer = (checkElement("render") == "0") ? "DX11" : "DX12";
	}
	myfile << "s_ImplName = CryAudioImplFmod\n";
	myfile << "ca_SkeletonEffectsPlayInEngine = 1\n";
	myfile << "r_Driver =" << "\"" << renderer << "\"\n";
	myfile.close();
}

//-----------------------------------------------------------------------------------------------------------------------------------------
