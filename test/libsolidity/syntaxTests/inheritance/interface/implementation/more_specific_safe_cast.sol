interface A {}

interface B is A {}

interface X {

    function a() external view returns(A);
    function func() external pure returns (A, A);
    function func2() external pure returns (address);
}

contract Y is X {

    B public a;

    function func() pure external override returns(B, A){
        return (B(address(0)), B(address(0)));
    }

    function func2() pure external override returns(address payable){
        return payable(address(0));
    }

}

// ----