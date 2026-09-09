error NamedArgsError2(uint256 a, uint256 b);
error NamedArgsError3(uint256 a, uint256 b, uint256 c);
error NamedArgsError4(uint256 a, string b, uint256 c, bool d);

contract C {
    function trigger1() external pure {
        revert NamedArgsError2({a: 2, b: 7});
    }
    function trigger2() external pure {
        revert NamedArgsError2({b: 7, a: 2});
    }
    function trigger3() external pure {
        revert NamedArgsError3({c: 9, a: 2, b: 7});
    }
    function trigger4() external pure {
        revert NamedArgsError4({b: "error", a: 2, c: 9, d: true});
    }
    function trigger5() external pure {
        revert NamedArgsError4(2, "error", 9, true);
    }
}

// ----
// trigger1() -> FAILURE, hex"59f176a3", 2, 7
// trigger2() -> FAILURE, hex"59f176a3", 2, 7
// trigger3() -> FAILURE, hex"b912b8d4", 2, 7, 9
// trigger4() -> FAILURE, hex"8ae9981b", 2, 0x80, 9, true, 5, "error"
// trigger5() -> FAILURE, hex"8ae9981b", 2, 0x80, 9, true, 5, "error"
