function s(uint x) pure returns (uint) { return x; }
function S(uint x) pure returns (uint) { return x; }
function suffix(uint x) pure returns (uint) { return x; }
function SUFFIX(uint x) pure returns (uint) { return x; }
function suffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffix(uint x) pure returns (uint) { return x; }
function __(uint x) pure returns (uint) { return x; }
function _____________________________________(uint x) pure returns (uint) { return x; }
function _s_(uint x) pure returns (uint) { return x; }
function $(uint x) pure returns (uint) { return x; }
function _$(uint x) pure returns (uint) { return x; }
function abcdef(uint x) pure returns (uint) { return x; }

contract C {
    function f() public pure {
        1000 s;
        1000 S;
        1000 suffix;
        1000 SUFFIX;
        1000 suffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffix;
        1000 __;
        1000 _____________________________________;
        1000 _s_;
        1000 $;
        1000 _$;
        0x1000_abcdef abcdef;
    }
}
