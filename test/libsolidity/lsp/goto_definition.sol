// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.8.0;

import "./lib.sol";

interface I
{
    function f(uint x) external returns (uint);
}

contract IA is I
{
    function f(uint x) public pure override returns (uint) { return x + 1; }
}

contract IB is I
{
    function f(uint x) public pure override returns (uint) { return x + 2; }
}

library IntLib
{
    function add(int self, int b) public pure returns (int) { return self + b; }
}

contract C
{
    I obj;
    function virtual_inheritance() public payable
    {
        obj = new IA();
        obj.f(1); // goto-definition should jump to definition of interface.
    }

    using IntLib for *;
    function using_for(int i) pure public
    {
        i.add(5);
        14.add(4);
    }

    function useLib(uint n) public payable returns (uint)
    {
        return Lib.add(n, 1);
    }

    function enums(Color c) public pure returns (Color d)
    {
        Color e = Color.Red;
        if (c == e)
            d = Color.Green;
        else
            d = c;
    }

    type Price is uint128;
    function udlTest() public pure returns (uint128)
    {
        Price p = Price.wrap(128);
        return Price.unwrap(p);
    }

    function structCtorTest(uint8 v) public pure returns (uint8 result)
    {
        RGBColor memory c = RGBColor(v, 2 * v, 3 * v);
        result = c.red;
    }
}
