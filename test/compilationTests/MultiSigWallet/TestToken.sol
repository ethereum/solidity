pragma solidity >=0.0;


/// @title Test token contract - Allows testing of token transfers with multisig wallet.
contract TestToken {

    event Transfer(address indexed from, address indexed to, uint256 value);
    event Approval(address indexed owner, address indexed spender, uint256 value);

    mapping (address => uint256) balances;
    mapping (address => mapping (address => uint256)) allowed;
    uint256 public totalSupply;

    string constant public name = "Test Token";
    string constant public symbol = "TT";
    uint8 constant public decimals = 1;

    function issueTokens(address _to, uint256 _value)
        public
    {
        balances[_to] += _value;
        totalSupply += _value;
    }

    function transfer(address _to, uint256 _value)
        public
        returns (bool success)
    {
        if (balances[msg.sender] < _value) {
            revert();
        }
        balances[msg.sender] -= _value;
        balances[_to] += _value;
        emit Transfer(msg.sender, _to, _value);
        return true;
    }

    function transferFrom(address _from, address _to, uint256 _value)
        public
        returns (bool success)
    {
        if (balances[_from] < _value || allowed[_from][msg.sender] < _value) {
            revert();
        }
        balances[_to] += _value;
        balances[_from] -= _value;
        allowed[_from][msg.sender] -= _value;
        emit Transfer(_from, _to, _value);
        return true;
    }

    function approve(address _spender, uint256 _value)
        public
        returns (bool success)
    {
        allowed[msg.sender][_spender] = _value;
        emit Approval(msg.sender, _spender, _value);
        return true;
    }

    function allowance(address _owner, address _spender)
        view
        public
        returns (uint256 remaining)
    {
        return allowed[_owner][_spender];
    }

    function balanceOf(address _owner)
        view
        public
        returns (uint256 balance)
    {
        return balances[_owner];
    }
}
