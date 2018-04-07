#include "ftime.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace std;

TimeNode::TimeNode() {
	children = vector<TimeNode>();
}

TimeNodeStack::TimeNodeStack()
{
	start = std::chrono::high_resolution_clock::now();
}


void TimeNodeStack::push(string name)
{
	TimeNode t_node;
	t_node.name = name;
	t_node.begin = std::chrono::high_resolution_clock::now();
	stack.push_back(t_node);
}

void TimeNodeStack::pop()
{
	if (stack.size() > 1) {
		TimeNode t_node = stack[stack.size() - 1];
		stack.pop_back();
		t_node.end = std::chrono::high_resolution_clock::now();
		stack[stack.size() - 1].children.push_back(t_node);
	} else if (stack.size() == 1) {
		stack[0].end = std::chrono::high_resolution_clock::now();
		print();
	} else {
		throw runtime_error("error");
	}
}

void TimeNodeStack::print_recursive(const TimeNode& x, const string& arrow)
{
	cout << setw(60) << left << arrow + x.name << setw(20) << left << std::chrono::duration_cast<std::chrono::microseconds>(x.begin - start).count() << setw(20) << left << std::chrono::duration_cast<std::chrono::microseconds>(x.end - x.begin).count() << '\n';

	for (TimeNode child : x.children) {
		print_recursive(child, arrow + "-->");
	}
}

void TimeNodeStack::print()
{
	if (stack.size() != 1) {
		throw runtime_error("Error: not finished visiting the call stack.");
	}
	cout << setw(60) << left << "namespace/function name" << setw(20) << left << "unix begin time" << setw(20) << left << "time elapsed" <<'\n';
        cout << string(100, '-') << '\n';
	auto node = stack[0];
	print_recursive(node, string(""));
	stack.pop_back();
}

TimeNodeStack t_stack = TimeNodeStack();
