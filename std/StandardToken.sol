import "./Token.sol";

contract StandardToken is Token {
	uint256 public totalSupply;
	mapping (address => uint256) public balanceOf;
	mapping (address =>
		mapping (address => uint256)) public allowance;

	function StandardToken(address _initialOwner, uint256 _supply) {
		totalSupply = _supply;
		balanceOf[_initialOwner] = _supply;
	}

	function transfer(address _to, uint256 _value) returns (bool success) {
		if (balanceOf[msg.sender] >= _value && balanceOf[_to] + _value >= balanceOf[_to]) {
			balanceOf[msg.sender] -= _value;
			balanceOf[_to] += _value;
			Transfer(msg.sender, _to, _value);
			return true;
		} else {
			return false;
		}
	}

	function transferFrom(address _from, address _to, uint256 _value) returns (bool success) {
		if (allowance[_from][msg.sender] >= _value && balanceOf[_to] + _value >= balanceOf[_to]) {
			allowance[_from][msg.sender] -= _value;
			balanceOf[_to] += _value;
			Transfer(_from, _to, _value);
			return true;
		} else {
			return false;
		}
	}

	function approve(address _spender, uint256 _value) returns (bool success) {
		allowance[msg.sender][_spender] = _value;
		Approval(msg.sender, _spender, _value);
		return true;
	}
}
