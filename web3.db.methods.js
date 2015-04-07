var chai = require('chai');
var assert = chai.assert; 
var web3 = require('../index.js');
var u = require('./helpers/test.utils.js');

describe('web3.db', function() {
    describe('methods', function() {
        u.methodExists(web3.db, 'putHex');
        u.methodExists(web3.db, 'getHex');
        u.methodExists(web3.db, 'putString');
        u.methodExists(web3.db, 'getString');
    });
});

