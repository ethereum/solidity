/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Killian <>
 * @author Jiayang <>
 * @author Raphael <raphael.s.norwitz@gmail.com>
 * @date 2017
 * A timing utility for debugging compiler performance
 */
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

#include "FTime.h"

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
	if (stack.size() > 1)
	{
		TimeNode t_node = stack[stack.size() - 1];
		stack.pop_back();
		t_node.end = std::chrono::high_resolution_clock::now();
		stack[stack.size() - 1].children.push_back(t_node);
	}
	else if (stack.size() == 1)
	{
		stack[0].end = std::chrono::high_resolution_clock::now();
                if(print_flag)
		{
                        print_recursive(stack[0], string(""));
                        stack.pop_back();
                }
		else
		{
                        print_stack.push_back(stack[0]);
                        stack.pop_back();
                }
                //print();
                //stack.pop_back();
	}
	else
	{
		throw runtime_error("error");
	}
}

void TimeNodeStack::print_recursive(const TimeNode& x, const string& arrow)
{
	cout << setw(70) << left << arrow + x.name << setw(20) << left << 
                std::chrono::duration_cast<std::chrono::microseconds>(
		x.begin - start).count() << setw(24) << left << 
		std::chrono::duration_cast<std::chrono::microseconds>(x.end
				- x.begin).count() << '\n';

	for (TimeNode child : x.children)
	{
		if (arrow.length() == 0)
		{
			print_recursive(child, " \\_");
		}
		else
		{
			print_recursive(child, arrow.substr(0, arrow.length() - 2) + 
					"    " + "\\_");
		}
	}
}

void TimeNodeStack::print()
{
        //User should be allowed to put more than one function at top level of tree, e.g. processInput and actonInput
        //if (stack.size() != 1) {
	//	throw runtime_error("Error: not finished visiting the call stack.");
	//}
        //if (!printed) {
        cout << setw(70) << left << "namespace/function name" << setw(20) << 
                left << "unix begin time(μs)" << setw(24) << left << "time elapsed(μs)" <<'\n';
        cout << string(110, '-') << '\n';
        //cout << "stack size: " << stack.size() << '\n';
	for(TimeNode node: print_stack){
                //auto node = stack[0];
	        print_recursive(node, string(""));
	        //stack.pop_back();
        }
}

TimeNodeStack t_stack = TimeNodeStack();
