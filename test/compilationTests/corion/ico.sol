pragma solidity ^0.4.11;

import "./safeMath.sol";
import "./token.sol";
import "./premium.sol";
import "./moduleHandler.sol";

contract ico is safeMath {
    
    struct icoLevels_s {
        uint256 block;
        uint8 rate;
    }
    struct affiliate_s {
        uint256 weight;
        uint256 paid;
    }
    struct interest_s {
        uint256 amount;
        bool empty;
    }
    struct brought_s {
        uint256 eth;
        uint256 cor;
        uint256 corp;
    }
    
    uint256 constant oneSegment = 40320;
    
    address public owner;
    address public tokenAddr;
    address public premiumAddr;
    uint256 public startBlock;
    uint256 public icoDelay;
    address public foundationAddress;
    address public icoEtcPriceAddr;
    uint256 public icoExchangeRate;
    uint256 public icoExchangeRateSetBlock;
    uint256 constant icoExchangeRateM = 1e4;
    uint256 constant interestOnICO   = 25;
    uint256 constant interestOnICOM  = 1e3;
    uint256 constant interestBlockDelay = 720;
    uint256 constant exchangeRateDelay = 125;
    bool public aborted;
    bool public closed;
    icoLevels_s[] public icoLevels;
    mapping (address => affiliate_s) public affiliate;
    mapping (address => brought_s) public brought;
    mapping (address => mapping(uint256 => interest_s)) public interestDB;
    uint256 public totalMint;
    uint256 public totalPremiumMint;
    
    constructor(address foundation, address priceSet, uint256 exchangeRate, uint256 startBlockNum, address[] genesisAddr, uint256[] genesisValue) {
        /*
            Installation function.
            
            @foundation     The ETC address of the foundation
            @priceSet       The address which will be able to make changes on the rate later on.
            @exchangeRate   The current ETC/USD rate multiplied by 1e4. For example: 2.5 USD/ETC = 25000
            @startBlockNum  The height (level) of the beginning of the ICO. If it is 0 then it will be the current array’s height.
            @genesisAddr    Array of Genesis addresses
            @genesisValue   Array of balance of genesis addresses
        */
        foundationAddress = foundation;
        icoExchangeRate = exchangeRate;
        icoExchangeRateSetBlock = block.number + exchangeRateDelay;
        icoEtcPriceAddr = priceSet;
        owner = msg.sender;
        if ( startBlockNum > 0 ) {
            require( startBlockNum >= block.number );
            startBlock = startBlockNum;
        } else {
            startBlock = block.number;
        }
        icoLevels.push(icoLevels_s(startBlock + oneSegment * 1, 3));
        icoLevels.push(icoLevels_s(startBlock + oneSegment / 7, 5));
        icoLevels.push(icoLevels_s(startBlock, 10));
        icoDelay = startBlock + oneSegment * 3;
        for ( uint256 a=0 ; a<genesisAddr.length ; a++ ) {
            interestDB[genesisAddr[a]][0].amount = genesisValue[a];
        }
    }
    
    function ICObonus() public constant returns(uint256 bonus) {
        /*
            Query of current bonus
            
            @bonus  Bonus %
        */
        for ( uint8 a=0 ; a<icoLevels.length ; a++ ) {
            if ( block.number > icoLevels[a].block ) {
                return icoLevels[a].rate;
            }
        }
    }
    
    function setInterestDB(address addr, uint256 balance) external returns(bool success) {
        /*
            Setting interest database. It can be requested by Token contract only.
            A database has to be built in order  that after ICO closed everybody can get their compound interest on their capital accumulated 
            
            @addr       Sender
            @balance    Quantity
            
            @success    Was the process successful or not
        */
        require( msg.sender == tokenAddr );
        uint256 _num = (block.number - startBlock) / interestBlockDelay;
        interestDB[addr][_num].amount = balance;
        if ( balance == 0 ) { 
            interestDB[addr][_num].empty = true;
        }
        return true;
    }
    
    function checkInterest(address addr) public constant returns(uint256 amount) {
        /*
            Query of compound interest
            
            @addr       Address
            
            @amount     Amount of compound interest
        */
        uint256 _lastBal;
        uint256 _tamount;
        bool _empty;
        interest_s memory _idb;
        uint256 _to = (block.number - startBlock) / interestBlockDelay;
        
        if ( _to == 0 || aborted ) { return 0; }
        
        for ( uint256 r=0 ; r < _to ; r++ ) {
            if ( r*interestBlockDelay+startBlock >= icoDelay ) { break; }
            _idb = interestDB[addr][r];
            if ( _idb.amount > 0 ) {
                if ( _empty ) {
                    _lastBal = _idb.amount + amount;
                } else {
                    _lastBal = _idb.amount;
                }
            }
            if ( _idb.empty ) {
                _lastBal = 0;
                _empty = _idb.empty;
            }
            _lastBal += _tamount;
            _tamount = _lastBal * interestOnICO / interestOnICOM / 100;
            amount += _tamount;
        }
    }
    
    function getInterest(address beneficiary) external {
        /*
            Request of  compound interest. This is deleted  from the database after the ICO closed and following the query of the compound interest.
            
            @beneficiary    Beneficiary who will receive the interest
        */
        uint256 _lastBal;
        uint256 _tamount;
        uint256 _amount;
        bool _empty;
        interest_s memory _idb;
        address _addr = beneficiary;
        uint256 _to = (block.number - startBlock) / interestBlockDelay;
        if ( _addr == address(0x00) ) { _addr = msg.sender; }
        
        require( block.number > icoDelay );
        require( ! aborted );
        
        for ( uint256 r=0 ; r < _to ; r++ ) {
            if ( r*interestBlockDelay+startBlock >= icoDelay ) { break; }
            _idb = interestDB[msg.sender][r];
            if ( _idb.amount > 0 ) {
                if ( _empty ) {
                    _lastBal = _idb.amount + _amount;
                } else {
                    _lastBal = _idb.amount;
                }
            }
            if ( _idb.empty ) {
                _lastBal = 0;
                _empty = _idb.empty;
            }
            _lastBal += _tamount;
            _tamount = _lastBal * interestOnICO / interestOnICOM / 100;
            _amount += _tamount;
            delete interestDB[msg.sender][r];
        }
        
        require( _amount > 0 );
        token(tokenAddr).mint(_addr, _amount);
    }
    
    function setICOEthPrice(uint256 value) external {
        /*
            Setting of the ICO ETC USD rates which can only be calle by a pre-defined address. 
            After this function is completed till the call of the next function (which is at least an exchangeRateDelay array) this rate counts.
            With this process avoiding the sudden rate changes.
            
            @value  The ETC/USD rate multiplied by 1e4. For example: 2.5 USD/ETC = 25000
        */
        require( isICO() );
        require( icoEtcPriceAddr == msg.sender );
        require( icoExchangeRateSetBlock < block.number);
        icoExchangeRateSetBlock = block.number + exchangeRateDelay;
        icoExchangeRate = value;
    }
    
    function extendICO() external {
        /*
            Extend the period of the ICO with one segment.
            
            It is only possible during the ICO and only callable by the owner.
        */
        require( isICO() );
        require( msg.sender == owner );
        icoDelay += oneSegment;
    }
    
    function closeICO() external {
        /*
            Closing the ICO.
            It is only possible when the ICO period passed and only by the owner.
            The 96% of the whole amount of the token is generated to the address of the fundation.
            Ethers which are situated in this contract will be sent to the address of the fundation.
        */
        require( msg.sender == owner );
        require( block.number > icoDelay );
        require( ! closed );
        closed = true;
        require( ! aborted );
        require( token(tokenAddr).mint(foundationAddress, token(tokenAddr).totalSupply() * 96 / 100) );
        require( premium(premiumAddr).mint(foundationAddress, totalMint / 5000 - totalPremiumMint) );
        require( foundationAddress.send(this.balance) );
        require( token(tokenAddr).closeIco() );
        require( premium(premiumAddr).closeIco() );
    }
    
    function abortICO() external {
        /*
            Withdrawal of the ICO.            
            It is only possible during the ICO period.
            Only callable by the owner.
            After this process only the receiveFunds function will be available for the customers.
        */
        require( isICO() );
        require( msg.sender == owner );
        aborted = true;
    }
    
    function connectTokens(address tokenContractAddr, address premiumContractAddr) external {
        /*
            Installation function which joins the two token contracts with this contract.
            Only callable by the owner
            
            @tokenContractAddr      Address of the corion token contract.
            @premiumContractAddr    Address of the corion premium token contract
        */
        require( msg.sender == owner );
        require( tokenAddr == address(0x00) && premiumAddr == address(0x00) );
        tokenAddr = tokenContractAddr;
        premiumAddr = premiumContractAddr;
    }
    
    function receiveFunds() external {
        /*
            Refund the amount which was purchased during the ICO period.
            This one is only callable if the ICO is withdrawn.
            In this case the address gets back the 90% of the amount which was spent for token during the ICO period.
        */
        require( aborted );
        require( brought[msg.sender].eth > 0 );
        uint256 _val = brought[msg.sender].eth * 90 / 100;
        delete brought[msg.sender];
        require( msg.sender.send(_val) );
    }
    
    function () external payable {
        /*
            Callback function. Simply calls the buy function as a beneficiary and there is no affilate address.
            If they call the contract without any function then this process will be taken place.
        */
        require( isICO() );
        require( buy(msg.sender, address(0x00)) );
    }
    
    function buy(address beneficiaryAddress, address affilateAddress) payable returns (bool success) {
        /*
            Buying a token
            
            If there is not at least 0.2 ether balance on the beneficiaryAddress then the amount of the ether which was intended for the purchase will be reduced by 0.2 and that will be sent to the address of the beneficiary.
            From the remaining amount calculate the reward with the help of the getIcoReward function.
            Only that affilate address is valid which has some token on it’s account.
            If there is a valid affilate address then calculate and credit the reward as well in the following way:
            With more than 1e12 token contract credit 5% reward based on the calculation that how many tokens did they buy when he was added as an affilate.
                More than 1e11 token: 4%
                More than 1e10 token: 3%
                More than 1e9 token: 2% below 1%
            @beneficiaryAddress     The address of the accredited where the token will be sent.
            @affilateAddress        The address of the person who offered who will get the referral reward. It can not be equal with the beneficiaryAddress.
        */
        require( isICO() );
        if ( beneficiaryAddress == address(0x00)) { beneficiaryAddress = msg.sender; }
        if ( beneficiaryAddress == affilateAddress ) {
            affilateAddress = address(0x00);
        }
        uint256 _value = msg.value;
        if ( beneficiaryAddress.balance < 0.2 ether ) {
            require( beneficiaryAddress.send(0.2 ether) );
            _value = safeSub(_value, 0.2 ether);
        }
        uint256 _reward = getIcoReward(_value);
        require( _reward > 0 );
        require( token(tokenAddr).mint(beneficiaryAddress, _reward) );
        brought[beneficiaryAddress].eth = safeAdd(brought[beneficiaryAddress].eth, _value);
        brought[beneficiaryAddress].cor = safeAdd(brought[beneficiaryAddress].cor, _reward);
        totalMint = safeAdd(totalMint, _reward);
        require( foundationAddress.send(_value * 10 / 100) );
        uint256 extra;
        if ( affilateAddress != address(0x00) && ( brought[affilateAddress].eth > 0 || interestDB[affilateAddress][0].amount > 0 ) ) {
            affiliate[affilateAddress].weight = safeAdd(affiliate[affilateAddress].weight, _reward);
            extra = affiliate[affilateAddress].weight;
            uint256 rate;
            if (extra >= 1e12) {
                rate = 5;
            } else if (extra >= 1e11) {
                rate = 4;
            } else if (extra >= 1e10) {
                rate = 3;
            } else if (extra >= 1e9) { 
                rate = 2;
            } else {
                rate = 1;
            }
            extra = safeSub(extra * rate / 100, affiliate[affilateAddress].paid);
            affiliate[affilateAddress].paid = safeAdd(affiliate[affilateAddress].paid, extra);
            token(tokenAddr).mint(affilateAddress, extra);
        }
        checkPremium(beneficiaryAddress);
        emit EICO(beneficiaryAddress, _reward, affilateAddress, extra);
        return true;
    }

    function checkPremium(address owner) internal {
        /*
            Crediting the premium token
        
            @owner The corion token balance of this address will be set based on the calculation which shows that how many times can be the amount of the purchased tokens devided by 5000. So after each 5000 token we give 1 premium token.
        */
        uint256 _reward = (brought[owner].cor / 5e9) - brought[owner].corp;
        if ( _reward > 0 ) {
            require( premium(premiumAddr).mint(owner, _reward) );
            brought[owner].corp = safeAdd(brought[owner].corp, _reward);
            totalPremiumMint = safeAdd(totalPremiumMint, _reward);
        }
    }
    
    function getIcoReward(uint256 value) public constant returns (uint256 reward) {
        /*
            Expected token volume at token purchase
            
            @value The amount of ether for the purchase
            @reward Amount of the token
                x = (value * 1e6 * USD_ETC_exchange rate / 1e4 / 1e18) * bonus percentage
                2.700000 token = (1e18 * 1e6 * 22500 / 1e4 / 1e18) * 1.20
        */
        reward = (value * 1e6 * icoExchangeRate / icoExchangeRateM / 1 ether) * (ICObonus() + 100) / 100;
        if ( reward < 5e6) { return 0; }
    }
    
    function isICO() public constant returns (bool success) {
        return startBlock <= block.number && block.number <= icoDelay && ( ! aborted ) && ( ! closed );
    }
    
    event EICO(address indexed Address, uint256 indexed value, address Affilate, uint256 AffilateValue);
}
