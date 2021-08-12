pragma solidity >=0.4.0 <0.9.0;

contract ERC20 {
    event Transfer(address indexed from, address indexed to, uint256 value);
    event Approval(address indexed owner, address indexed spender, uint256 value);

    mapping (address => uint256) private _balances;
    mapping (address => mapping (address => uint256)) private _allowances;
    uint256 private _totalSupply;

    constructor() {
        _mint(msg.sender, 20);
    }

    function totalSupply() public view returns (uint256) {
        return _totalSupply;
    }

    function balanceOf(address owner) public view returns (uint256) {
        return _balances[owner];
    }

    function allowance(address owner, address spender) public view returns (uint256) {
        return _allowances[owner][spender];
    }

    function transfer(address to, uint256 value) public returns (bool) {
        _transfer(msg.sender, to, value);
        return true;
    }

    function approve(address spender, uint256 value) public returns (bool) {
        _approve(msg.sender, spender, value);
        return true;
    }

    function transferFrom(address from, address to, uint256 value) public returns (bool) {
        _transfer(from, to, value);
        // The subtraction here will revert on overflow.
        _approve(from, msg.sender, _allowances[from][msg.sender] - value);
        return true;
    }

    function increaseAllowance(address spender, uint256 addedValue) public returns (bool) {
        // The addition here will revert on overflow.
        _approve(msg.sender, spender, _allowances[msg.sender][spender] + addedValue);
        return true;
    }

    function decreaseAllowance(address spender, uint256 subtractedValue) public returns (bool) {
        // The subtraction here will revert on overflow.
        _approve(msg.sender, spender, _allowances[msg.sender][spender] - subtractedValue);
        return true;
    }

    function _transfer(address from, address to, uint256 value) internal {
        require(to != address(0), "ERC20: transfer to the zero address");

        // The subtraction and addition here will revert on overflow.
        _balances[from] = _balances[from] - value;
        _balances[to] = _balances[to] + value;
        emit Transfer(from, to, value);
    }

    function _mint(address account, uint256 value) internal {
        require(account != address(0), "ERC20: mint to the zero address");

        // The additions here will revert on overflow.
        _totalSupply = _totalSupply + value;
        _balances[account] = _balances[account] + value;
        emit Transfer(address(0), account, value);
    }

    function _burn(address account, uint256 value) internal {
        require(account != address(0), "ERC20: burn from the zero address");

        // The subtractions here will revert on overflow.
        _totalSupply = _totalSupply - value;
        _balances[account] = _balances[account] - value;
        emit Transfer(account, address(0), value);
    }

    function _approve(address owner, address spender, uint256 value) internal {
        require(owner != address(0), "ERC20: approve from the zero address");
        require(spender != address(0), "ERC20: approve to the zero address");

        _allowances[owner][spender] = value;
        emit Approval(owner, spender, value);
    }

    function _burnFrom(address account, uint256 value) internal {
        _burn(account, value);
        _approve(account, msg.sender, _allowances[account][msg.sender] - value);
    }
}
// ====
// compileViaYul: also
// ----
// constructor()
// ~ emit Transfer(address,address,uint256): #0x00, #0x1212121212121212121212121212120000000012, 0x14
// gas irOptimized: 438139
// gas legacy: 833310
// gas legacyOptimized: 401952
// totalSupply() -> 20
// gas irOptimized: 23415
// gas legacy: 23524
// gas legacyOptimized: 23368
// transfer(address,uint256): 2, 5 -> true
// ~ emit Transfer(address,address,uint256): #0x1212121212121212121212121212120000000012, #0x02, 0x05
// gas irOptimized: 48482
// gas legacy: 49317
// gas legacyOptimized: 48448
// decreaseAllowance(address,uint256): 2, 0 -> true
// ~ emit Approval(address,address,uint256): #0x1212121212121212121212121212120000000012, #0x02, 0x00
// gas irOptimized: 26275
// gas legacy: 27012
// gas legacyOptimized: 26241
// decreaseAllowance(address,uint256): 2, 1 -> FAILURE, hex"4e487b71", 0x11
// gas irOptimized: 24042
// gas legacy: 24467
// gas legacyOptimized: 24028
// transfer(address,uint256): 2, 14 -> true
// ~ emit Transfer(address,address,uint256): #0x1212121212121212121212121212120000000012, #0x02, 0x0e
// gas irOptimized: 28582
// gas legacy: 29417
// gas legacyOptimized: 28548
// transfer(address,uint256): 2, 2 -> FAILURE, hex"4e487b71", 0x11
// gas irOptimized: 24071
// gas legacy: 24453
// gas legacyOptimized: 24025
