struct S { uint x; }
enum E {A, B, C}
type T is uint72;
interface I {
    function f(uint) external pure returns (T);
}

function uintSuffix(uint) pure suffix returns (uint) { return 1234; }
function bytes4Suffix(uint) pure suffix returns (bytes4) { return 0x12345678; }
function addressSuffix(uint) pure suffix returns (address) { return 0x1111111111222222222233333333334444444444; }
function stringSuffix(uint) pure suffix returns (string memory) { return "abcd"; }
function bytesSuffix(uint) pure suffix returns (bytes memory) { return "abcd"; }
function arraySuffix(uint) pure suffix returns (bytes3[3] memory) { return [bytes3(0x123456), 0x123456, 0x123456]; }
function structSuffix(uint) pure suffix returns (S memory) { return S(42); }
function enumSuffix(uint) pure suffix returns (E) { return E.C; }
function udvtSuffix(uint) pure suffix returns (T) { return T.wrap(42); }
function interfaceSuffix(uint) pure suffix returns (I) { return I(address(42)); }
function functionSuffix(uint) pure suffix returns (function (uint) external pure returns (T)) { return I(address(0x1111111111222222222233333333334444444444)).f; }

contract C {
    uint public a = 1 uintSuffix;
    bytes4 public b = 1 bytes4Suffix;
    address public c = 1 addressSuffix;
    string public d = 1 stringSuffix;
    bytes public e = 1 bytesSuffix;
    bytes3[3] public f = 1 arraySuffix;
    S public g = 1 structSuffix;
    E public h = 1 enumSuffix;
    T public i = 1 udvtSuffix;
    I public j = 1 interfaceSuffix;
    function (uint) external pure returns (T) public k = 1 functionSuffix;
}
// ----
// a() -> 1234
// b() -> 0x1234567800000000000000000000000000000000000000000000000000000000
// c() -> 0x1111111111222222222233333333334444444444
// d() -> 0x20, 4, "abcd"
// e() -> 0x20, 4, "abcd"
// f(uint256): 0 -> 0x1234560000000000000000000000000000000000000000000000000000000000
// f(uint256): 1 -> 0x1234560000000000000000000000000000000000000000000000000000000000
// f(uint256): 2 -> 0x1234560000000000000000000000000000000000000000000000000000000000
// g() -> 42
// h() -> 2
// i() -> 42
// j() -> 42
// k() -> 0x1111111111222222222233333333334444444444b3de648b0000000000000000
