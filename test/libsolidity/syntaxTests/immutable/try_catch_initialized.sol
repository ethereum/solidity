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
// ----
// TypeError 4130: (108-116): Cannot write to immutable here: Immutable variables cannot be initialized inside a try/catch statement.
// TypeError 4130: (144-152): Cannot write to immutable here: Immutable variables cannot be initialized inside a try/catch statement.
// TypeError 4130: (216-224): Cannot write to immutable here: Immutable variables cannot be initialized inside a try/catch statement.
// TypeError 4130: (297-305): Cannot write to immutable here: Immutable variables cannot be initialized inside a try/catch statement.
// TypeError 4130: (357-365): Cannot write to immutable here: Immutable variables cannot be initialized inside a try/catch statement.
