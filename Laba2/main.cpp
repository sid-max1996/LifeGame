//Задача «пожар». Дано двумерное поле клеток, каждая из которых либо содержит дерево (1), либо куст (2), либо пуста (0). 
//Пожар начинается в одной клетке (либо выбирается случайным образом, либо задаётся явно). 
//Теперь клетка может гореть (5), или быть потушенной (0). 
//Каждая клетка проверяет состояние своих соседей (их 4) и изменяет своё по правилам:

//Куст или дерево загорается, если любой из соседей горит.
//Куст горит 1 поколение, дерево 2 поколения.
//Куст или дерево загорается, если его окружают 3 горящие клетки по диагонали.
//С помощью алгоритма пульсации, показать картину пожара поля, пока пожар не закончится.

#include <iostream>
#include <ctime>
#include <thread>
#include <mutex>
#include <boost/thread/barrier.hpp>

using namespace std;

#define empty 0
#define tree 1
#define bush 2
#define burn 5

const size_t BOARD_SIZE = 10;
const size_t THREADS_NUMBER = 5;


class Board {

	int main_matrix[BOARD_SIZE][BOARD_SIZE];

public:
	Board() {
		for (int i = 0; i < BOARD_SIZE; i++)
			for (int j = 0; j < BOARD_SIZE; j++)
				main_matrix[i][j] = rand() % 3;
	}

	void Print() {
		for (int i = 0; i < BOARD_SIZE; i++) {
			for (int j = 0; j < BOARD_SIZE; j++)
				cout << main_matrix[i][j] << " ";
			cout << endl;
		}
		cout << endl;
	}

	//загорание ячейки и установка поколения для горения
	static void burn_cell(int value, int row, int col, int(&gener_matrix)[BOARD_SIZE][BOARD_SIZE], 
		int(&working_matrix)[BOARD_SIZE][BOARD_SIZE], size_t & burn_count) {
		gener_matrix[row][col] = value == tree ? 2 : 1;
		working_matrix[row][col] = burn;
		burn_count++;
	}

	static void continued_burn(
		int(&main_matrix)[BOARD_SIZE][BOARD_SIZE],
		int(&working_matrix)[BOARD_SIZE][BOARD_SIZE],
		int start_row, int finish_row, bool is_control, 
		boost::barrier & barrier, size_t & fire_gener, size_t & burn_count, 
		int(&gener_matrix)[BOARD_SIZE][BOARD_SIZE]) {

		for (int r = start_row; r <= finish_row; r++)
			for (int c = 0; c < BOARD_SIZE; c++) {
				int val = main_matrix[r][c];
				working_matrix[r][c] = val;
				if (val == tree || val == bush) {
					int burn_diagonal_count = 0;//счетчик 3 по диогонали горят
					//up neighbor
					if (r != 0) {
						if (main_matrix[r - 1][c] == burn) {
							burn_cell(val, r, c, gener_matrix, working_matrix, burn_count);
							continue;
						}
						//diagonal
						if (c != 0)
							main_matrix[r - 1][c - 1] == burn ? burn_diagonal_count++: burn_diagonal_count;
						if (c != BOARD_SIZE - 1)
							main_matrix[r - 1][c + 1] == burn ? burn_diagonal_count++ : burn_diagonal_count;
					}
					//down neighbor
					if (r != BOARD_SIZE-1) {
						if (main_matrix[r + 1][c] == burn) {
							burn_cell(val, r, c, gener_matrix, working_matrix, burn_count);
							continue;
						}
						//diagonal
						if (c != 0)
							main_matrix[r - 1][c - 1] == burn ? burn_diagonal_count++ : burn_diagonal_count;
						if (c != BOARD_SIZE - 1)
							main_matrix[r - 1][c + 1] == burn ? burn_diagonal_count++ : burn_diagonal_count;
					}
					//left neighbor
					if (c != 0) {
						if (main_matrix[r][c-1] == burn) {
							burn_cell(val, r, c, gener_matrix, working_matrix, burn_count);
							continue;
						}
					}
					//right neighbor
					if (c != BOARD_SIZE - 1) {
						if (main_matrix[r][c+1] == burn) {
							burn_cell(val, r, c, gener_matrix, working_matrix, burn_count);
							continue;
						}
					}
					//3 diagonal burn
					if (burn_diagonal_count >= 3) 
						burn_cell(val, r, c, gener_matrix, working_matrix, burn_count);
				
				}
				else {
					if(val == burn){
						int cur_gener = --gener_matrix[r][c];
						if (cur_gener == 0) {
							//mt.lock();
							burn_count--;
							//mt.unlock();
							working_matrix[r][c] = empty;
						}
					}
				}
			}

		barrier.wait();
		if (is_control)
		{
			swap(main_matrix, working_matrix);
			//печать
			cout << "stage of fire = " << ++fire_gener << endl;
			for (int i = 0; i < BOARD_SIZE; i++) {
				for (int j = 0; j < BOARD_SIZE; j++)
					cout << main_matrix[i][j] << " ";
				cout << endl;
			}
			cout << endl;	
		}
		//barrier.wait();
	}

	void start_burning() {

		mutex mt;
		//начало пожара
		int working_matrix[BOARD_SIZE][BOARD_SIZE];//доска для изменений
		int burn_row = rand() % 10;
		int burn_col = rand() % 10;
		while (main_matrix[burn_row][burn_col] == 0) {
			burn_row = rand() % 10;
			burn_col = rand() % 10;
		}
		int gener_matrix[BOARD_SIZE][BOARD_SIZE];
		gener_matrix[burn_row][burn_col] = main_matrix[burn_row][burn_col] == tree?2:1;
		main_matrix[burn_row][burn_col] = burn;	
		size_t burn_count = 1;
		size_t fire_gener = 0;
		cout << "stage of fire = " << fire_gener << endl;
		Print();
		//процесс пожара
		while (burn_count != 0 && fire_gener < 100) {
			thread threads[THREADS_NUMBER - 1];
			boost::barrier barrier(THREADS_NUMBER);
			int row_size = BOARD_SIZE / THREADS_NUMBER;
			int cur_row_num1 = 0;
			int cur_row_num2 = row_size - 1;
			for (int i = 0; i < THREADS_NUMBER - 1; ++i) {
				threads[i] = thread(continued_burn, ref(main_matrix), ref(working_matrix),
					cur_row_num1, cur_row_num2, false, ref(barrier), ref(fire_gener), ref(burn_count), ref(gener_matrix));
				cur_row_num1 = cur_row_num2 + 1;
				cur_row_num2 += row_size;
			}
			continued_burn(main_matrix, working_matrix, cur_row_num1,
				cur_row_num2 < BOARD_SIZE - 1 ? BOARD_SIZE - 1 : cur_row_num2,
				true, barrier, fire_gener, burn_count, gener_matrix);
			for (int i = 0; i < THREADS_NUMBER - 1; i++)
				threads[i].join();
			cout << "burn_count = " << burn_count << " " << "fire_gener = " << fire_gener << endl << endl;
		}
	}
};

int main() {
	srand(time(0));
	Board board;
	board.Print();
	board.start_burning();
}