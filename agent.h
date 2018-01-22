#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"
#include "limits.h"
#include <unistd.h>

using namespace std;
class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			property[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string name() const { return property.at("name"); }
	virtual std::string role() const { return property.at("role"); }
	virtual void notify(const std::string& msg) { property[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> property;
};

/**
 * evil (environment agent)
 * add a new random tile on board, or do nothing if the board is full
 * 2-tile: 90%
 * 4-tile: 10%
 */
class rndenv : public agent {
public:
	rndenv(const std::string& args = "") : agent("name=rndenv role=environment " + args) {
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
		if (property.find("load") != property.end())
			load_weights(property["load"]);
		else {
			weights.push_back(std::pow(24,4));//weights[0]
			weights.push_back(std::pow(24,4));//weights[1]
			weights.push_back(std::pow(24,4));//weights[2]
			weights.push_back(std::pow(24,4));//weights[3]
			weights.push_back(std::pow(24,4));//weights[4]
			weights.push_back(std::pow(24,4));//weights[5]
			weights.push_back(std::pow(24,4));//weights[6]
			weights.push_back(std::pow(24,4));//weights[7]
		}
	}
	virtual action take_action(const board& after) {
		int space[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		std::shuffle(space, space + 16, engine);

		std::uniform_int_distribution<int> popup(0, 3);
		int tile = popup(engine) ? 1 : 3;

		int min = INT_MAX;
		int min_position = -1;
		
		//cout<<"*************************new"<<endl;
		for (int pos : space) {
			//cout<<"considering "<<pos<<endl;
			if (after(pos) != 0)
			{
				//cout<<"not choosing because is"
				continue;
			}
			board sim_before = after;
			action env_move = action::place(tile,pos);
			env_move.apply(sim_before);  //check invalid?
			//int opcode = ((tile<<4)|(pos));
			//sim_before(opcode & 0x0f) = (opcode >> 4);
			int max = INT_MIN;
			int max_action = 0;
			for (int i=0;i<4;i++)
			{
				double value = evaluate(sim_before,i);
				if (value > max)
				{
					max = value;
					max_action = i;
				}
			}
			if (max < min)
			{
				min = max;
				min_position = pos;
			}
			//usleep(10000);
			//return action::place(tile, min_position);
		}
		if (min_position != -1)
			return action::place(tile, min_position);
		return action();
	}
	double evaluate(const board& before, int act)
	{
		board after = before;
		double reward = compute_afterstate(after,act);
	}
	double compute_afterstate(board& after, int act) {
		
		int reward = after.move(act);
		//if reward !=-1 ... intermediate reward
		return reward;
	}
	double approx(board& after) {
		//...
		//std::cout<<after;
		//fflush(stdout);
		double appr = 0;
		for(int i=0;i<4;i++) {
			long long index = 0;
			for(int j=0;j<4;j++) {
				index += after[i][j]*std::pow(24,3-j);
				
			}
			appr += weights[i][index];
			//after[0][0]*std::pow(24,3)
		}
		for(int i=0;i<4;i++) {
			long long index = 0;
			for(int j=0;j<4;j++) {
				index += after[j][i]*std::pow(24,3-j);
				
			}
			appr += weights[i+4][index];
			//after[0][0]*std::pow(24,3)
		}
		return appr;
	}
	virtual void load_weights(const std::string& path) {
		//std::cout<<"evil reading weights"<<std::endl;
		std::ifstream in;
		in.open(path.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		size_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		weights.resize(size);
		for (weight& w : weights)
			in >> w;
		in.close();
	}
	/*virtual action take_action(const board& after) {
		int space[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		std::shuffle(space, space + 16, engine);
		for (int pos : space) {
			if (after(pos) != 0) continue;
			std::uniform_int_distribution<int> popup(0, 9);
			int tile = popup(engine) ? 1 : 2;
			return action::place(tile, pos);
		}
		return action();
	}*/

private:
	std::vector<weight> weights;
	std::default_random_engine engine;
};

/**
 * TODO: player (non-implement)
 * always return an illegal action
 */
class player : public agent {
public:
	player(const std::string& args = "") : agent("name=player role=player " + args), alpha(0.0025f) {
		episode.reserve(32768);
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
		if (property.find("alpha") != property.end())
			alpha = float(property["alpha"]);

		if (property.find("load") != property.end())
			load_weights(property["load"]);
		// TODO: initialize the n-tuple network
	}
	~player() {
		if (property.find("save") != property.end())
			save_weights(property["save"]);
	}

	virtual void open_episode(const std::string& flag = "") {
		episode.clear();
		episode.reserve(32768);
	}

	virtual void close_episode(const std::string& flag = "") {
		// TODO: train the n-tuple network by TD(0)
	}

	virtual action take_action(const board& before) {
		action best;
		// TODO: select a proper action
		// TODO: push the step into episode
		return best;
	}

public:
	virtual void load_weights(const std::string& path) {
		std::ifstream in;
		in.open(path.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		size_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		weights.resize(size);
		for (weight& w : weights)
			in >> w;
		in.close();
	}

	virtual void save_weights(const std::string& path) {
		std::ofstream out;
		out.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		size_t size = weights.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : weights)
			out << w;
		out.flush();
		out.close();
	}

private:
	std::vector<weight> weights;

	struct state {
		// TODO: select the necessary components of a state
		board before;
		board after;
		action move;
		int reward;
	};

	std::vector<state> episode;
	float alpha;

private:
	std::default_random_engine engine;
};
