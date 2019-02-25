contract C {
    C[] y = new C[](3);
    C[3] x;
    function f() public {
        C[3] memory z;
        y.push(this);
        x[0] = this;
        z[0] = this;
    }
}

