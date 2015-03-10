var assert = require('assert');
var web3 = require('../index.js');
var u = require('./test.utils.js');

describe('web3', function() {
    describe('net', function() {
        u.propertyExists(web3.net, 'listening');
        u.propertyExists(web3.net, 'peerCount');
    });
});