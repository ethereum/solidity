#include <chrono>
#include <string>
#include <vector>

using namespace std;


class TimeNode
{
public:
	TimeNode();
	string name;
	std::chrono::high_resolution_clock::time_point begin;
	std::chrono::high_resolution_clock::time_point end;
	vector<TimeNode> children;
};

class TimeNodeStack
{
public:
	TimeNodeStack();
	void push(string name);
	void pop();
	void print();
	void print_recursive(const TimeNode& x, const string& arrow);
        bool print_flag = false;
private:
	vector<TimeNode> stack;
        vector<TimeNode> print_stack;
	std::chrono::high_resolution_clock::time_point start;
};

extern TimeNodeStack t_stack;
