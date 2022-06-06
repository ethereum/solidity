==== Source: A ====
function gwei(uint x) pure returns (uint) { return x; }

==== Source: B ====
function wei(uint x) pure returns (uint) { return x; }

==== Source: C ====
function ether(uint x) pure returns (uint) { return x; }

==== Source: D ====
function seconds(uint x) pure returns (uint) { return x; }

==== Source: E ====
function minutes(uint x) pure returns (uint) { return x; }

==== Source: F ====
function hours(uint x) pure returns (uint) { return x; }

==== Source: G ====
function days(uint x) pure returns (uint) { return x; }

==== Source: H ====
function weeks(uint x) pure returns (uint) { return x; }

==== Source: I ====
function years(uint x) pure returns (uint) { return x; }

==== Source: test.sol ====
import "A";
import "B";
import "C";
import "D";
import "E";
import "F";
import "G";
import "H";
import "I";

contract C {
    function f() public pure {
        1 wei;
        1 gwei;
        1 ether;
        1 seconds;
        1 minutes;
        1 hours;
        1 days;
        1 weeks;
        1 years;
    }
}
// ----
// ParserError 2314: (A:9-13): Expected identifier but got 'gwei'
// ParserError 2314: (B:9-12): Expected identifier but got 'wei'
// ParserError 2314: (C:9-14): Expected identifier but got 'ether'
// ParserError 2314: (D:9-16): Expected identifier but got 'seconds'
// ParserError 2314: (E:9-16): Expected identifier but got 'minutes'
// ParserError 2314: (F:9-14): Expected identifier but got 'hours'
// ParserError 2314: (G:9-13): Expected identifier but got 'days'
// ParserError 2314: (H:9-14): Expected identifier but got 'weeks'
// ParserError 2314: (I:9-14): Expected identifier but got 'years'
