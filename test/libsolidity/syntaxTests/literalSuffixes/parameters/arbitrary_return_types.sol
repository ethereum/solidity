struct S { uint x; }
enum E {A, B, C}
type T is uint72;
interface I {}

function uintSuffix(uint) pure returns (uint) { return 1234; }
function bytes4Suffix(uint) pure returns (bytes4) { return 0x12345678; }
function addressSuffix(uint) pure returns (address) { return 0x1111111111222222222233333333334444444444; }
function stringSuffix(uint) pure returns (string memory) { return "abcd"; }
function bytesSuffix(uint) pure returns (bytes memory) { return "abcd"; }
function arraySuffix(uint) pure returns (bytes3[3] memory) { return [bytes3(0x123456), 0x123456, 0x123456]; }
function structSuffix(uint) pure returns (S memory) { return S(42); }
function enumSuffix(uint) pure returns (E) { return E.A; }
function udvtSuffix(uint) pure returns (T) { return T.wrap(1); }
function interfaceSuffix(uint) pure returns (I) { return I(address(0)); }
function functionSuffix(uint) pure returns (function (uint) pure returns (T)) { return udvtSuffix; }

contract C {
    uint a = 1 uintSuffix;
    bytes4 b = 1 bytes4Suffix;
    address c = 1 addressSuffix;
    string d = 1 stringSuffix;
    bytes e = 1 bytesSuffix;
    bytes3[3] f = 1 arraySuffix;
    S g = 1 structSuffix;
    E h = 1 enumSuffix;
    T i = 1 udvtSuffix;
    I j = 1 interfaceSuffix;
    function (uint) pure returns (T) k = 1 functionSuffix;
}
