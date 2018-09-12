contract C {
	string a;
	string b;
	function f() public view {
		string storage c = a;
		string memory d = b;
		d = string(c);
	}
}
