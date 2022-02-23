// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
pragma abicoder v2;

// The point of this test is to check that the
// AST IDs are removed from the optimized IR
// so that they do not have a big effect on the
// optimizer if it has a bug that makes it
// depen on the actual identifiers.

struct S { uint x; }
struct T { uint[2] y; }

contract C {
	S[2] values;
	T t;

	function sumArray(S[] memory _s) public returns (uint, string memory) {
		values[0].x = _s[0].x;
		t.y[0] = _s[1].x;
		return (t.y[0], "longstringlongstringlongstringlongstringlongstringlongstringlongstringlongstringlongstringlongstring");
	}
}
