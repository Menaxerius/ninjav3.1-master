{
	"bindAddress": "0.0.0.0",
	"listenPort": "8124",
	"documentRoot": "./static",
	"databasePoolSize": "350",
	"databaseURI": "unix:///run/mysqld/mysqld.sock",
	"databaseSchema": "burstpool",
	"databaseUser": "DBUSERNAME",
	"databasePassword": "SECRETDBPASS",
	
	"pushURL": "https://api.pushover.net/1/messages.json",
	"pushKey": "YOUROWNPUSHKEY",
	"pushRecipient": "YOUROWNPUSHPASS",

	"burstServers": [ "pascal.to:8135", "bwallet.burstmining.club:8125"],
	"burstServerTimeout": 2,

	"poolName": "Flex Pool",
	"poolHostname": "pool.flexasset.club",

	"accountUpdateTimeout": 600,
        "poolNumericAccountId": "YOURACCOUNTID",
        "poolEncodedPassphrase": "YOURPOOLPASS",

	"poolDeadlineLimit": "259200000",
	"poolDeadlineReallyBad": "83113904000022",

	"historicBlockCount": 500,
	"capacityBlockCount": 500,

	"submitNonceCooloff": 0,

	"poolFeePercent": 1.0,
	"poolFeeNumericAccountId": "8914871999082705304",

	"minimumPayout": 300,
	
	"sharePowerFactor": 1.2,
	"currentBlockRewardPercent": 60,

	"recentBlockHistoryDepth": 40,

	"maximumPayoutBlockDelay": 360,
	"minimumPayoutBlockDelay": 4,

	"bonusNumericAccountId": "2963154836836381611",
	"bonusEncodedPassphrase": "YOURPASS",
	"bonusMinimumThreshold": 20
}
