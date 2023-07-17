contract A
{
    uint256 public immutable variable;

    constructor()
    {
        B b;
        try b.foo(variable = 1)
        {
            variable = 2;
        }
        catch Panic(uint)
        {
            variable = 3;
        }
        catch Error(string memory)
        {
            variable = 4;
        }
        catch
        {
            variable = 5;
        }
    }
}

contract B
{
    function foo(uint256) external pure
    {
        revert();
    }
}
// ====
// EVMVersion: >=byzantium
