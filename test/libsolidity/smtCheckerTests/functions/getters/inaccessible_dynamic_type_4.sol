pragma experimental SMTChecker;
contract C {
	string public s;
	function g() public view returns (uint256) {
		this.s();
	}
}
// ====
// EVMVersion: <=spuriousDragon
// ----
// Warning 6321: (98-105): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
