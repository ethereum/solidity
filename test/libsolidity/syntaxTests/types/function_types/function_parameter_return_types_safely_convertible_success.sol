contract A {

}

contract B is A {

}

contract Test
{
  function a() internal returns(A) { return new A(); }
  function b() internal returns(B) { return new B(); }

  function address_() internal pure returns (address) { return address(0); }
  function address_payable() internal pure returns (address payable) { return payable(0); }

  function tests() internal
  {
    function () internal returns (A) var_to_A = a;
    function () internal returns (A) var_to_A2 = b;
    function () internal returns (B) var_to_B = b;

    function () internal pure returns (address) var_address = address_;
    function () internal pure returns (address) var_address2 = address_payable;
    function () internal pure returns (address payable) var_address_payable = address_payable;

    var_to_A();
    var_to_A2();
    var_to_B();

    var_address();
    var_address2();
    var_address_payable();
  }
}
// ----
