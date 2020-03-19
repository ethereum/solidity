// Test for a bug where we did not increment the counter properly while deleting a dynamic array.
contract C {
    struct S {
        uint256 x;
        uint256[] y;
    }
    S[] data;

    function f() public returns (bool) {
        S storage s1 = data.push();
        s1.x = 2**200;
        S storage s2 = data.push();
        s2.x = 2**200;
        delete data;
        return true;
    }
}

// ----
// f() -> true # This code interprets x as an array length and thus will go out of gas. neither of the two should throw due to out-of-bounds access #
