pragma solidity ^0.4.11;

import "./module.sol";
import "./moduleHandler.sol";
import "./safeMath.sol";
import "./announcementTypes.sol";

contract provider is module, safeMath, announcementTypes {
    /*
        module callbacks
    */
    function connectModule() external returns (bool success) {
        require( super.isModuleHandler(msg.sender) );
        super._connectModule();
        (bool _success, uint256 currentSchellingRound) = moduleHandler(moduleHandlerAddress).getCurrentSchellingRoundID();
        require( _success );
        return true;
    }
    function transferEvent(address from, address to, uint256 value) external returns (bool success) {
        /*
            Transaction completed. This function is ony available for the modulehandler.
            It should be checked if the sender or the acceptor does not connect to the provider or it is not a provider itself if so than the change should be recorded.
            
            @from       From whom?
            @to         For who?
            @value      amount
            @bool       Was the function successful?
        */
        require( super.isModuleHandler(msg.sender) );
        transferEvent_(from, value, true);
        transferEvent_(to, value, false);
        return true;
    }
    function newSchellingRoundEvent(uint256 roundID, uint256 reward) external returns (bool success) {
        /*
            New schelling round. This function is only available for the moduleHandler.
            We are recording the new schelling round and we are storing the whole current quantity of the tokens.
            We generate a reward quantity of tokens directed to the providers address. The collected interest will be tranfered from this contract.
            
            @roundID        Number of the schelling round.
            @reward         token emission 
            @bool           Was the function successful?
        */
        require( super.isModuleHandler(msg.sender) );
        globalFunds[roundID].reward = reward;
        globalFunds[roundID].supply = globalFunds[roundID-1].supply;
        currentSchellingRound = roundID;
        require( moduleHandler(moduleHandlerAddress).mint(address(this), reward) );
        return true;
    }
    modifier isReady {
        (bool _success, bool _active) = super.isActive();
        require( _success && _active ); 
        _;
    }
    /*
        Provider module
    */
    uint256 private minFundsForPublic   = 3000;
    uint256 private minFundsForPrivate  = 8000;
    uint256 private privateProviderLimit  = 250;
    uint8 private publicMinRate     = 30;
    uint8 private privateMinRate    = 0;
    uint8 private publicMaxRate     = 70;
    uint8 private privateMaxRate    = 100;
    uint256 private gasProtectMaxRounds = 630;
    uint256 private interestMinFunds = 25000;
    uint256 private rentRate = 20;

    struct _rate {
        uint8 value;
        bool valid;
    }
    struct __providers {
        address admin;
        string name;
        string website;
        string country;
        string info;
        bool isForRent;
        mapping(uint256 => _rate) rateHistory;
        uint8 currentRate;
        bool priv;
        uint256 clientsCount;
        mapping(address => bool) allowedUsers;
        mapping(uint256 => uint256) supply;
        uint256 lastSupplyID;
        mapping(uint256 => uint256) ownSupply;
        uint256 lastOwnSupplyID;
        uint256 paidUpTo;
        uint8 lastPaidRate;
        uint256 create;
        uint256 close;
        bool valid;
    }
    struct _providers {
        mapping(uint256 => __providers) data;
        uint256 currentHeight;
    }
    mapping(address => _providers) private providers;
    
    struct _globalFunds {
        uint256 reward;
        uint256 supply;
    }
    mapping(uint256 => _globalFunds) private globalFunds;
    
    struct _client{
        address providerAddress;
        uint256 providerHeight;
        uint256 providerConnected;
        uint8 lastRate;
        mapping(uint256 => uint256) supply;
        uint256 lastSupplyID;
        uint256 paidUpTo;
    }
    mapping(address => _client) private clients;
    
    uint256 private currentSchellingRound = 1;

    function provider(address _moduleHandler) {
        /*
            Install function.
            
            @_moduleHandler     Address of the moduleHandler.
        */
        super.registerModuleHandler(_moduleHandler);
    }
    function configure(announcementType a, uint256 b) external returns(bool) {
        /*
            Configuration of the provider. Can be invited just by the moduleHandler.
            
            @a      Type of the setting
            @b      value
        */
        require( super.isModuleHandler(msg.sender) );
        if      ( a == announcementType.providerPublicFunds )          { minFundsForPublic = b; }
        else if ( a == announcementType.providerPrivateFunds )         { minFundsForPrivate = b; }
        else if ( a == announcementType.providerPrivateClientLimit )   { privateProviderLimit = b; }
        else if ( a == announcementType.providerPublicMinRate )        { publicMinRate = uint8(b); }
        else if ( a == announcementType.providerPublicMaxRate )        { publicMaxRate = uint8(b); }
        else if ( a == announcementType.providerPrivateMinRate )       { privateMinRate = uint8(b); }
        else if ( a == announcementType.providerPrivateMaxRate )       { privateMaxRate = uint8(b); }
        else if ( a == announcementType.providerGasProtect )           { gasProtectMaxRounds = b; }
        else if ( a == announcementType.providerInterestMinFunds )     { interestMinFunds = b; }
        else if ( a == announcementType.providerRentRate )             { rentRate = b; }
        else { return false; }
        return true;
    }
    function getUserDetails(address addr, uint256 schellingRound) public constant returns (address ProviderAddress, uint256 ProviderHeight, uint256 ConnectedOn, uint256 value) {
        /*
            Collecting the datas of the client.
            
            @addr               Address of the client.
            @schellingRound     Number of the schelling round. If it is not defined then the current one.
            @ProviderAddress    Address of the provider the one where connected to
            @ProviderHeight     The height (level) of the provider where is connected.
            @ConnectedOn        Time of connection
            @value              Quantity of the client’s token
        */
        if ( schellingRound == 0 ) {
            schellingRound = currentSchellingRound;
        }
        if ( clients[addr].providerAddress != 0 ) {
            ProviderAddress = clients[addr].providerAddress;
            ProviderHeight  = clients[addr].providerHeight;
            ConnectedOn     = clients[addr].providerConnected;
            value           = clients[addr].supply[schellingRound];
        }
    }
    function rightForInterest(uint256 value, bool priv) internal returns (bool) {
        /*
            the share from the token emission.
            In case is a private provider it has to be checked if it has enough connected capital to be able to accept share from the token emission.
            The provider’s account counts as a capital for the emission as well.
            
            @value      amount of the connected capital
            @priv       Is the provider private or not? 
            @bool       Gets the share from the token emission.
        */
        if ( priv ) {
            return ( value >= interestMinFunds );
        }
        return true;
    }
    function setRightForInterest(uint256 oldValue, uint256 newValue, bool priv) internal {
        /*
            It checks if the provider has enough connected captital to be able to get from the token emission.
            In case the provider is not able to get the share from the token emission then the connected capital will not count to the value of the globalFunds, to the current schelling round.
            
            @oldValue       old  
            @newValue       new
            @priv           Is the provider private?
        */
        bool a = rightForInterest(oldValue, priv);
        bool b = rightForInterest(newValue, priv);
        if ( a && b ) {
            globalFunds[currentSchellingRound].supply = globalFunds[currentSchellingRound].supply - oldValue + newValue;
        } else if ( a && ! b ) {
            globalFunds[currentSchellingRound].supply -= oldValue;
        } else if ( ! a && b ) {
            globalFunds[currentSchellingRound].supply += newValue;
        }
    }
    function checkCorrectRate(bool priv, uint8 rate) internal returns(bool) {
        /*
            Inner function which checks if the amount of interest what is given by the provider is fits to the criteria.
            
            @priv       Is the provider private or not?
            @rate       Percentage/rate of the interest
            @bool       Correct or not?
        */
        return ( ! priv && ( rate >= publicMinRate && rate <= publicMaxRate ) ) || 
                ( priv && ( rate >= privateMinRate && rate <= privateMaxRate ) );
    }
    function createProvider(bool priv, string name, string website, string country, string info, uint8 rate, bool isForRent, address admin) isReady external {
        /*
            Creating a provider.
            During the ICO its not allowed to create provider.
            To one address only one provider can belong to.
            Address, how is connected to the provider can not create a provider.
            For opening, has to have enough capital.
            All the functions of the provider except of the closing are going to be handled by the admin.
            The provider can be start as a rent as well, in this case the isForRent has to be true/correct. In case it runs as a rent the 20% of the profit will belong to the leser and the rest goes to the admin.
            
            @priv           Privat szolgaltato e. Is private provider?
            @name           Provider’s name.
            @website        Provider’s website
            @country        Provider’s country
            @info           Provider’s short introduction.
            @rate           Rate of the emission what is going to be transfered to the client by the provider.
            @isForRent      is for Rent or not?
            @admin          The admin’s address
        */
        require( ! providers[msg.sender].data[providers[msg.sender].currentHeight].valid );
        require( clients[msg.sender].providerAddress == 0x00 );
        require( ! checkICO() );
        if ( priv ) {
            require( getTokenBalance(msg.sender) >= minFundsForPrivate );
        } else {
            require( getTokenBalance(msg.sender) >= minFundsForPublic );
        }
        require( checkCorrectRate(priv, rate) );
        
        providers[msg.sender].currentHeight++;
        uint256 currHeight = providers[msg.sender].currentHeight;
        providers[msg.sender].data[currHeight].valid           = true;
        if ( admin == 0x00 ) { 
            providers[msg.sender].data[currHeight].admin      = msg.sender;
        } else {
            providers[msg.sender].data[currHeight].admin      = admin;
        }
        providers[msg.sender].data[currHeight].name            = name;
        providers[msg.sender].data[currHeight].website         = website;
        providers[msg.sender].data[currHeight].isForRent       = isForRent;
        providers[msg.sender].data[currHeight].country         = country;
        providers[msg.sender].data[currHeight].info            = info;
        providers[msg.sender].data[currHeight].currentRate     = rate;
        providers[msg.sender].data[currHeight].create          = now;
        providers[msg.sender].data[currHeight].lastPaidRate    = rate;
        providers[msg.sender].data[currHeight].priv            = priv;
        providers[msg.sender].data[currHeight].lastSupplyID    = currentSchellingRound;
        providers[msg.sender].data[currHeight].paidUpTo        = currentSchellingRound;
        if ( priv ) {
            providers[msg.sender].data[currHeight].supply[currentSchellingRound]        = getTokenBalance(msg.sender);
            providers[msg.sender].data[currHeight].ownSupply[currentSchellingRound]     = getTokenBalance(msg.sender);
            providers[msg.sender].data[currHeight].lastOwnSupplyID                      = currentSchellingRound;
        } else {
            delete providers[msg.sender].data[currHeight].supply[currentSchellingRound];
        }
        EProviderOpen(msg.sender, currHeight);
    }
    function setProviderDetails(address addr, string website, string country, string info, uint8 rate, address admin) isReady external {
        /*
            Modifying the datas of the provider.
            This can only be invited by the provider’s admin.
            The emission rate is only valid for the next schelling round for this one it is not.
            The admin can only be changed by the address of the provider.
            
            @addr               Address of the provider.
            @website            Website.
            @admin              The new address of the admin. If we do not want to set it then we should enter 0X00. 
            @country            Country
            @info               Short intro.
            @rate               Rate of the emission what will be given to the client.
        */
        uint256 currHeight = providers[addr].currentHeight;
        require( providers[addr].data[currHeight].valid );
        require( checkCorrectRate(providers[addr].data[currHeight].priv, rate) );
        require( providers[addr].data[currHeight].admin == msg.sender || msg.sender == addr );
        if ( admin != 0x00 ) {
            require( msg.sender == addr );
            providers[addr].data[currHeight].admin = admin;
        }
        providers[addr].data[currHeight].rateHistory[currentSchellingRound] = _rate( rate, true );
        providers[addr].data[currHeight].website         = website;
        providers[addr].data[currHeight].country         = country;
        providers[addr].data[currHeight].info            = info;
        providers[addr].data[currHeight].currentRate     = rate;
        EProviderDetailsChanged(addr, currHeight, website, country, info, rate, admin);
    }
    function getProviderInfo(address addr, uint256 height) public constant returns (string name, string website, string country, string info, uint256 create) {
        /*
            for the infos of the provider.
            In case the height is unknown then the system will use the last known height.
            
            @addr           Addr of the provider
            @height         Height
            @name           Name of the provider.
            @website        Website of the provider.
            @country        Country of the provider.
            @info           Short intro of the provider.
            @create         Timestamp of creating the provider
        */
        if ( height == 0 ) {
            height = providers[addr].currentHeight;
        }
        name            = providers[addr].data[height].name;
        website         = providers[addr].data[height].website;
        country         = providers[addr].data[height].country;
        info            = providers[addr].data[height].info;
        create          = providers[addr].data[height].create;
    }
    function getProviderDetails(address addr, uint256 height) public constant returns (uint8 rate, bool isForRent, uint256 clientsCount, bool priv, bool getInterest, bool valid) {
        /*
            Asking for the datas of the provider.
            In case the height is unknown then the system will use the last known height.

            @addr           Address of the provider
            @height         Height
            @rate           The rate of the emission which will be transfered to the client.
            @isForRent      Rent or not.
            @clientsCount   Number of the clients.
            @priv           Private or not?
            @getInterest    Does get from the token emission?
            @valid          Is an active provider?
        */
        if ( height == 0 ) {
            height = providers[addr].currentHeight;
        }
        rate            = providers[addr].data[height].currentRate;
        isForRent       = providers[addr].data[height].isForRent;
        clientsCount    = providers[addr].data[height].clientsCount;
        priv            = providers[addr].data[height].priv;
        getInterest     = rightForInterest(getProviderCurrentSupply(addr), providers[addr].data[height].priv );
        valid           = providers[addr].data[height].valid;
    }
    function getProviderCurrentSupply(address addr) internal returns (uint256) {
        /*
            Inner function for polling the current height and the current quantity of the connected capital of the schelling round.
            
            @addr           Provider’s address.
            @uint256        Amount of the connected capital
        */
        return providers[addr].data[providers[addr].currentHeight].supply[currentSchellingRound];
    }
    function closeProvider() isReady external {
        /*
            Closing and inactivate the provider.
            It is only possible to close that active provider which is owned by the sender itself after calling the whole share of the emission.
            Whom were connected to the provider those clients will have to disconnect after they’ve called their share of emission which was not called before.
        */
        uint256 currHeight = providers[msg.sender].currentHeight;
        require( providers[msg.sender].data[currHeight].valid );
        require( providers[msg.sender].data[currHeight].paidUpTo == currentSchellingRound );
        
        providers[msg.sender].data[currHeight].valid = false;
        providers[msg.sender].data[currHeight].close = currentSchellingRound;
        setRightForInterest(getProviderCurrentSupply(msg.sender), 0, providers[msg.sender].data[currHeight].priv);
        EProviderClose(msg.sender, currHeight);
    }
    function allowUsers(address provider, address[] addr) isReady external {
        /*
            Permition of the user to be able to connect to the provider.
            This can only be invited by the provider’s admin.
            With this kind of call only 100 address can be permited. 
            
            @addr       Array of the addresses for whom the connection is allowed.
        */
        uint256 currHeight = providers[provider].currentHeight;
        require( providers[provider].data[currHeight].valid );
        require( providers[provider].data[currHeight].priv );
        require( providers[provider].data[currHeight].admin == msg.sender );
        require( addr.length <= 100 );
        
        for ( uint256 a=0 ; a<addr.length ; a++ ) {
            providers[provider].data[currHeight].allowedUsers[addr[a]] = true;
        }
    }
    function disallowUsers(address provider, address[] addr) isReady external {
        /*
            Disable of the user not to be able to connect to the provider.
            It is can called only for the admin of the provider.
            With this kind of call only 100 address can be permited. 
            
            @addr      Array of the addresses for whom the connection is allowed.
        */
        uint256 currHeight = providers[provider].currentHeight;
        require( providers[provider].data[currHeight].valid );
        require( providers[provider].data[currHeight].priv );
        require( providers[provider].data[currHeight].admin == msg.sender );
        require( addr.length <= 100 );
        
        for ( uint256 a=0 ; a<addr.length ; a++ ) {
            delete providers[provider].data[currHeight].allowedUsers[addr[a]];
        }
    }
    function joinProvider(address provider) isReady external {
        /*
            Connection to the provider.
            Providers can not connect to other providers.
            If is a client at any provider, then it is not possible to connect to other provider one.
            It is only possible to connect to valid and active providers.
            If is an active provider then the client can only connect, if address is permited at the provider (Whitelist).
            At private providers, the number of the client is restricted. If it reaches the limit no further clients are allowed to connect.
            This process has a transaction fee based on the senders whole token quantity.
            
            @provider       Address of the provider.
        */
        uint256 currHeight = providers[provider].currentHeight;
        require( ! providers[msg.sender].data[currHeight].valid );
        require( clients[msg.sender].providerAddress == 0x00 );
        require( providers[provider].data[currHeight].valid );
        if ( providers[provider].data[currHeight].priv ) {
            require( providers[provider].data[currHeight].allowedUsers[msg.sender] &&
                     providers[provider].data[currHeight].clientsCount < privateProviderLimit );
        }
        uint256 bal = getTokenBalance(msg.sender);
        require( moduleHandler(moduleHandlerAddress).processTransactionFee(msg.sender, bal) );
        
        checkFloatingSupply(provider, currHeight, false, bal);
        providers[provider].data[currHeight].clientsCount++;
        clients[msg.sender].providerAddress = provider;
        clients[msg.sender].providerHeight = currHeight;
        clients[msg.sender].supply[currentSchellingRound] = bal;
        clients[msg.sender].lastSupplyID = currentSchellingRound;
        clients[msg.sender].paidUpTo = currentSchellingRound;
        clients[msg.sender].lastRate = providers[provider].data[currHeight].currentRate;
        clients[msg.sender].providerConnected = now;
        ENewClient(msg.sender, provider, currHeight, bal);
    }
    function partProvider() isReady external {
        /*
            Disconnecting from the provider.
            Before disconnecting we should poll our share from the token emission even if there was nothing factually.
            It is only possible to disconnect those providers who were connected by us before.
        */
        address provider = clients[msg.sender].providerAddress;
        require( provider != 0x0 );
        uint256 currHeight = clients[msg.sender].providerHeight;
        bool providerHasClosed = false;
        if ( providers[provider].data[currHeight].close > 0 ) {
            providerHasClosed = true;
            require( clients[msg.sender].paidUpTo == providers[provider].data[currHeight].close );
        } else {
            require( clients[msg.sender].paidUpTo == currentSchellingRound );
        }
        
        uint256 bal = getTokenBalance(msg.sender);
        if ( ! providerHasClosed ) {
            providers[provider].data[currHeight].clientsCount--;
            checkFloatingSupply(provider, currHeight, true, bal);
        }
        delete clients[msg.sender].providerAddress;
        delete clients[msg.sender].providerHeight;
        delete clients[msg.sender].lastSupplyID;
        delete clients[msg.sender].paidUpTo;
        delete clients[msg.sender].lastRate;
        delete clients[msg.sender].providerConnected;
        EClientLost(msg.sender, provider, currHeight, bal);
    }
    function checkReward(address addr) public constant returns (uint256 reward) {
        /*
            Polling the share from the token emission for clients and for providers.
            
            @addr           The address want to check.
            @reward         Accumulated amount.
        */
        if ( providers[addr].data[providers[addr].currentHeight].valid ) {
            uint256 a;
            (reward, a) = getProviderReward(addr, 0);
        } else if ( clients[addr].providerAddress != 0x0 ) {
            reward = getClientReward(0);
        }
    }
    function getReward(address beneficiary, uint256 limit, address provider) isReady external returns (uint256 reward) {
        /*
            Polling the share from the token emission token emission for clients and for providers.

            It is optionaly possible to give an address of a beneficiary for whom we can transfer the accumulated amount. In case we don’t enter any address then the amount will be transfered to the caller’s address.
            As the interest should be checked at each schelling round in order to get the share from that so to avoid the overflow of the gas the number of the check-rounds should be limited.
            Opcionalisan megadhato az ellenorzes koreinek szama. It is possible to enter optionaly the number of the check-rounds.  If it is 0 then it is automatic.
            Provider variable should only be entered if the real owner of the provider is not the caller’s address.
            In case the client/provider was far behind then it is possible that this function should be called several times to check the total generated schelling rounds and to collect the share.
            If is neighter a client nor a provider then the function is not available.
            The tokens will be sent to the beneficiary from the address of the provider without any transaction fees.
            
            @beneficiary        Address of the beneficiary
            @limit              Quota of the check-rounds.
            @provider           Address of the provider
            @reward             Accumulated amount from the previous rounds.
        */
        uint256 _limit = limit;
        address _beneficiary = beneficiary;
        address _provider = provider;
        if ( _limit == 0 ) { _limit = gasProtectMaxRounds; }
        if ( _beneficiary == 0x00 ) { _beneficiary = msg.sender; }
        if ( _provider == 0x00 ) { _provider = msg.sender; }
        uint256 clientReward;
        uint256 providerReward;
        if ( providers[_provider].data[providers[_provider].currentHeight].valid ) {
            require( providers[_provider].data[providers[_provider].currentHeight].admin == msg.sender || msg.sender == _provider );
            (providerReward, clientReward) = getProviderReward(_provider, _limit);
        } else if ( clients[msg.sender].providerAddress != 0x00 ) {
            clientReward = getClientReward(_limit);
        } else {
            throw;
        }
        if ( clientReward > 0 ) {
            require( moduleHandler(moduleHandlerAddress).transfer(address(this), _beneficiary, clientReward, false) );
        }
        if ( providerReward > 0 ) {
            require( moduleHandler(moduleHandlerAddress).transfer(address(this), provider, providerReward, false) );
        }
        EReward(msg.sender, provider, clientReward, providerReward);
    }
    function getClientReward(uint256 limit) internal returns (uint256 reward) {
        /*
            Inner function for the client in order to collect the share from the token emission
            
            @limit          Quota of checking the schelling-rounds.
            @reward         Collected token amount from the checked rounds.
        */
        uint256 value;
        uint256 steps;
        address provAddr;
        uint256 provHeight;
        bool interest = false;
        uint256 a;
        uint8 rate = clients[msg.sender].lastRate;
        for ( a = (clients[msg.sender].paidUpTo + 1) ; a <= currentSchellingRound ; a++ ) {
            if (globalFunds[a].reward > 0 && globalFunds[a].supply > 0) {
                provAddr = clients[msg.sender].providerAddress;
                provHeight = clients[msg.sender].providerHeight;
                if ( providers[provAddr].data[provHeight].rateHistory[a].valid ) {
                    rate = providers[provAddr].data[provHeight].rateHistory[a].value;
                }
                if ( rate > 0 ) {
                    if ( a > providers[provAddr].data[provHeight].lastSupplyID ) {
                        interest = rightForInterest(providers[provAddr].data[provHeight].supply[providers[provAddr].data[provHeight].lastSupplyID], providers[provAddr].data[provHeight].priv);
                    } else {
                        interest = rightForInterest(providers[provAddr].data[provHeight].supply[a], providers[provAddr].data[provHeight].priv);
                    }
                    if ( interest ) {
                        if ( limit > 0 && steps > limit ) {
                            a--;
                            break;
                        }
                        if (clients[msg.sender].lastSupplyID < a) {
                            value = clients[msg.sender].supply[clients[msg.sender].lastSupplyID];
                        } else {
                            value = clients[msg.sender].supply[a];
                        }
                        if ( globalFunds[a].supply > 0) {
                            reward += value * globalFunds[a].reward / globalFunds[a].supply * uint256(rate) / 100;
                        }
                        steps++;
                    }
                }
            }
        }
        clients[msg.sender].lastRate = rate;
        clients[msg.sender].paidUpTo = a-1;
    }
    function getProviderReward(address addr, uint256 limit) internal returns (uint256 providerReward, uint256 adminReward) {
        /*
            Inner function for the provider in order to collect the share from the token emission            
            @addr               Address of the provider.
            @limit              Quota of the check-rounds.
            @providerReward     The reward of the provider’s address from the checked rounds.
            @adminReward        Admin’s reward from the checked rounds.
        */
        uint256 reward;
        uint256 ownReward;
        uint256 value;
        uint256 steps;
        uint256 currHeight = providers[addr].currentHeight;
        uint256 LTSID = providers[addr].data[currHeight].lastSupplyID;
        uint256 a;
        uint8 rate = providers[addr].data[currHeight].lastPaidRate;
        for ( a = (providers[addr].data[currHeight].paidUpTo + 1) ; a <= currentSchellingRound ; a++ ) {
            if (globalFunds[a].reward > 0 && globalFunds[a].supply > 0) {
                if ( providers[addr].data[currHeight].rateHistory[a].valid ) {
                    rate = providers[addr].data[currHeight].rateHistory[a].value;
                }
                if ( rate > 0 ) {
                    if ( ( a > LTSID && rightForInterest(providers[addr].data[currHeight].supply[LTSID], providers[addr].data[currHeight].priv) || 
                        rightForInterest(providers[addr].data[currHeight].supply[a], providers[addr].data[currHeight].priv) ) ) {
                        if ( limit > 0 && steps > limit ) {
                            a--;
                            break;
                        }
                        if ( LTSID < a ) {
                            value = providers[addr].data[currHeight].supply[LTSID];
                        } else {
                            value = providers[addr].data[currHeight].supply[a];
                        }
                        if ( globalFunds[a].supply > 0) {
                            reward += value * globalFunds[a].reward / globalFunds[a].supply * ( 100 - uint256(rate) ) / 100;
                            if ( providers[addr].data[currHeight].priv ) {
                                LTSID = providers[addr].data[currHeight].lastOwnSupplyID;
                                if ( LTSID < a ) {
                                    value = providers[addr].data[currHeight].ownSupply[LTSID];
                                } else {
                                    value = providers[addr].data[currHeight].ownSupply[a];
                                }
                                ownReward += value * globalFunds[a].reward / globalFunds[a].supply;
                            }
                        }
                        steps++;
                    }
                }
            }
        }
        providers[addr].data[currHeight].lastPaidRate = uint8(rate);
        providers[addr].data[currHeight].paidUpTo = a-1;
        if ( providers[addr].data[currHeight].isForRent ) {
            providerReward = reward * rentRate / 100;
            adminReward = reward - providerReward;
            if ( providers[addr].data[currHeight].priv ) { providerReward += ownReward; }
        } else {
            providerReward = reward + ownReward;
        }
    }
    function checkFloatingSupply(address providerAddress, uint256 providerHeight, bool neg, uint256 value) internal {
        /*
            Inner function for updating the database when some token change has happened.
            In this case we are checking if despite the changes the provider is still entitled to the token emission. In case the legitimacy changes then the global supply should be set as well.
            
            @providerAddress        Provider address.
            @providerHeight         Provider height.
            @neg                    the change was negative or not
            @value                  Rate of the change
        */
        uint256 LSID = providers[providerAddress].data[providerHeight].lastSupplyID;
        if ( currentSchellingRound != LSID ) {
            if ( neg ) {
                setRightForInterest(
                    providers[providerAddress].data[providerHeight].supply[LSID], 
                    providers[providerAddress].data[providerHeight].supply[LSID] - value, 
                    providers[providerAddress].data[providerHeight].priv
                );
                providers[providerAddress].data[providerHeight].supply[currentSchellingRound] = providers[providerAddress].data[providerHeight].supply[LSID] - value;
            } else {
                setRightForInterest(
                    providers[providerAddress].data[providerHeight].supply[LSID], 
                    providers[providerAddress].data[providerHeight].supply[LSID] + value, 
                    providers[providerAddress].data[providerHeight].priv
                );
                providers[providerAddress].data[providerHeight].supply[currentSchellingRound] = providers[providerAddress].data[providerHeight].supply[LSID] + value;
            }
            providers[providerAddress].data[providerHeight].lastSupplyID = currentSchellingRound;
        } else {
            if ( neg ) {
                setRightForInterest(
                    getProviderCurrentSupply(providerAddress), 
                    getProviderCurrentSupply(providerAddress) - value, 
                    providers[providerAddress].data[providerHeight].priv
                );
                providers[providerAddress].data[providerHeight].supply[currentSchellingRound] -= value;
            } else {
                setRightForInterest(
                    getProviderCurrentSupply(providerAddress), 
                    getProviderCurrentSupply(providerAddress) + value, 
                    providers[providerAddress].data[providerHeight].priv
                );
                providers[providerAddress].data[providerHeight].supply[currentSchellingRound] += value;
            }
        }
    }
    function checkFloatingOwnSupply(address providerAddress, uint256 providerHeight, bool neg, uint256 value) internal {
        /*
            Inner function for updating the database in case token change has happened.
            In this case we check if the provider despite the changes is still entitled to the token emission.
            We just call this only if the private provider and it’s own capital bears emission.
            
            @providerAddress        Provider address.
            @providerHeight         Provider height.
            @neg                    Was the change negative?
            @value                  Rate of the change.
        */
        uint256 LSID = providers[providerAddress].data[providerHeight].lastOwnSupplyID;
        if ( currentSchellingRound != LSID ) {
            if ( neg ) {
                setRightForInterest(
                    providers[providerAddress].data[providerHeight].ownSupply[LSID], 
                    providers[providerAddress].data[providerHeight].ownSupply[LSID] - value, 
                    true
                );
                providers[providerAddress].data[providerHeight].ownSupply[currentSchellingRound] = providers[providerAddress].data[providerHeight].ownSupply[LSID] - value;
            } else {
                setRightForInterest(
                    providers[providerAddress].data[providerHeight].ownSupply[LSID], 
                    providers[providerAddress].data[providerHeight].ownSupply[LSID] + value, 
                    true
                );
                providers[providerAddress].data[providerHeight].ownSupply[currentSchellingRound] = providers[providerAddress].data[providerHeight].ownSupply[LSID] + value;
            }
            providers[providerAddress].data[providerHeight].lastOwnSupplyID = currentSchellingRound;
        } else {
            if ( neg ) {
                setRightForInterest(
                    getProviderCurrentSupply(providerAddress), 
                    getProviderCurrentSupply(providerAddress) - value, 
                    true
                );
                providers[providerAddress].data[providerHeight].ownSupply[currentSchellingRound] -= value;
            } else {
                setRightForInterest(
                    getProviderCurrentSupply(providerAddress), 
                    getProviderCurrentSupply(providerAddress) + value, 
                    true
                );
                providers[providerAddress].data[providerHeight].ownSupply[currentSchellingRound] += value;
            }
        }
    }
    function TEMath(uint256 a, uint256 b, bool neg) internal returns (uint256) {
        /*
            Inner function for the changes of the numbers
            
            @a      First number
            @b      2nd number
            @neg    Operation with numbers. If it is TRUE then subtraction, if it is FALSE then addition.
        */
        if ( neg ) { return a-b; }
        else { return a+b; }
    }
    function transferEvent_(address addr, uint256 value, bool neg) internal {
        /*
            Inner function for perceiving the changes of the balance and updating the database.
            If the address is a provider and the balance is decreasing than can not let it go under the minimum level.
            
            @addr       The address where the change happened.
            @value      Rate of the change.
            @neg        ype of the change. If it is TRUE then the balance has been decreased if it is FALSE then it has been increased.
        */
        if ( clients[addr].providerAddress != 0 ) {
            checkFloatingSupply(clients[addr].providerAddress, providers[clients[addr].providerAddress].currentHeight, ! neg, value);
            if (clients[addr].lastSupplyID != currentSchellingRound) {
                clients[addr].supply[currentSchellingRound] = TEMath(clients[addr].supply[clients[addr].lastSupplyID], value, neg);
                clients[addr].lastSupplyID = currentSchellingRound;
            } else {
                clients[addr].supply[currentSchellingRound] = TEMath(clients[addr].supply[currentSchellingRound], value, neg);
            }
        } else if ( providers[addr].data[providers[addr].currentHeight].valid ) {
            uint256 currentHeight = providers[addr].currentHeight;
            if ( neg ) {
                uint256 balance = getTokenBalance(addr);
                if ( providers[addr].data[currentHeight].priv ) {
                    require( balance-value >= minFundsForPrivate );
                } else {
                    require( balance-value >= minFundsForPublic );
                }
            }
            if ( providers[addr].data[currentHeight].priv ) {
                checkFloatingOwnSupply(addr, currentHeight, ! neg, value);
            }
        }
    }
    function getTokenBalance(address addr) internal returns (uint256 balance) {
        /*
            Inner function in order to poll the token balance of the address.
            
            @addr       Address
            
            @balance    Balance of the address.
        */
        (bool _success, uint256 _balance) = moduleHandler(moduleHandlerAddress).balanceOf(addr);
        require( _success );
        return _balance;
    }
    function checkICO() internal returns (bool isICO) {
        /*
            Inner function to check the ICO status.
            
            @isICO      Is the ICO in proccess or not?
        */
        (bool _success, bool _isICO) = moduleHandler(moduleHandlerAddress).isICO();
        require( _success );
        return _isICO;
    }
    event EProviderOpen(address addr, uint256 height);
    event EClientLost(address indexed client, address indexed provider, uint256 height, uint256 indexed value);
    event ENewClient(address indexed client, address indexed provider, uint256 height, uint256 indexed value);
    event EProviderClose(address indexed addr, uint256 height);
    event EProviderDetailsChanged(address indexed addr, uint256 height, string website, string country, string info, uint8 rate, address admin);
    event EReward(address indexed client, address indexed provider, uint256 clientreward, uint256 providerReward);
}
