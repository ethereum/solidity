function suffix(uint x) pure suffix returns (bool) {
    return x < 3;
}

contract C {
    bool public suffixed1 =  1 suffix ? 2 suffix :  3 suffix  ? 4 suffix : 5 suffix;
    bool public suffixed2 =  1 suffix ? 2 suffix : (3 suffix  ? 4 suffix : 5 suffix);
    bool public suffixed3 = (1 suffix ? 2 suffix :  3 suffix) ? 4 suffix : 5 suffix;

    bool public bare1 =  true ? true :  false  ? false : false;
    bool public bare2 =  true ? true : (false  ? false : false);
    bool public bare3 = (true ? true :  false) ? false : false;
}

// ----
// suffixed1() -> true
// suffixed2() -> true
// suffixed3() -> false
// bare1() -> true
// bare2() -> true
// bare3() -> false
