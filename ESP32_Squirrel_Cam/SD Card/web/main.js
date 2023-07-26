"use strict";
var colorToReset;
var colorError = "#ea401b";
var colorSucess = "#25bb0d";

function ToggleBurger(){
	var burger = document.getElementById("hamburger-6");
	burger.classList.toggle('is-active');
	var x = document.getElementById("myLinks");
  if (x.style.display === "block")
    x.style.display = "none";
  else
    x.style.display = "block";
}
function NavigateMenu(url){
	ToggleBurger();
	document.getElementById("subframe").src = url;
}
function SetSetup(submitData = true){
	SetLoadingAnimation(true);
	var oRequest = new XMLHttpRequest();
	var sURL  = '/setup';
	if(submitData)
		sURL = CreateUrl(sURL);
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			var arr = oRequest.responseText.split("|");	
			UpdateSetup(arr);
			if(submitData){
				colorToReset = document.getElementById("SaveBtn").style.backgroundColor;
				document.getElementById("SaveBtn").style.backgroundColor = colorSucess;
				ResetControlsDelayed();	
			}
			else
				UpdateWiFi();
		}
	};
	oRequest.onerror = function (e) {
		colorToReset = document.getElementById("SaveBtn").style.backgroundColor;
		if(submitData)
			SetOutput("Set Setup failed!", true);
		else{
			SetOutput("Get Setup failed!", true);	
			UpdateSetup(FakeOutput());
		}
		document.getElementById("SaveBtn").style.backgroundColor = colorError;
	};
		oRequest.send(null);
}
function GetInfo(){
	SetLoadingAnimation(true);
	var oRequest = new XMLHttpRequest();
	var sURL  = '/espinfo';	
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			var div = document.getElementById("espinfo");
			div.innerHTML = "";
			var arr = oRequest.responseText.split("|");	
			for (var i = 0; i < arr.length; i++) {
				var vals = arr[i].split(";");
				var lbl = document.createElement('label');
				lbl.htmlFor = vals[0];
				lbl.innerHTML = vals[0];
				var inp = document.createElement('input');
				inp.type = "text";
				inp.id = vals[0];
				inp.value = vals[1];
				inp.readOnly = true;
				div.append(lbl, inp);
			}
			colorToReset = document.getElementById("SaveBtn").style.backgroundColor;
			document.getElementById("SaveBtn").style.backgroundColor = colorSucess;
			ResetControlsDelayed();	
		}
	};
	oRequest.onerror = function (e) {
		colorToReset = document.getElementById("SaveBtn").style.backgroundColor;
		SetOutput("Get WiFi failed!", true);
		UpdateSetup(FakeOutput());
		document.getElementById("SaveBtn").style.backgroundColor = colorError;
	};
	oRequest.send(null);
}
function UpdateWiFi(){
	var oRequest = new XMLHttpRequest();
	var sURL  = '/wifi';
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			var select = document.getElementById("ssidOption");
			var arr = oRequest.responseText.split("|");	
			for (var i = 0; i < arr.length; i++) {
				var opt = document.createElement('option');
				opt.value = i;
				opt.innerHTML = arr[i];
				select.appendChild(opt);
			}
		}
	};
	oRequest.onerror = function (e) {
		colorToReset = document.getElementById("SaveBtn").style.backgroundColor;
		SetOutput("Get WiFi failed!", true);
		UpdateSetup(FakeOutput());
		document.getElementById("SaveBtn").style.backgroundColor = colorError;
	};
	oRequest.send(null);
}
function UpdateSSID(){
	var select = document.getElementById("ssidOption");
	var selectedValue = select.options[select.selectedIndex].text;
	var arr = selectedValue.split(";");
	document.getElementById("ssid").value = arr[0];
}
function UpdateSetup(arr){	
	document.getElementById("serial_enabled").checked = arr[0] == '1' ? true : false;
	document.getElementById("ota_enabled").checked = arr[1] == '1' ? true : false;	
	document.getElementById("webserver_enabled").checked = arr[2] == '1' ? true : false;
	document.getElementById("force_accesspoint").checked = arr[3] == '1' ? true : false;
	document.getElementById("ssid").value = arr[4];
	document.getElementById("password").value = arr[5];
}
function CreateUrl(sURL){
	sURL += '?serial_enabled=';
	sURL += document.getElementById("serial_enabled").checked ? '1' : '0';
	sURL += '&ota_enabled=';
	sURL += document.getElementById("ota_enabled").checked ? '1' : '0';
	sURL += '&webserver_enabled=';
	sURL += document.getElementById("webserver_enabled").checked ? '1' : '0';
	sURL += '&force_accesspoint=';
	sURL += document.getElementById("force_accesspoint").checked ? '1' : '0';
	sURL += '&ssid=';
	sURL += document.getElementById("ssid").value;
	sURL += '&password=';
	sURL += document.getElementById("password").value;
	return sURL;
}
function ResetFiles(){
	var oRequest = new XMLHttpRequest();
	var sURL  = '/filereset';
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			colorToReset = document.getElementById("NewFilesBtn").style.backgroundColor;
			document.getElementById("NewFilesBtn").style.backgroundColor = colorSucess;
			ResetControlsDelayed();
		}
	};
	oRequest.onerror = function (e) {
		colorToReset = document.getElementById("NewFilesBtn").style.backgroundColor;
		SetOutput("File reset failed!", true);
		document.getElementById("NewFilesBtn").style.backgroundColor = colorError;
	};
	oRequest.send(null);
}
function Restart(){
	var oRequest = new XMLHttpRequest();
	var sURL  = '/restart';	
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			colorToReset = document.getElementById("RestartBtn").style.backgroundColor;
			document.getElementById("RestartBtn").style.backgroundColor = colorSucess;
			ResetControlsDelayed();
		}
	};
	oRequest.onerror = function (e) {
		colorToReset = document.getElementById("RestartBtn").style.backgroundColor;
		SetOutput("Reboot failed!", true);
		document.getElementById("RestartBtn").style.backgroundColor = colorError;
	};
	oRequest.send(null);
}
function SerialMonitor(){
	if(!document.getElementById("wireless_serial").checked)
		return;
	var oRequest = new XMLHttpRequest();
	var sURL  = '/serial';	
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			if(oRequest.responseText == "")
				return;
			var textarea = document.getElementById("wireless_serial_text");
			textarea.innerHTML += oRequest.responseText;
			textarea.scrollTop = textarea.scrollHeight;
		}
	};
	oRequest.onerror = function (e) {
		var serial = document.getElementById("wireless_serial_text");
		if(!serial.innerHTML.endsWith("Serial connection failed!"))
			serial.innerHTML += "Serial connection failed!";
	};
	oRequest.send(null);
	setTimeout(SerialMonitor,2000);
}
function FakeOutput(){
	var arr = ["0", "1", "1", "0", "", ""];
	return arr;
}
function SetOutput(text, error){
	var output = document.getElementById("Output");
	output.innerHTML = text;
	output.style.color = error? colorError : document.body.style.color;
	ResetControlsDelayed();
}
function ResetControlsDelayed() {
	setTimeout(ResetControls,3000);
}
function SetLoadingAnimation(active){
	var roller = document.getElementsByClassName("lds-roller")[0];
	if (roller == null)
		return;
	roller.style.display = active ? "inline-block" : "none";
}
function ResetControls() {
	var output = document.getElementById("Output");
	output.innerHTML = "";
	output.style.color = document.body.style.color;	
	if(document.getElementById("SaveBtn") != null)
		document.getElementById("SaveBtn").style.backgroundColor = colorToReset;
	if(document.getElementById("NewFilesBtn") != null)
		document.getElementById("NewFilesBtn").style.backgroundColor = colorToReset;
	if(document.getElementById("RestartBtn") != null)
		document.getElementById("RestartBtn").style.backgroundColor = colorToReset;
	SetLoadingAnimation(false);
}