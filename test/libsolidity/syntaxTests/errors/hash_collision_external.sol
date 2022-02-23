library L {
    error gsf();
}
contract test {
    error tgeo();
    function f(bool a) public {
        if (a)
            revert L.gsf();
        else
            revert tgeo();
    }
}
// ----
// TypeError 4883: (57-61): Error signature hash collision for tgeo()
