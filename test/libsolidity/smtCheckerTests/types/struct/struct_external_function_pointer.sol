struct S {
    function() external returns (uint) x;
}
contract C {

    function X() public pure returns (uint) {
        return 1;
    }

    uint y;
    uint z;

    function setZ(uint _z) public {
        z = _z;
    }


    function f() public returns (uint) {
        S memory s;
        s.x = this.X;
        require(y == 0);
        require(z == 0);
        uint ret = s.x();
        assert(y == 0); // should hold
        assert(z == 0); // can report violation due to overapproximation after externall call s.x()
        return ret;
    }
}


// ----
// Warning 6328: (431-445): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
