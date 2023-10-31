"use strict";
var colorToReset;
var colorError = "#ea401b";
var colorSucess = "#25bb0d";
document.addEventListener("DOMContentLoaded", function(){
	var slider = document.getElementById("deepSleep");
	slider.oninput = function() {
		document.getElementById("intText").innerHTML = this.value;
	};
	colorToReset = document.getElementById("SaveBtn").style.backgroundColor;
});
function UpdateData(arr){	
	document.getElementById("highRes").checked = arr[0] == '1' ? true : false;
	document.getElementById("skipDeepsleep").checked = arr[1] == '1' ? true : false;
	document.getElementById("deepSleep").value = arr[2];
	document.getElementById("intText").innerHTML = arr[2];
	document.getElementById("pirActive").checked = arr[3] == '1' ? true : false;
	document.getElementById("sleepHour").value = arr[4];
	document.getElementById("wakeupHour").value = arr[5];	
}
function GetData(){
	var oRequest = new XMLHttpRequest();
	var sURL  = '/base';
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			var arr = oRequest.responseText.split("|");	
			UpdateData(arr);
		}
	};
	oRequest.onerror = function (e) {
		
		UpdateData(FakeData());
		SetOutput("Get Data failed!", true);
	};
	oRequest.send(null);
}
function SetData(){
	var oRequest = new XMLHttpRequest();
	var sURL  = '/base?';
	sURL += 'highRes=' + (document.getElementById("highRes").checked ? '1' : '0');
	sURL += '&skipDeepsleep=' + (document.getElementById("skipDeepsleep").checked ? '1' : '0');	
	sURL += '&deepSleep=' + document.getElementById("deepSleep").value;	
	sURL += '&pirActive=' + (document.getElementById("pirActive").checked ? '1' : '0');	
	sURL += '&hourToKeepAwake=' + document.getElementById("wakeupHour").value;	
	sURL += '&hourToSleep=' + document.getElementById("sleepHour").value;	
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			var arr = oRequest.responseText.split("|");	
			UpdateData(arr);
			SetOutput("Data set sucessfully!", false);
			var btn = document.getElementById("SaveBtn");
			btn.style.backgroundColor = colorSucess;
		}
	};
	oRequest.onerror = function (e) {
		
		UpdateData(FakeData());
		SetOutput("Set Data failed!", true);
	};
	oRequest.send(null);
}
function ToggleLed(){
	var oRequest = new XMLHttpRequest();
	var sURL  = '/led';
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			var btn = document.getElementById("ToggleBtn");
			if(oRequest.responseText == '1')
				btn.style.backgroundColor = colorSucess;
			else
				btn.style.backgroundColor = colorToReset;
		}
	};
	oRequest.onerror = function (e) {
		
		UpdateData(FakeData());
		SetOutput("Toggle Led failed!", true);
	};
	oRequest.send(null);
}
function SendToSleep(){
	var oRequest = new XMLHttpRequest();
	var sURL  = '/sleep';
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){
			SetOutput(oRequest.responseText, false);
			var btn = document.getElementById("SleepBtn");
			btn.style.backgroundColor = colorSucess;
		}
	};
	oRequest.onerror = function (e) {
		SetOutput("Toggle Led failed!", true);
	};
	oRequest.send(null);
}
function FakeData(){
	var arr = ["1", "1", "120", "1", "7", "21"];
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
function ResetControls() {
	var output = document.getElementById("Output");
	output.innerHTML = "";
	output.style.color = document.body.style.color;	
	if(document.getElementById("SaveBtn") != null)
		document.getElementById("SaveBtn").style.backgroundColor = colorToReset;
	if(document.getElementById("SleepBtn") != null)
		document.getElementById("SleepBtn").style.backgroundColor = colorToReset;
}