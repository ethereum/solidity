pragma experimental SMTChecker;
contract A {
        mapping(string=>uint) map;
        function f(string memory s, uint x) public {
                map[s] - x;
        }
}
// ----
// Warning 2018: (88-170): Function state mutability can be restricted to view
// Warning 3944: (149-159): CHC: Underflow (resulting value less than 0) happens here.
