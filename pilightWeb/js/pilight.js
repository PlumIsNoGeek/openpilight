/*
* Copyright (c)2015 Florian Geyer
* https://github.com/PlumIsNoGeek/openpilight
*/

var EVENT_HUE = 0;
var EVENT_LUM = 1;

var hideErrorTimeout = 0;
var debounceAjaxMS = 100;
var lastTimestamps = [0, 0];
var lastEventTimers = [0, 0];
function debounceAjax(dataToSend, evtId) {
  var timestamp = $.now();
  if (lastEventTimers[evtId] != 0) {
  	window.clearTimeout(lastEventTimers[evtId]);
  }
  if ((timestamp-lastTimestamps[evtId]) > debounceAjaxMS) {
  	lastTimestamps[evtId] = timestamp;
  	sendAjax(dataToSend);
  } else {
  	lastEventTimers[evtId] = window.setTimeout(sendAjax, debounceAjaxMS, dataToSend);
  }
}

function sendAjax(dataToSend) {
$.ajax({
	url: "pilight_ajax.php",
	data: dataToSend,
	success: function(data) {
		if (data != "OK") {
			showError(data, dataToSend);
		} else {
			if (hideErrorTimeout == 0) {
				hideErrorTimeout = window.setTimeout(hideError, 2500);
			}
		}
	},
	error: function() {
		showError("REQUEST_FAILED", dataToSend);
	}
});
}