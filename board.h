#pragma once
#include <array>
#include <iostream>
#include <cstdio>

/**
 * array-based board for 2048
 *
 * index (2-d form):
 * [0][0] [0][1] [0][2] [0][3]
 * [1][0] [1][1] [1][2] [1][3]
 * [2][0] [2][1] [2][2] [2][3]
 * [3][0] [3][1] [3][2] [3][3]
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */
class board {

public:
	board() : tile() {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	std::array<int, 4>& operator [](const int& i) { return tile[i]; }
	const std::array<int, 4>& operator [](const int& i) const { return tile[i]; }
	int& operator ()(const int& i) { return tile[i / 4][i % 4]; }
	const int& operator ()(const int& i) const { return tile[i / 4][i % 4]; }

public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:
	/**
	 * apply an action to the board
	 * return the reward gained by the action, or -1 if the action is illegal
	 */
	int fibdriver(int n) const{
		if (n>=2)
			n++;
		return fib(n);
	}
	int fib(int n) const{
		//n++;
		if(n==0)
			return 0;
		if(n==1)
			return 1;
		return (fib(n-1)+fib(n-2));
	}
	/*int getfib(int n) const{
		if(n==0)
			return 0;
		if(n==1)
			return 1;
		return (getfib(n-1)+getfib(n-2));
	}*/
	int move(const int& opcode) {
		switch (opcode) {
		case 0: return move_up();
		case 1: return move_right();
		case 2: return move_down();
		case 3: return move_left();
		default: return -1;
		}
	}

	int move_left() {
		board prev = *this;
		int score = 0;
		for (int r = 0; r < 4; r++) {
			auto& row = tile[r];
			int top = 0, hold = 0;
			for (int c = 0; c < 4; c++) {
				int tile = row[c];
				if (tile == 0) continue;
				row[c] = 0;
				if (hold) {
					if ( (tile == hold+1) || (tile==1&&hold==1)) {
						row[top++] = ++tile;
						
						score += fibdriver(tile);//(1 << tile);//Modifiy here
						//std::cout<<"tile: "<<tile<<" fib: "<<fib(tile)<<std::endl;
						hold = 0;
					} 
					else if (tile == hold-1) {
						row[top++] = ++hold;
						score += fibdriver(hold);//(1 << tile);//Modifiy here
						hold = 0;
					}	
					else {
						row[top++] = hold;
						hold = tile;
					}
				} else {
					hold = tile;
				}
			}
			if (hold) tile[r][top] = hold;
		}
		return (*this != prev) ? score : -1;
	}
	int move_right() {
		reflect_horizontal();
		int score = move_left();
		reflect_horizontal();
		return score;
	}
	int move_up() {
		rotate_right();
		int score = move_right();
		rotate_left();
		return score;
	}
	int move_down() {
		rotate_right();
		int score = move_left();
		rotate_left();
		return score;
	}

	void transpose() {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}

	void reflect_horizontal() {
		for (int r = 0; r < 4; r++) {
			std::swap(tile[r][0], tile[r][3]);
			std::swap(tile[r][1], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 4; c++) {
			std::swap(tile[0][c], tile[3][c]);
			std::swap(tile[1][c], tile[2][c]);
		}
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(const int& r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
    friend std::ostream& operator <<(std::ostream& out, const board& b) {
		//char buff[32];
		char buff[64];
		out << "+------------------------+" << std::endl;
/*		for (int r = 0; r < 4; r++) {
			std::snprintf(buff, sizeof(buff), "|%6u%6u%6u%6u|",
				(1 << b[r][0]) & -2u, // use -2u (0xff...fe) to remove the unnecessary 1 for (1 << 0)
				(1 << b[r][1]) & -2u,
				(1 << b[r][2]) & -2u,
				(1 << b[r][3]) & -2u);
			out << buff << std::endl;
		}*/
		for (int r = 0; r < 4; r++) {
			std::snprintf(buff, sizeof(buff), "|%6u%6u%6u%6u|",
				(b.fibdriver(b[r][0])), // use -2u (0xff...fe) to remove the unnecessary 1 for (1 << 0)
				(b.fibdriver(b[r][1])),
				(b.fibdriver(b[r][2])),
				(b.fibdriver(b[r][3])));
			out << buff << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;
	}

private:
    std::array<std::array<int, 4>, 4> tile;
};
