contract C {
	function f1() external pure returns (string memory) { return "abcabc"; }
	function f2() external pure returns (string memory) { return "abcabc`~12345677890- _=+!@#$%^&*()[{]}|;:',<.>?"; }
	function g() external pure returns (bytes32) { return "abcabc"; }
	function h() external pure returns (bytes4) { return 0xcafecafe; }
}
// ====
// compileToEwasm: also
// ----
// f1() -> 0x20, 6, left(0x616263616263)
// f2() -> 32, 47, 44048183223289766195424279195050628400112610419087780792899004030957505095210, 18165586057823232067963737336409268114628061002662705707816940456850361417728
// g() -> left(0x616263616263)
// h() -> left(0xcafecafe)
