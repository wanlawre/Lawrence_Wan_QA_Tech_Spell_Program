#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "json/json.h"

int main()
{
	Json::Value root;   // holds root value after parsing.
	Json::Reader reader;
	std::ifstream doc("spells.json", std::ifstream::binary);
	bool parsingSuccessful = reader.parse( doc, root, false );
	if ( !parsingSuccessful )
	{
    // report to the user the failure and their locations in the document.
    std::cout  << "Failed to parse!\n"
               << reader.getFormattedErrorMessages();
    return 1;
	}

	//create struct that will hold all spell information
	struct spell_info {
		std::string name;
		int damage;
		double cooldown;
		double ratio;
		std::string type;
		spell_info(std::string _name, int _damage, double _cooldown, double _ratio, std::string _type){
			name = _name;
			damage = _damage;
			cooldown = _cooldown;
			ratio = _ratio;
			type = _type;
		}	
	};

	//read all champions into a vector
	Json::Value data = root.get("data", "invalid");
	std::vector<std::string> names = data.getMemberNames();
	//holder vector for temporary spells
	std::vector<spell_info> current_champ_spells;

	//create map to store champion and relevant spell info
	std::map<std::string, std::vector<spell_info> > all_champ_damage_spells;

	for (size_t i = 0; i < names.size(); i++){
		current_champ_spells.clear();
		//get a particular champion's information
		Json::Value champion = data.get(names[i], "invalid");
		//load up that champion's spells
		Json::Value champion_spells = champion.get("spells", "invalid");
		//counter for the 4 spells
		int counter = 0;
		//iterate through a champ's 4 spells
		for (Json::ValueIterator it = champion_spells.begin(); it != champion_spells.end(); ++it){
			//make sure we get the right values for damage, see get_right_effectBurn
			int get_right_effectBurn = 0;
			//verify that it is a damage spell by looking at the spell's labels
			for (int j = 0; j < champion_spells[counter]["leveltip"]["label"].size(); j++){
				bool already_exists = false;
				//if the word contains "Damage", then we know it's a damage spell
				if (champion_spells[counter]["leveltip"]["label"][j].asString().find("Damage") != std::string::npos){

					std::string name_temp = champion_spells[counter]["name"].asString();
					//j+1 to account for "null" in effects
					int damage_temp = champion_spells[counter]["effect"][j+1][champion_spells[counter]["effect"][j+1].size()-1].asInt();
					std::string type_temp = champion_spells[counter]["vars"][0].get("link", "invalid").asString();
					double ratio_temp = champion_spells[counter]["vars"][0]["coeff"][0].asDouble();
					double cooldown_temp = champion_spells[counter]["cooldown"][champion_spells[counter]["cooldown"].size()-1].asDouble();

					for (size_t i = 0; i < current_champ_spells.size(); i++){
						if (name_temp == current_champ_spells[i].name){
							//checks if spell already exists in the array
							already_exists = true;
							//check for strongest part of the spell (replace passive spell values that may have been added)
							if (damage_temp > current_champ_spells[i].damage){
								current_champ_spells[i].name = name_temp;
								current_champ_spells[i].damage = damage_temp;
								current_champ_spells[i].type = type_temp;
								current_champ_spells[i].ratio = ratio_temp;
								current_champ_spells[i].cooldown = cooldown_temp;
							}
						}
					}
					if (already_exists == false){
						spell_info* current_spell_info = new spell_info(name_temp, damage_temp, cooldown_temp, ratio_temp, type_temp);
						current_champ_spells.push_back(*current_spell_info);
						delete current_spell_info;
					}
				}
			}
			counter++;
		}
		//insert champ name and array of spell information
		all_champ_damage_spells.insert(std::pair<std::string, std::vector<spell_info> >(names[i], current_champ_spells));
	}

	//used this to print out all spells and spell information, piped into output.txt
	/*
	for (std::map<std::string,std::vector<spell_info> >::iterator it= all_champ_damage_spells.begin(); it != all_champ_damage_spells.end(); ++it){
		std::cout << it->first << std::endl;
		for (size_t i = 0; i < it->second.size(); i++){
			std::cout << "SPELL " << i+1 << ": " << it->second[i].name << std::endl;
			std::cout << "MAX RANK DAMAGE: " << it->second[i].damage << std::endl;
			std::cout << "RATIO TYPE: " << it->second[i].type << std::endl;
			std::cout << "MAX RANK RATIO: " << it->second[i].ratio << std::endl;
			std::cout << "MAX RANK COOLDOWN: " << it->second[i].cooldown << std::endl;
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
	*/


	int base_ad = 100;
	int base_ap = 0;
	double base_cd = 1;
	int time_period;
	int user_ad;
	int user_ap;
	int user_cd = 0;
	int num_times_cast = 0;
	char c = '.';
	bool done = false;
	std::string check;
	while (done == false){
		//the cin.clear() is for the exit state, need to clear input to read just a period
		std::cout << std::endl;
		std::cout << "TheGreatLawrlus's Spell Efficiency Calculator!" << std::endl;
		std::cout << "______________________________________________" << std::endl;
		std::cout << "       Type \".\" to exit at any time" << std::endl;
		std::cout << std::endl;

		//user input for AP
		std::cin.clear();
		std::cout << "Please enter the amount of bonus Ability Power (AP) this champion has: " << std::endl;
		std::cout << "_______________________________________________________________________" << std::endl;
		if(std::cin.get() == c){
			done == true;
      		break;
      	}
		std::cin >> user_ap;
		std::cout << std::endl;
		if (user_ap < 0){
			std::cerr << std::endl << std::endl << "*Invalid value for AP! Please Try Again*" << std::endl << std::endl << std::endl << std::endl;
			continue;
		}

		//user input for AD
		std::cin.clear();
		std::cout << "Please enter the amount of bonus Attack Damage (AD) this champion has: " << std::endl;
		std::cout << "_______________________________________________________________________" << std::endl;
		if(std::cin.get() == c){
      		done == true;
      		break;
      	}
		std::cin >> user_ad;
		std::cout << std::endl;
		if (user_ad < 0){
			std::cerr << std::endl << std::endl << "*Invalid value for AD! Please Try Again*" << std::endl << std::endl << std::endl << std::endl;
			continue;
		}

		//user input for CDR
		std::cin.clear();
		std::cout << "Please enter the amount of cooldown reduction (in percent, between 0 and 40) this champion has: " << std::endl;
		std::cout << "________________________________________________________________________________________________" << std::endl;
		if(std::cin.get() == c){
      		done == true;
      		break;
      	}
		std::cin >> user_cd;
		if (user_cd > 40){
			std::cerr << std::endl << std::endl << "*Over 40% CDR! Please try again.*" << std::endl << std::endl << std::endl << std::endl;
			continue;
		}
		if (user_cd < 0){
			std::cerr << std::endl << std::endl << "*Invalid Value for CDR! Please try again.*" << std::endl << std::endl << std::endl << std::endl;
			continue;
		}

		//user input for teamfight length, input of 1 assumes just 1 spell cast
		std::cin.clear();
		std::cout << std::endl;
		std::cout << "Please enter how long the team fight was! (in seconds, >= 1): " << std::endl;
		std::cout << "______________________________________________________________" << std::endl;
		if(std::cin.get() == c){
      		done == true;
      		break;
      	}
		std::cin >> time_period;
		std::cout << std::endl << std::endl;
		if (time_period < 1){
			std::cerr << std::endl << std::endl << "*Invalid Value for teamfight! Please try again.*" << std::endl << std::endl << std::endl << std::endl;
			continue;
		}

		//iterate through the vector, find the highest DPS spell with user values inputted
		std::map<std::string,std::vector<spell_info> >::iterator max;
		double max_damage = 0;
		double current_damage;
		int spell_number;
		for (std::map<std::string,std::vector<spell_info> >::iterator it= all_champ_damage_spells.begin(); it != all_champ_damage_spells.end(); ++it){
			for (size_t i = 0; i < it->second.size(); i++){
				//doesn't count spells that had "damage" somewhere in their label but are not actualy damaging spells
				if (it->second[i].type == "invalid" || it->second[i].ratio == 0){
					continue;
				}
				else{
					//calculations for damage takes the base damage and ratio times user inputs for 1 spell, then adds a multiple for however many times that spell would have been case in the user's time frame
					if (it->second[i].type == "spelldamage" && it->second[i].cooldown != 0){
						current_damage = (it->second[i].damage + (user_ap*it->second[i].ratio)) + ((it->second[i].damage + (user_ap*it->second[i].ratio))*(int)(((time_period - (it->second[i].cooldown*(1-(user_cd/100))))/(it->second[i].cooldown*(1-(user_cd/100))))));
					}
					if (it->second[i].type == "bonusattackdamage" && it->second[i].cooldown != 0){
						current_damage = (it->second[i].damage + (user_ad*it->second[i].ratio)) + ((it->second[i].damage + (user_ad*it->second[i].ratio))*(int)(((time_period - (it->second[i].cooldown*(1-(user_cd/100))))/(it->second[i].cooldown*(1-(user_cd/100))))));
					}
					if (it->second[i].type == "attackdamage" && it->second[i].cooldown != 0){
						current_damage = (it->second[i].damage + ((base_ad+user_ad)*it->second[i].ratio)) + ((it->second[i].damage + ((base_ad+user_ad)*it->second[i].ratio))*(int)(((time_period - (it->second[i].cooldown*(1-(user_cd/100))))/(it->second[i].cooldown*(1-(user_cd/100))))));
					}
					//update highest DPS spell
					if (current_damage > max_damage){
						max_damage = current_damage;
						max = it;
						spell_number = i;
						num_times_cast = 1 + (int)(((time_period - (it->second[i].cooldown*(1-(user_cd/100))))/(it->second[i].cooldown*(1-(user_cd/100)))));
					}
				}
			}
			
		}

		std::cout << "With a Base AD of 100," << std::endl << "Bonus AP of " << user_ap << "," << std::endl << "Bonus AD of " << user_ap << "," << std::endl << "Cooldown reduction of " << user_cd << "%," << std::endl << "The teamfight lasting " << time_period << " second(s)," << std::endl << "The most efficient spell in League of Legends is: " << std::endl << std::endl;
		std::cout << "           " << max->first << ", " << max->second[spell_number].name << ", " << max_damage  << " Damage, " << "cast "  << num_times_cast << " times" << std::endl;
	}

}
