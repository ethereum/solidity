contract C {

    string uninitializedString;
    string emptyString = "";
    string nonEmptyString = "This is a non empty string";
    string nonEmptyString2 = "Another string";
    bytes uninitializedBytes;
    bytes emptyBytes = "";
    error EmptyError(string);
    event EmptyEvent(string);

    function f() public returns (string memory) {
        return uninitializedString;
    }

    function g() public returns (string memory, string memory) {
        return (uninitializedString, emptyString);
    }

    function h() public returns (string memory, string memory) {
        return (uninitializedString, nonEmptyString);
    }

    function i() public returns (string memory, string memory) {
        return (nonEmptyString, emptyString);
    }

    function j(string calldata _s) public returns (string memory) {
        return _s;
    }

    function k() public returns (string memory) {
        nonEmptyString2 = "";
        return nonEmptyString2;
    }

    function l(string calldata _s) public returns (bytes memory) {
        return abi.encode(_s);
    }

    function m() public returns (string memory) {
        bytes memory b = abi.encode(emptyString);
        return string(b);
    }

    function n() public {
        revert EmptyError(uninitializedString);
    }

    function o() public {
        emit EmptyEvent(emptyString);
    }

    function p() public {
        emit EmptyEvent("");
    }

    function q() public returns (bytes memory) {
        return uninitializedBytes;
    }

    function r() public returns (bytes memory) {
        emptyBytes = abi.encode("");
        return emptyBytes;
    }

    function s() public returns (bytes memory) {
        emptyBytes = abi.encode(uninitializedString);
        return emptyBytes;
    }

    function set(string calldata _s) public {
        nonEmptyString = _s;
    }

    function get() public returns (string memory) {
        return nonEmptyString;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0x20, 0
// g() -> 0x40, 0x60, 0, 0
// h() -> 0x40, 0x60, 0, 0x1a, 38178759162904981154304545770567765692299154484752076569098748838215919075328
// i() -> 0x40, 0x80, 0x1a, 38178759162904981154304545770567765692299154484752076569098748838215919075328, 0
// j(string): 0x20, 0, "" -> 0x20, 0
// k() -> 0x20, 0
// l(string): 0x20, 0, "" -> 0x20, 0x40, 0x20, 0
// m() -> 0x20, 0x40, 0x20, 0
// n() -> FAILURE, hex"d3f13430", hex"0000000000000000000000000000000000000000000000000000000000000020", hex"0000000000000000000000000000000000000000000000000000000000000000"
// o() ->
// ~ emit EmptyEvent(string): 0x20, 0x00
// p() ->
// ~ emit EmptyEvent(string): 0x20, 0x00
// q() -> 0x20, 0
// r() -> 0x20, 0x40, 0x20, 0
// s() -> 0x20, 0x40, 0x20, 0
// set(string): 0x20, 0, "" ->
// get() -> 0x20, 0
