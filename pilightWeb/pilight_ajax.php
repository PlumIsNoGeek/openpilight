<?php
/*
* Copyright (c) 2015 Florian Geyer
* https://github.com/PlumIsNoGeek/openpilight
*/
require "config.inc.php";
require 'Milight.php';
$result = "ERR";
$milight = new Milight($milight_server, $milight_port);
$milight->setDelay(0);
$milight->setActiveGroupSend(0);
if (isset($_GET['off'])) {
    $milight->rgbwSendOffToGroup($_GET['off']);
	$result = "OK";
}
if (isset($_GET['on'])) {
	$milight->setActiveGroupSend(1);
    $milight->rgbwSendOnToGroup($_GET['on']);
	$milight->setActiveGroupSend(0);
	$result = "OK";
}
if (isset($_GET['white'])) {
	$milight->rgbwSetGroupToWhite($_GET['white']);
	$result = "OK";
}
if (isset($_GET['night'])) {
	$milight->rgbwSetGroupToNightMode($_GET['night']);
	$result = "OK";
}
if (isset($_GET['hue'])) {
	//echo $_GET['hue'];
	$milight->rgbwSetColorHsv([$_GET['hue'], 0, 0]);
	$result = "OK";
}
if (isset($_GET['brightness'])) {
	//echo $_GET['hue'];
	$milight->rgbwBrightnessPercent(min(100,$_GET['brightness']));
	$result = "OK";
}
if (isset($_GET['assoc'])) {
	$data = [0xff, hexdec($_GET['assocAddrH']), hexdec($_GET['assocAddrL']), $_GET['assocGroup']]; //0x55 is appended
	$milight->sendCommand($data);
	$result = "OK";
}
if (isset($_GET['raw'])) {
	$repeats = [];
	$rdelays = [];
	$delays = [];
	$sendraw = [];
	$lines = explode("\n", trim($_GET['raw']));
	foreach ($lines as $i=>$l) {
		$line = $i+1;
		$bytes = explode(" ", $l);
		$data = [];
		$delay = 0;
		$rdelay = 0;
		$repeat = 0;
		foreach ($bytes as $b) {
			if (preg_match("/rd=([0-9]{1,})/", $b, $regs)) {
				$rdelay = $regs[1];
			} else if (preg_match("/d=([0-9]{1,})/", $b, $regs)) {
				$delay = $regs[1];
			} else if (preg_match("/r=([0-9]{1,})/", $b, $regs)) {
				$repeat = $regs[1];
			} else {
				$val = hexdec($b);
				if (($val < 0) || ($val > 255)) {
					echo "invalid number $b at line $line";
					exit;
				} else {
					$data[] = $val;
				}
			}
		}
		if (count($data) != 7) {
			echo "line $line does not contain not 7 bytes";
			exit;
		}
		$sendraw[] = $data;
		$repeats[] = $repeat;
		$delays[] = $delay;
		$rdelays[] = $rdelay;
	}
	
	foreach ($sendraw as $i=>$data) {
		array_unshift($data, 0xff);
		$repeat = $repeats[$i]+1;
		for ($j = 0; $j < $repeat; $j++) {
			$milight->sendCommand($data);
			if ($rdelays[$i] > 0) {
				usleep(1000*$rdelays[$i]);
			}
		}
		if ($delays[$i] > 0) {
			usleep(1000*$delays[$i]);
		}
	}
	$result = "OK";
}
echo $result;
?>