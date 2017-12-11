var shares = [];
var historicShares = [];
var currentBlockStart;
var accountsCache = {};
var resizeTimeout;


HTMLTableRowElement.prototype.appendTd = function(id, className, content) {
	var td = document.createElement('TD');
	this.appendChild(td);
	
	if (id)
		td.id = id;
	
	if (className)
		td.className = className;
	
	if (content != undefined) {
		if ( typeof(content) === "string" || typeof(content) === "number" )
			td.appendChild( document.createTextNode(content) );
		else
			td.appendChild(content);
	}
	
	return td;
}


function XHR(options) {
	var xhr = new XMLHttpRequest();
	
	if ("form" in options) {
		options.data = new FormData(options.form);
		
		if ( !("url" in options) )
			options.url = options.form.action;
			
		if ( !("method" in options) && ("method" in options.form) )
			options.method = options.form.method;
	}

	if ("json" in options) {
		options.data = JSON.stringify(options.json);
		options.method = "POST";
		options.responseType = "json";
	}
	
	if ( !("method" in options) )
		options.method = "GET";
	
	if ("responseType" in options)
		try {
			xhr.responseType = options.responseType;
		} catch(e) {
			console.log("XMLHttpRequest doesn't support responseType of '" + options.responseType + "'");
			xhr.bodgeJSON = true;
		}
	
	if ("onload" in options) {
		if (options.responseType == "json" && xhr.bodgeJSON)
			xhr.addEventListener("load", function(e) { var e = { target: { response: JSON.parse(e.target.response) } }; options.onload(e) }, false);
		else
			xhr.addEventListener("load", options.onload, false);
	}

	xhr.open(options.method, options.url);

	if ("json" in options)
		xhr.setRequestHeader( "Content-Type", "application/json" );
	
	if ("contentType" in options)
		xhr.setRequestHeader( "Content-Type", options.contentType );
	
	xhr.send(options.data);
	
	return xhr;
}


function blockExplorerLink(queryData, content) {
	var link = document.createElement('A');
	link.className = "blockex-link";
	link.setAttribute( 'href', 'http://burstcoin.biz/' + queryData );
	link.setAttribute( 'target', '_blank' );
	link.appendChild( document.createTextNode(content) );
	return link;
}


function deadlineElem(deadline) {
	var deadlineElem = document.createElement('SPAN');
	deadlineElem.className = 'deadline';
	deadlineElem.appendChild( document.createTextNode( deadlineToString( parseInt(deadline) ) ) );
	return deadlineElem;
}


function deadlineToString(d) {
	var units = [ 'year', 'month', 'day', 'hour', 'min', 'sec'];
	
	if (d < 2) {
		return Math.floor(d) + " " + units[units.length - 1];
	}
	
	var unit_multipliers = [ 365*24*60*60, 30*24*60*60, 24*60*60, 60*60, 60, 1 ];
	var ds = '';
	for(var i=0; i<units.length; i++) {
		if (d > unit_multipliers[i]) {
			var n_units = Math.floor( d / unit_multipliers[i] );
			d = d % unit_multipliers[i];
			if (ds.length > 0) {
				ds += ', ';
			}
			ds += n_units + " " + units[i];
			if (n_units > 1) {
				ds += 's';
			}
		}
	}
	
	return ds;
}


function HSVToHSLString(h, s, v) {
	var hsl = HSVToHSL(h, s, v);
	return "HSL(" + (hsl[0] * 360) + ", " + (hsl[1] * 100) + "%, " + (hsl[2] * 100) + "%)";
}


function HSVToHSL(h, s, v) { return[h,s*v/((h=(2-s)*v)<1?h:2-h),h/2] };


function accountIdToBGColour(accountId) {
	var h = (accountId & 0x0000ff) / 256.0;
	var s = (accountId & 0x00ff00) / 256.0 / 256.0 / 2.0;
	var v = (accountId & 0xff0000) / 256.0 / 256.0 / 256.0 / 4.0 + 0.75;
	return HSVToHSLString(h, s, v);
}


function accountIdToFGColour(accountId) {
	var h = 1.0 - ( (accountId & 0x0000ff) / 256.0 );
	var s = 1.0 - ( (accountId & 0x00ff00) / 256.0 / 256.0 / 2.0 );
	var v = 1.0 - ( (accountId & 0xff0000) / 256.0 / 256.0 / 256.0 / 4.0 + 0.75 );
	return HSVToHSLString(h, s, v);
}


function prettyMiningCapacity(capacity, notEstimated) {
	return (notEstimated ? '' : '~') + capacity.toFixed(1) + 'TB';
}


function renderPieChart( id, shares, text ) {
	var canvas = document.getElementById(id);
	
	var canvasParent = canvas.parentNode;
	var edgeLength = Math.min(canvasParent.clientWidth, canvasParent.clientHeight);
	canvas.style.width = edgeLength + "px";
	canvas.style.height = edgeLength + "px";
	
	canvas.setAttribute('width', edgeLength);
	canvas.setAttribute('height', edgeLength);
	
	var context = canvas.getContext('2d');
	context.clearRect(0, 0, canvas.width, canvas.height);
	context.lineStyle = 'black';
	context.lineWidth = 2;

	var midX = canvas.width / 2;
	var midY = canvas.height / 2;
	var radius = midX; // assuming square canvas
	var fixedStartAngle = 0 - Math.PI / 2;
	var startAngle = fixedStartAngle;
	var endAngle;

	var shareTotal = 0.0;
	for(var i=0; i<shares.length; i++) {
		// pre-multiply by 1.0 to force conversion of shares[i].share from string to float
		var thisShare = 1.0 * shares[i].share;
		
		// if this share is bigger than the remaining space,
		// or it's the last one
		// then finish off the pie nicely
		if ( i == shares.length - 1 || thisShare > 1.0 - shareTotal )
			endAngle = fixedStartAngle + 2 * Math.PI;
		else
			endAngle = startAngle + (thisShare * 2.0 * Math.PI);

		context.beginPath();
		context.moveTo( midX, midY );
		context.arc(midX, midY, radius, startAngle, endAngle, false);
		context.closePath();
		context.fillStyle = shares[i].background;
		context.fill();
		
		startAngle = endAngle;
		shareTotal += thisShare;
	}

	if (shares.length) {
		context.font = 'bold 16pt Roboto';
		context.fillStyle = '#e91e63';
		context.textAlign = 'center';
		context.lineWidth = 1;
		context.strokeStyle = '#795548';
		multilineText( context, text, midX, midY );
	}
}


function sharesUpdate( sharesInfo ) {
	shares = sharesInfo;
	
	var currentList = document.getElementById('current-shares-list');
	currentList.innerHTML = '';

	for(var i=0; i<shares.length; i++) {
		var share = shares[i];
		var account = accountsCache[ share.accountId ];
		
		share.background = account.bgColour;
		share.color = account.fgColour;
		
		var entry = document.createElement('TR');
		entry.style.background = share.background;
		entry.style.color = share.color;

		entry.appendTd( '', '', blockExplorerLink('address/' + account.accountId, account.displayName) );

		var minerTd = entry.appendTd( '', 'miner', share.miner);
		minerTd.setAttribute("miner", share.miner);
		minerTd.setAttribute("miner-char", share.miner.substr(0, 1) );
		
		var deadlineTd = entry.appendTd('', '', deadlineElem(share.deadline) );
		
		entry.appendTd('', '', (share.share * 100.0).toFixed(3) + '%');
		entry.appendTd('', '', share.estimatedReward ? Number(share.estimatedReward / 100000000).toFixed(2) : '[wait]' );

		currentList.appendChild(entry);
	}

	renderPieChart('current-shares-chart', shares, 'Current block shares');
}


function historicUpdate(sharesInfo) {
	historicShares = sharesInfo;

	var historicList = document.getElementById('historic-shares-list');
	historicList.innerHTML = '';
	
	document.getElementById('num-active-miners').innerHTML = historicShares.length;

	for(var i=0; i<historicShares.length; i++) {
		var share = historicShares[i];
		var account = accountsCache[ share.accountId ];
		
		share.background = account.bgColour;
		share.color = account.fgColour;

		var entry = document.createElement('TR');
		entry.style.background = share.background;
		entry.style.color = share.color;
		
		entry.appendTd( '', '', blockExplorerLink('address/' + account.accountId, account.displayName) );

		entry.appendTd('', '', prettyMiningCapacity(account.miningCapacityTB, account.miningCapacityNotEstimated) );
		entry.appendTd('', '', (share.share * 100.0).toFixed(3) + '%');
		entry.appendTd('', '', share.estimatedReward ? Number(share.estimatedReward / 100000000).toFixed(2) : '[wait]' );
		
		var payouts = [];
		
		if (share.unconfirmedPayouts)
			payouts.push( (Number(share.unconfirmedPayouts) / 100000000).toFixed(2) + 'U' );
			 
		if (share.queuedPayouts)
			payouts.push( (Number(share.queuedPayouts) / 100000000).toFixed(2) + 'Q' );
			 
		if (share.deferredPayouts)
			payouts.push( (Number(share.deferredPayouts) / 100000000).toFixed(2) + 'D' );

		payouts.push( (Number(share.totalPayouts) / 100000000).toFixed(2) + 'P' );
			 
		entry.appendTd('', '', payouts.join(" / "));

		historicList.appendChild(entry);
	}
	
	renderPieChart('historic-shares-chart', historicShares, 'Historic blocks shares');
}


function updateAwards( listId, awardsData ) {
	var awards = document.getElementById(listId);
	awards.innerHTML = '';
	
	for(var award in awardsData) {
		var tr = document.createElement('TR');
		
		var awardTd = tr.appendTd('', 'award', award);
		
		if (awardsData[award].accountId) {
			var account = accountsCache[ awardsData[award].accountId ];
			
			var accountTd = tr.appendTd();
			accountTd.appendChild( blockExplorerLink('address/' + account.accountId, account.displayName) );
			accountTd.style.color = account.fgColour;
			accountTd.style.backgroundColor = account.bgColour;
			accountTd.style.textAlign = 'center';
		} else {
			var accountTd = tr.appendTd('', '', awardsData[award]);
			accountTd.style.textAlign = 'center';
		}
		
		awards.appendChild(tr);
	}
}


function blockUpdate( blockInfo ) {
	console.log("New block: " + blockInfo.block);

	shares = [];
	renderPieChart('current-shares-chart', shares, 'Current block share');
	
	var list = document.getElementById('current-shares-list');
	list.innerHTML = '';
	
	document.getElementById('current-block-height').innerHTML = "\n" + blockInfo.block;
	document.getElementById('current-block-scoop').innerHTML = "\n" + blockInfo.scoop;
	document.getElementById('num-assigned-miners').innerHTML = blockInfo.accountsRewardingUs;
	
	currentBlockStart = new Date(blockInfo.newBlockWhen * 1000);
	
	// 'awards' from previous block
	updateAwards( 'previous-awards-list', blockInfo.awards );
	
	// update recent blocks after a short pause to allow blockchain refresh process to catch up
	window.setTimeout( updateRecentBlocks, 8000 );
	
	// clear current awards
	updateAwards( 'current-awards-list', [] );
}


function accountsUpdate( accounts ) {
	for(var i=0; i<accounts.length; i++) {
		var accountInfo = accounts[i];
		
		// pre-gen colours
		accountInfo.fgColour = accountIdToFGColour( accountInfo.accountId32 );
		accountInfo.bgColour = accountIdToBGColour( accountInfo.accountId32 );
		
		// account name or RS string?
		accountInfo.displayName = accountInfo.accountName ? accountInfo.accountName : accountInfo.account; 
		
		accountsCache[ accountInfo.accountId ] = accountInfo;
	}
}


function awardsUpdate(awards) {
	// 'awards' from previous block
	updateAwards( 'current-awards-list', awards );
}


var ws;
function grabPoolUpdates() {
	console.log("Opening 'updates' WebSocket...");
	ws = new WebSocket('ws://' + window.location.host + '/WS/updates', 'updates');
	
	ws.onmessage = function(e) {
		if (e.data) {
			var colonIndex = e.data.indexOf(':');
			
			if ( colonIndex != -1 ) {
				var method = e.data.substr(0, colonIndex).toLowerCase() + "Update";
				
				if ( window[method] )
					window[method]( JSON.parse( e.data.substr(colonIndex+1) ) );
			}
		}
	};
	
	ws.onopen = function(e) {
		ws.send("hello");
		ws.pingInterval = window.setInterval( function() { ws.send("ping") }, 30000 );
	};
	
	ws.onerror = function(e) {
		console.log("WebSocket error");
		
		if (ws)
			ws.close();
	};
	
	ws.onclose = function(e) {
		console.log("WebSocket closed");
		
		if (ws && ws.pingInterval) {
			window.clearInterval(ws.pingInterval);
		}
		
		ws = null;
		
		// try to reconnect
		window.setTimeout(grabPoolUpdates, 5000);
	};
}


function multilineText( context, text, x, y, upwards ) {
	var lines = text.split(/\n/);
	
	if (upwards)
		lines = lines.reverse();
	
	for(var i=0; i<lines.length; ++i) {
		var line = lines[i];
		context.strokeText(line, x, y);
		context.fillText(line, x, y);

		var span = document.createElement('SPAN');
		span.style.visibility = 'hidden';
		span.appendChild( document.createTextNode( line ) );
		document.body.insertBefore(span, document.body.firstElementChild);

		if (upwards)
			y -= span.offsetHeight;
		else
			y += span.offsetHeight;

		span.parentNode.removeChild(span);
	}
}


function renderTimer() {
	if (currentBlockStart) {
		var canvas = document.getElementById('timer');
		
		
		var canvasParent = canvas.parentNode;
		var edgeLength = Math.min(canvasParent.clientWidth, canvasParent.clientHeight) - 8;	// 4px padding
		
		canvas.setAttribute('width', edgeLength / 2);
		canvas.setAttribute('height', edgeLength);

		canvas.style.width = canvas.width + "px";
		canvas.style.height = canvas.height + "px";
		
		var context = canvas.getContext('2d');
		context.clearRect(0, 0, canvas.width, canvas.height);
		
		// we need at least one share
		if ( shares.length > 0 ) {
			var deadline = shares[0].deadline + 1;	// +1 because sometimes deadline is 0!
			var secondsSoFar = (new Date() - currentBlockStart) / 1000;
			if (secondsSoFar > deadline) {
				secondsSoFar = deadline;
			}
			var secondsToGo = deadline - secondsSoFar;
			
			var midX = canvas.width / 2;
			var midY = canvas.height / 2;

			// upper bulb is seconds to go
			context.beginPath();
			// centre
			context.moveTo( midX, midY );
			// top-left
			context.lineTo( midX - midX * secondsToGo / deadline, midY - midY * secondsToGo / deadline );
			// top-right
			context.lineTo( midX + midX * secondsToGo / deadline, midY - midY * secondsToGo / deadline );
			// centre
			context.lineTo( midX, midY );
			context.closePath();
			var upperGradient = context.createLinearGradient(midX, midY, midX, midY - midY * secondsToGo / deadline);
			upperGradient.addColorStop(0, '#004000');
			upperGradient.addColorStop(1, '#008000');
			context.fillStyle = upperGradient;
			context.fill();
			
			// lower bulb is seconds so far
			context.beginPath();
			// bottom-left
			context.moveTo( 0, midY * 2 );
			// bottom-top
			context.lineTo( midX * 2, midY * 2 );
			// top
			context.lineTo( midX, midY * 2 - midY * secondsSoFar / deadline ); 
			// bottom-left
			context.lineTo( 0, midY * 2);
			context.closePath();
			var lowerGradient = context.createLinearGradient(midX, midY*2, midX, midY * 2 - midY * secondsSoFar / deadline);
			lowerGradient.addColorStop(0, '#400000');
			lowerGradient.addColorStop(1, '#800000');
			context.fillStyle = lowerGradient;
			context.fill();
			
			// draw bulbs
			context.beginPath();
			// top-left
			context.moveTo( 0, 0 );
			// top-right
			context.lineTo( midX * 2, 0 );
			// centre
			context.lineTo( midX + 4, midY );
			// bottom-right
			context.lineTo( midX * 2, midY * 2 );
			// bottom-left
			context.lineTo( 0, midY * 2 );
			// centre 
			context.lineTo( midX - 4, midY );
			// top-left
			context.lineTo( 0, 0 );
			context.closePath();
			context.lineWidth = 4;
			context.strokeStyle = 'black';
			context.stroke();
			
			// write times into bulbs
			context.font = 'bold 10pt Roboto';
			context.fillStyle = '#e91e63';
			context.textAlign = 'center';
			context.lineWidth = 1;

			if (secondsToGo < 1)
				multilineText( context, deadlineToString("FORGING!"), midX, midY * 0.25 );
			else
				multilineText( context, deadlineToString(secondsToGo).replace(/, /g, "\n"), midX, 20 );
				
			multilineText( context, deadlineToString(secondsSoFar).replace(/, /g, "\n"), midX, canvas.height - 8, true );
		}
	}
}


function updateRecentBlocks() {
	XHR( { url: "/XHR/getRecentBlocks", responseType: "json", onload: updateRecentBlocksCallback } );
}


function updateRecentBlocksCallback(e) {
	var blocksInfo = e.target.response;
	
	if (blocksInfo && blocksInfo.blocks) {
		var recentBlocks = document.getElementById('recent-blocks-list');
		recentBlocks.innerHTML = '';
		
		for(var i=0; i<blocksInfo.blocks.length; i++) {
			var block = blocksInfo.blocks[i];
			
			var tr = document.createElement('TR');
			
			tr.appendTd('', '', blockExplorerLink('block/' + block.block_id, block.block) );

			if ( !block.isOurBlock ) {
				// two columns: 
				// first is actual generator (not one belonging to pool)
				// second is our best miner
				var genAccount = block.generatorAccountName ? block.generatorAccountName : block.generatorAccount;
				
				// first column
				var generatorTd = tr.appendTd('', '', blockExplorerLink('address/' + block.generatorAccountId, genAccount) );
				generatorTd.appendChild( deadlineElem(block.deadline) );
			}

			if (block.ourBestAccount != undefined) {
				var bestAccount = block.ourBestAccountName ? block.ourBestAccountName : block.ourBestAccount;
				
				var ourMinerTd = tr.appendTd('', '', blockExplorerLink('address/' + block.ourBestAccountId, bestAccount) );
				ourMinerTd.appendChild( deadlineElem(block.ourBestDeadline) );
				ourMinerTd.style.textAlign = 'center';
				ourMinerTd.style.backgroundColor = accountIdToBGColour(block.ourBestAccountId32);
				ourMinerTd.style.color = accountIdToFGColour(block.ourBestAccountId32);

				// if we won block then this is actually the first column, so spread it over both columns
				if ( block.isOurBlock )
					ourMinerTd.setAttribute('colspan', 2);
			} else {
				// somehow we didn't have ANY miner for this block, so empty column
				tr.appendTd();
			}
			
			recentBlocks.appendChild(tr);
		}
	}
}


function changeTab(e) {
	var tabId = e.target.getAttribute('tab-id');
	
	var tabs = document.getElementsByTagName("LI");
	for(var i=0; i<tabs.length; ++i) {
		var tab = tabs[i];
		
		if (tab.getAttribute('tab-id') == tabId)
			tab.setAttribute('tab', 'active');
		else
			tab.removeAttribute('tab');
	}

	var tabPanes = document.getElementsByClassName("tab");
	for(var i=0; i<tabPanes.length; ++i) {
		var pane = tabPanes[i];
		
		if (pane.id == tabId)
			pane.setAttribute('tab', 'active');
		else
			pane.removeAttribute('tab');
	}
	
	// re-render pie charts as their sizes are calculated only when rendered
	// (can't get width/height when display:none)
	renderPieChart('current-shares-chart', shares, 'Current block shares');
	renderPieChart('historic-shares-chart', historicShares, 'Historic blocks shares');
}


function resizeWindowHandler() {
	if ( resizeTimeout )
		return;
	
	resizeTimeout = window.setTimeout( resizeWindow, 100 );
}


function resizeWindow() {
	// rerender items that don't get reflowed by the browser
	renderPieChart('current-shares-chart', shares, 'Current block shares');
	renderPieChart('historic-shares-chart', historicShares, 'Historic blocks shares');
	renderTimer();
	
	resizeTimeout = null;
}


function init() {
	// add support to "tab"s
	var tabs = document.getElementById('tabs');
	for(var i=0; i<tabs.children.length; ++i) {
		var tab = tabs.children[i];
		tab.addEventListener('click', changeTab, false);
	}
	
	grabPoolUpdates();
	updateRecentBlocks();
	window.setInterval( renderTimer, 1000 );
	
	window.addEventListener('resize', resizeWindowHandler, false); 
}

init();
